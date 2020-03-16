// main.c
// Execution engine / player code.
// Copyright (C)1997 BJ Eirich

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <emscripten.h>
#include <emscripten/fetch.h>
#include "control.h"
#include "engine.h"
#include "keyboard.h"
#include "menu.h"
#include "menu2.h"
#include "render.h"
#include "timer.h"
#include "sound.h"
#include "vga.h"
#include "vc.h"
#include "vclib.h"
#include "render.h"
#include "fs.h"
using namespace verge;

char* strbuf;
unsigned char* speech;
extern unsigned char menuptr[256], qabort, itmptr[576], charptr[960];
extern char menuactive;
extern char fade;

extern unsigned int effectofstbl[1024], startupofstbl[1024], magicofstbl[1024];

namespace verge {
    std::string gameRoot = "sully/";
    std::vector<std::string> manifest;
    std::string saveGameRoot;

    void preload(std::string_view);

    EM_JS(void, fetchSync, (const char* pathPtr, size_t* size, char** data), {
        return Asyncify.handleSleep(resume => {
            const path = UTF8ToString(pathPtr);
            // console.log('fetchSync', path);
            return fetch(path).then(response => {
                if (!response.ok) {
                    console.error('fetchSync failed', path);
                    HEAP32[size >> 2] = 0;
                    HEAP32[data >> 2] = 0;
                    resume();
                    return;
                }
                return response.blob();
            }).then(blob =>
                blob.arrayBuffer()
            ).then(array => {
                const bytes = new Uint8Array(array);
                HEAP32[size >> 2] = bytes.length;
                const dataPtr = _malloc(bytes.length);
                HEAP32[data >> 2] = dataPtr;
                HEAP8.set(bytes, dataPtr);
                resume();
            });
        });
    });

    struct FreeDelete { void operator()(char* p) { free(p); } };
    using Deleter = std::unique_ptr<char, FreeDelete>;

    EM_JS(void, setLoadingProgress, (int progress), {
        verge.setLoadingProgress(progress);
    });

    void downloadGame() {
        std::string manifestPath = gameRoot + "manifest.txt";
        char* manifestPtr;
        size_t manifestLength;
        fetchSync(manifestPath.c_str(), &manifestLength, &manifestPtr);
        Deleter hello{ manifestPtr };

        std::string_view manifest{ manifestPtr, manifestLength };

        std::vector<std::string_view> files;
        auto append = [&](std::string_view name) {
            if (!name.empty())
                files.push_back(name);
        };

        while (!manifest.empty()) {
            auto pos = manifest.find('\n');
            if (pos == std::string::npos) {
                append(manifest);
                break;
            }
            append(manifest.substr(0, pos));
            manifest.remove_prefix(pos + 1);
        }

        int i = 0;

        for (const auto& filename: files) {
            preload(filename);

            int progress = i * 100 / files.size();
            ++i;
            setLoadingProgress(progress);
        }

        setLoadingProgress(100);
    }

    void preload(std::string_view path) {
        std::string filename = gameRoot;
        filename.append(path.begin(), path.end());

        size_t contentLength;
        char* content;
        fetchSync(filename.c_str(), &contentLength, &content);
        Deleter hello{ content };

        verge::DataVec vec(content, content + contentLength);

        // printf("Preloaded '%s' %zi bytes\n", filename.c_str(), vec.size());
        verge::vset(std::string{ path }, std::move(vec));
    }

    EM_JS(void, wasm_initFileSystem, (const char* c), {
        let sgr = UTF8ToString(c);
        if (sgr.endsWith('/'))
            sgr = sgr.substr(0, sgr.length - 1);
        FS.mkdir("/persist");
        FS.mkdir(sgr);
        // Then mount with IDBFS type
        FS.mount(IDBFS, {}, sgr);

        // Then sync
        FS.syncfs(true, function (err) {
            // Error
            if (err)
                console.error('wasm_initFileSystem failed!', err);
        });
    });

    void initFileSystem() {
        saveGameRoot = "/persist/" + gameRoot;
        wasm_initFileSystem(saveGameRoot.c_str());
    }
}

int Exist(const char* fname) {
    VFILE* tempf;

    tempf = vopen(fname, "rb");
    if (tempf) {
        vclose(tempf);
        return 1;
    } else {
        return 0;
    }
}

void err(const char* ermsg, ...) {
    keyboard_close();
    stopsound();
    timer_close();
    closevga();

    va_list args;
    va_start(args, ermsg);
    vprintf(ermsg, args);
    va_end(args);

    putchar('\n');
    exit(-1);
}

void MiscSetup() {
    strbuf = (char*)valloc(100, "strbuf");
    keyboard_init();
    keyboard_chain(0);
    sound_init();
    timer_init();
    //  allocbuffers(); /* -- ric: 30/Apr/98 -- */
    /* Removed as already called in sound_init() */
    LoadFont();
    InitRenderSystem();
    srand(time());
    if (Exist("STARTUP.SCR")) {
        printf("Warning: startup.scr found, this file is no longer used \n");
    }
}

void PutOwnerText() {
    printf("VERGE - System version 18.May.99\n");
    printf("Copyright (C)1997 vecna\n");
    printf("\n");
    printf("The VERGE Development Team\n");
    printf("--\n");
    printf("vecna, hahn, zeromus, McGrue, Locke, aen, Ric, NichG, xBig_D\n");
    printf("--\n");
    printf("Additional modifications by andy\n");
}

void InitPStats() {
    auto pdat = vopen("PARTY.DAT", "r");
    if (!pdat) {
        err("Fatal error: PARTY.DAT not found");
    }
    vscanf(pdat, "%s", strbuf);
    tchars = atoi((char*)strbuf);
    for (auto i = 0; i < tchars; i++) {
        vscanf(pdat, "%s", &pstats[i].chrfile);
        vscanf(pdat, "%s", strbuf);
        vscanf(pdat, "%s", strbuf);
        auto cdat = vopen(strbuf, "r");
        if (!cdat) {
            err("Could not open character DAT file.");
        }
        vscanf(cdat, "%s", &pstats[i].name);
        vgets(strbuf, 99, cdat);
        vgets(strbuf, 99, cdat);
        vscanf(cdat, "%s", strbuf);
        pstats[i].exp = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].curhp = atoi(strbuf);
        pstats[i].maxhp = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].curmp = atoi(strbuf);
        pstats[i].maxmp = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].str = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].end = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].mag = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].mgr = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].hit = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].dod = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].mbl = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].fer = atoi(strbuf);
        vscanf(cdat, "%s", strbuf);
        pstats[i].rea = atoi(strbuf);
        pstats[i].lv = 1;
        vscanf(cdat, "%s", strbuf);
        pstats[i].nxt = atoi(strbuf);
        pstats[i].status = 0;
        pstats[i].invcnt = 6;
        memset(&pstats[i].inv, 0, 512);
        vclose(cdat);
    }
    vclose(pdat);
}

void StartNewGame(char* startp) {
    numchars = 0;
    InitPStats();
    addcharacter(1);
    usenxy = 0;
    gp = 0;
    party[0].speed = 4;
    memset(flags, 0, 32000);
    hr = 0;
    min = 0;
    sec = 0;
    tickctr = 0;
    quake = 0;
    hooktimer = 0;
    hookretrace = 0;
    foregroundlock = 1;
    screengradient = 0;
    layervc = 0;
    VCClear();
    layer0 = 1;
    layer1 = 1;
    drawparty = 1;
    drawentities = 1;
    cameratracking = 1;
    UpdateEquipStats();
    startmap(startp);
}

void LoadGame(char* fn) {
    char i, b;

    quake = 0;
    hooktimer = 0;
    hookretrace = 0;
    fade = 1;
    screengradient = 0;
    layervc = 0;
    VCClear();
    foregroundlock = 1;
    layer0 = 1;
    layer1 = 1;
    drawparty = 1;
    drawentities = 1;
    cameratracking = 1;
    numchars = 0;

    std::string realPath = verge::saveGameRoot + fn;
    FILE* f = fopen(realPath.c_str(), "rb");
    fread(strbuf, 1, 51, f);
    fread(&gp, 1, 4, f);
    fread(&hr, 1, 1, f);
    fread(&min, 1, 1, f);
    fread(&sec, 1, 1, f);
    fread(&b, 1, 1, f);
    fread(&menuactive, 1, 1, f);
    fread(virscr, 1, 2560, f);
    fread(&mapname, 1, 13, f);
    fread(&party, 1, sizeof party, f);
    fread(&partyidx, 1, 5, f);
    fread(&flags, 1, 32000, f);
    fread(&tchars, 1, 1, f);
    fread(&pstats, 1, sizeof pstats, f);
    fclose(f);
    for (i = 0; i < b; i++) {
        addcharacter(partyidx[i]);
    }
    nx = party[0].x / 16;
    ny = party[0].y / 16;
    usenxy = 1;
    startmap(mapname);
}

void toUpperCase(char* s) {
    while (char c = *s++) {
        if (c >= 'a' && c <= 'z') {
            c ^= 32;
        }
    }
}

void ProcessEquipDat() {
    VFILE* f;
    int a, i;
    int i2;

    // This function parses EQUIP.DAT, which sets all the stats for equipment.
    // It's pretty long as it has to have a processing section for each possible
    // stat, plus some other stuff. :P

    f = vopen("EQUIP.DAT", "r");
    if (!f) {
        err("Could not open EQUIP.DAT.");
    }
    vscanf(f, "%d", &a);
    for (i = 1; i <= a; i++) {
pl1:
        vscanf(f, "%s", strbuf);
        toUpperCase(strbuf);
        if (!strcmp(strbuf, "//")) {
            vgets(strbuf, 99, f);
            goto pl1;
        }
        if (!strcmp(strbuf, "ATK")) {
            vscanf(f, "%s", strbuf);
            equip[i].str = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "DEF")) {
            vscanf(f, "%s", strbuf);
            equip[i].end = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "MAG")) {
            vscanf(f, "%s", strbuf);
            equip[i].mag = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "MGR")) {
            vscanf(f, "%s", strbuf);
            equip[i].mgr = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "HIT")) {
            vscanf(f, "%s", strbuf);
            equip[i].hit = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "DOD")) {
            vscanf(f, "%s", strbuf);
            equip[i].dod = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "MBL")) {
            vscanf(f, "%s", strbuf);
            equip[i].mbl = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "FER")) {
            vscanf(f, "%s", strbuf);
            equip[i].fer = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "REA")) {
            vscanf(f, "%s", strbuf);
            equip[i].rea = atoi(strbuf);
            goto pl1;
        }
        if (!strcmp(strbuf, "ONEQUIP")) {
            vscanf(f, "%s", strbuf);
            equip[i].onequip = atoi(strbuf) + 1;
            goto pl1;
        }
        if (!strcmp(strbuf, "ONDEEQUIP")) {
            vscanf(f, "%s", strbuf);
            equip[i].ondeequip = atoi(strbuf) + 1;
            goto pl1;
        }
        if (!strcmp(strbuf, "EQABLE")) {
eqloop:
            vscanf(f, "%s", strbuf);
            if (!strcmp(strbuf, "-")) {
                continue;
            }
            equip[i].equipable[atoi(strbuf) - 1] = 1;
            goto eqloop;
        }
    }
    // FOLLOWING IS NEW, AND NEEDS TRIMMING

    f = vopen("MAGICEQ.DAT", "r");
    if (!f) {
        err("Could not open MAGICEQ.DAT.");
    }
    vscanf(f, "%d", &a);
    for (i = 1; i <= a; i++) {
mpl1:
        vscanf(f, "%s", strbuf);
        toUpperCase(strbuf);
        if (!strcmp(strbuf, "//")) {
            vgets(strbuf, 99, f);
            goto mpl1;
        }

        if (!strcmp(strbuf, "EQABLE")) {
meqloop:
            vscanf(f, "%s", strbuf);
            if (!strcmp(strbuf, "-")) {
                continue;
            }
            mequip[i].equipable[atoi(strbuf) - 1] = 1;
            goto meqloop;
        }
        if (!strcmp(strbuf, "LEVEL")) {
mlevloop:
            vscanf(f, "%s", strbuf);
            if (!strcmp(strbuf, "-")) {
                continue;
            }
            i2 = atoi(strbuf);
            mequip[i].level[i2 - 1] = 1;
            goto mlevloop;
        }

    }
    // END NEW
}

void InitItems() {
    printf("initItems\n");
    unsigned char b, i;
    int j;
    auto f = vopen("ITEMICON.DAT", "rb");
    if (!f) {
        err("Could not open ITEMICON.DAT.");
    }
    vread(&b, 1, 1, f);
    vread(itemicons + 256, 256, b, f);
    vclose(f);
    printf("itemicons read ok\n");

    f = vopen("ITEMS.DAT", "r");
    if (!f) {
        err("Could not open ITEMS.DAT.");
    }
    vscanf(f, "%s", strbuf);
    b = atoi(strbuf);
    for (i = 1; i < b + 1; i++) {
        vscanf(f, "%s", items[i].name);
        vscanf(f, "%s", strbuf);
        items[i].icon = atoi(strbuf) + 1;
        vscanf(f, "%s", items[i].desc);
        vscanf(f, "%s", strbuf);
        items[i].useflag = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        items[i].useeffect = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        items[i].itemtype = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        items[i].equipflag = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        items[i].equipidx = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        items[i].itmprv = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        items[i].price = atoi(strbuf);
    }
    vclose(f);
    // *** NEW ***  MAGIC INIT

    f = vopen("MAGICON.DAT", "rb");
    if (!f) {
        err("Could not open MAGICON.DAT.");
    }
    vread(&b, 1, 1, f);
    vread(magicicons + 256, 256, b, f);
    vclose(f);

    f = vopen("MAGIC.DAT", "r");
    if (!f) {
        err("Could not open MAGIC.DAT.");
    }
    vscanf(f, "%s", strbuf);
    b = atoi(strbuf);
    for (i = 1; i < b + 1; i++) {
        vscanf(f, "%s", magic[i].name);
        vscanf(f, "%s", strbuf);
        magic[i].icon = atoi(strbuf) + 1;
        vscanf(f, "%s", magic[i].desc);
        vscanf(f, "%s", strbuf);
        magic[i].useflag = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].useeffect = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].itemtype = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].equipflag = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].equipidx = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].itmprv = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].price = atoi(strbuf);
        vscanf(f, "%s", strbuf);
        magic[i].cost = atoi(strbuf);
    }
    vclose(f);
    printf("magic.dat read ok\n");

    f = vopen("MAGIC.VCS", "rb");
    if (!f) {
        err("Could not open MAGIC.VCS");
    }
    vread(&j, 1, 4, f);
    printf("j = %i\n", j);
    vread(&magicofstbl, 4, j, f);
    printf("magicofstbl\n");
    vread(magicvc, 1, 50000, f);
    printf("magicvc\n");
    vclose(f);
    printf("magic.vcs read ok\n");

    // END NEW

    ProcessEquipDat();

    f = vopen("MISCICON.DAT", "rb");
    if (!f) {
        err("Could not open MISCICON.DAT.");
    }
    vread(&b, 1, 1, f);
    vread(&menuptr, 1, 256, f);
    vread(&itmptr, 1, 576, f);
    vread(&charptr, 1, 960, f);
    vclose(f);
    printf("miscicon read ok\n");

    f = vopen("SPEECH.SPC", "rb");
    if (!f) {
        err("Could not open SPEECH.SPC");
    }
    vread(&b, 1, 1, f);
    vread(speech, b, 1024, f);
    vclose(f);
    printf("speech.spc read ok\n");

    f = vopen("EFFECTS.VCS", "rb");
    if (!f) {
        err("Could not open EFFECTS.VCS");
    }
    vread(&j, 1, 4, f);
    vread(&effectofstbl, 4, j, f);
    vread(effectvc, 1, 50000, f);
    vclose(f);
    printf("effects.vcs read ok\n");

    f = vopen("STARTUP.VCS", "rb");
    if (!f) {
        err("Could not open STARTUP.VCS");
    }
    vread(&j, 1, 4, f);
    vread(&startupofstbl, 4, j, f);
    vread(startupvc, 1, 50000, f);
    vclose(f);
    printf("startup.vcs read ok\n");
}

void StartupMenu() {
    char cursel = 1;
    int i, s;

drawloop:
    menubox(100, 90, 252, 142);
    gotoxy(130, 103);
    printstring("New Game");
    gotoxy(130, 113);
    printstring("Load Game");
    gotoxy(130, 123);
    printstring("Exit to DOS");

    if (!cursel) {
        tcopysprite(110, 102, 16, 16, menuptr);
    }
    if (cursel == 1) {
        tcopysprite(110, 112, 16, 16, menuptr);
    }
    if (cursel == 2) {
        tcopysprite(110, 122, 16, 16, menuptr);
    }

    vgadump();
    while ((down) || (up)) {
        readcontrols();
        wait();
    }

inputloop:
    readcontrols();
    if (down) {
        cursel++;
        if (cursel == 3) {
            cursel = 0;
        }
        playeffect(0);
        goto drawloop;
    }
    if (up) {
        if (!cursel) {
            cursel = 2;
        } else {
            cursel--;
        }
        playeffect(0);
        goto drawloop;
    }

    wait();

    if (!b1) {
        goto inputloop;
    }
    if (cursel == 0) {
        StartNewGame("TEST.MAP");
    }
    if (cursel == 1) {
        LoadSaveErase(0);
    }
    if (cursel == 2) {
        setTimerCount(0);
        s = 91;
fadeloop:
        i = (getTimerCount() * 64) / s;
        i = 64 - i;
        set_intensity(i);
        if (getTimerCount() < s) {
            goto fadeloop;
        }
        set_intensity(0);
        err("");
    }
}

int main(int argc, const char* argv[]) {
    gameRoot = argv[1];

    verge::downloadGame();
    verge::initFileSystem();

    MiscSetup();
    PutOwnerText();
    initvga();
    InitItems();

    while (1) {
        qabort = 0;
        /* -- ric: 01/Jun/98 --
         * These variables set to allow the vc layer functions to work
         * by preventing the engine from trying to draw a non-existant
         * map
         */
        cameratracking = 0;
        layer0 = 0;
        layer1 = 0;
        drawparty = 0;
        drawentities = 0;

        StartupScript();
    }

    return 0;
}
