// engine.c
// All core engine code
// Copyright (C)1997 BJ Eirich

#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>
#include "control.h"
#include "entity.h"
#include "engine.h"
#include "fs.h"
#include "keyboard.h"
#include "main.h"
#include "menu.h"
#include "render.h"
#include "sound.h"
#include "timer.h"
#include "ricvc.h"
#include "vga.h"

using namespace verge;

// ============================ Data ============================

struct zoneinfo {                      // zone data
    char zonename[15];                   // zone description
    unsigned short int callevent;        // event number to call
    unsigned char percent;               // chance (in 255) of event occurance
    unsigned char delay;                 // step-delay before last occurance
    unsigned char aaa;                   // accept adjacent activation
    char savedesc[30];                   // savegame description
};

// Useflag values:   0 for nonusable.
//                   1 for usable once, then it goes away.
//                   2 for usable an infinite number of times.
//                   3 for usable ONLY camped, only once.
//                   4 for usable ONLY camped, infinitely
//                   5 for usable in combat only, once, then it goes away.
//                   6 for usable in combat only, indefinetly.

// Itemtype values:  0 for OS. (Own Self)
//                   1 for 1A. (One Ally)
//                   2 for AA. (All Allies)
//                   3 for 1E. (One Enemy)
//                   4 for AE. (All Enemy)

char lastmoves[6];                     // the party leader's last six moves
char partyidx[5];                      // current character-ref indexes
char numchars = 0;                     // number of characters to draw
struct vspanim va0[100];               // tile animation data
struct r_entity party[101];            // party entities
struct zoneinfo zone[128];             // entity records
struct charstats pstats[30];           // party-stats record
struct itemdesc items[255];            // item definition array

struct magicdesc magic[255];
unsigned char* magicicons;        // more graphic ptrs
struct mequipstruc mequip[255];          // equipment definition array

// END NEW

#include <string.h>

struct equipstruc equip[255];          // equipment definition array
char zonedelay, lz;                    // zone delay counter
char tchars;                           // total characters in PARTY.DAT
int gp;                                // party's gold (or meseta, s, $, etc)

unsigned short int vadelay[100];       // delay counter

std::vector<short> map0; // Tile data
std::vector<short> map1;
std::vector<unsigned char> mapp;

std::vector<uint8_t> vsp0;

unsigned char* chrs;      // graphic data pointers
unsigned char* itemicons, *chr2;       // more graphic ptrs

/* Not sure what's correct here.
 * Originally, the header declared "short int flags[8000]"
 * but engine.cpp has always said "signed int flags[8000]"
 * The save format writes and reads enough storage for 8000 32 bit flags,
 * so I _think_ this is what we really get.
 * -- andy 16 August 2011
 */
short int flags[16000];              // misc flags

VFILE* map;
VFILE* vsp;                       // file handles
char mapname[13], musname[13];         // map/music filenames
char vsp0name[13], levelname[30];      // vsp filename / level name
unsigned char layerc;
char showname;
unsigned char saveflag;       // control flags

/* -- ric: 21/Apr/98 - moved to process_controls -- */
// char l,r,t,b;                          // wall status

int xwin, ywin;                        // window coordinates
int xtc, ytc, xofs, yofs;              // variables for drawmap()
unsigned short int xsize, ysize;       // x/y map dimensions
unsigned short int startx, starty;     // default start location
unsigned char pmultx, pdivx;           // parallax speed X
unsigned char pmulty, pdivy;           // parallax speed Y
char warp, hide;                       // map warpable / hidable flags

char dontshowname = 0, usenxy = 0, qabort = 0;
unsigned char autoent = 0;
unsigned short int nx = 0, ny = 0;
short int numtiles;

extern char* strbuf, *msbuf, menuactive;
extern unsigned char nummovescripts;
//extern int mapvcm; // -- aen; 31/May/98 -- no longer used; see LoadVC().
extern int msofstbl[100];
extern int vspm; // -- aen; 30/May/98 -- no longer used; see load_map().
//extern int mapm; // -- aen; 30/May/98 -- no longer used; see load_map().

void LoadVC(VFILE* map);
void CalcVSPMask();

// ============================ Code ============================

void* valloc(int amount, const char* whatfor) {
    /* -- ric: 10/May/98 --
     * Well actually this is aen's code to replace malloc. It'll help
     * with debugging :)
     */
    /* -- aen: 10/May-98--
     * Well actually this is vec's idea. I just extended it a little.
     */
    char* tmp;
    static char debug_buf[256];
    tmp = (char*)malloc(amount);
    if (!tmp) {
        sprintf(debug_buf, "Failed allocating %i bytes for %s.", amount, whatfor);
        err(debug_buf);
    }
    memset(tmp, 0, amount); // -- aen: 30/May/98; clear allocated mem
    return tmp;
}

void vfree(void* thismem) {
    if (thismem) {
        free(thismem);
    }
    thismem = NULL;
}

void InitVCMem();

void allocbuffers() {
    //map0 = valloc(mapm*2, "map0"); // -- aen; 30/May/98 -- removed; this is
    //map1 = valloc(mapm*2, "map1"); // -- taken care of automatically within
    //mapp = valloc(mapm, "mapp");   // -- load_map().

    //vsp0 = valloc(vspm, "vsp0");
    chrs = (unsigned char*)valloc(512000, "chrs");
    chr2 = (unsigned char*)valloc(46080, "chr2");

    // -- NichG; ??/??/?? --
    magicicons = (unsigned char*)valloc(50688, "magicicons");

    itemicons = (unsigned char*)valloc(50688, "itemicons");
    speech = (unsigned char*)valloc(450000, "speech");
    msbuf = (char*)valloc(20000, "msbuf");

    //memset(map0,0,mapm*2);     /* -- ric:28/Apr/98 --                 */
    //memset(map1,0,mapm*2);     /* Clear all the allocated memory just */
    //memset(mapp,0,mapm);       /* in case.                            */
    //memset(vsp0,0,vspm);
    //memset(chrs,0,512000);     // -- aen; 30/May/98 -- removed; this is
    //memset(chr2,0,46080);      // -- automatically taken care of by valloc()
    //memset(speech,0,450000);   // -- now.
    //memset(msbuf,0,20000);
    //memset(flags,0,32000);
    //memset(itemicons,0,50688);
    //memset(&party, 0, sizeof party);

    InitVCMem();
}

void addcharacter(int i) {
    VFILE* chrf;
    VFILE* p;
    int b;

    numchars++;
    partyidx[numchars - 1] = i;
    p = vopen("PARTY.DAT", "r");
    if (!p) {
        err("addCharacter(): Fatal error: PARTY.DAT not found");
    }
    vscanf(p, "%s", strbuf);
    tchars = atoi(strbuf);
    if (i > atoi(strbuf)) {
        err("addcharacter(): index out of range");
    }
    for (b = 1; b < i; b++) {
        vscanf(p, "%s", strbuf);
        vscanf(p, "%s", strbuf);
        vscanf(p, "%s", strbuf);
    }
    vscanf(p, "%s", strbuf);

    party[0].chrindex = 0;

    party[1].x = party[0].x;
    party[1].y = party[0].y;
    party[1].cx = party[0].cx;
    party[1].cy = party[0].cy;
    party[1].facing = 0;
    party[1].moving = 0;
    party[1].chrindex = 1;

    party[2].x = party[0].x;
    party[2].y = party[0].y;
    party[2].cx = party[0].cx;
    party[2].cy = party[0].cy;
    party[2].facing = 0;
    party[2].moving = 0;
    party[2].chrindex = 2;

    party[3].x = party[0].x;
    party[3].y = party[0].y;
    party[3].cx = party[0].cx;
    party[3].cy = party[0].cy;
    party[3].facing = 0;
    party[3].moving = 0;
    party[3].chrindex = 3;

    party[4].x = party[0].x;
    party[4].y = party[0].y;
    party[4].cx = party[0].cx;
    party[4].cy = party[0].cy;
    party[4].facing = 0;
    party[4].moving = 0;
    party[4].chrindex = 4;

    lastmoves[0] = 0;
    lastmoves[1] = 0;
    lastmoves[2] = 0;
    lastmoves[3] = 0;
    lastmoves[4] = 0;
    lastmoves[5] = 0;
    chrf = vopen(pstats[i - 1].chrfile, "rb");
    if (!chrf) {
        err("addcharacter(): CHR file not found");
    }
    vread(chrs + ((numchars - 1) * 15360), 1, 15360, chrf);
    vclose(chrf);

    vscanf(p, "%s", strbuf);
    vclose(p);
    chrf = vopen(strbuf, "rb");
    if (!chrf) {
        err("addcharacter(): CR2 file not found");
    }
    vread(chr2 + ((numchars - 1) * 9216), 1, 9216, chrf);
    vclose(chrf);
}

void LoadCHRList() {
    int i;
    VFILE* f;

    for (i = 0; i < 20; i++) {
        if (strlen(chrlist[i].fname)) {
            f = vopen(chrlist[i].fname, "rb");
            if (!f) {
                printf("Unable to load CHR %s\n", chrlist[i].fname);
                memset(chrs + (i + 5) * 15360, 0, 30 * 512);
                continue;
            }
            vread(chrs + ((i + 5) * 15360), 30, 512, f);
            vclose(f);
        }
    }
}

void load_map(char* fname) {
    printf("load_map %s\n", fname);

    unsigned char b;
    int i;

    memcpy(mapname, fname, 13);
    map = vopen(fname, "rb");
    if (!map) {
        err("Could not open specified MAP file %s.", fname);
    }

    vread(&b, 1, 1, map);
    if (b != 4) {
        err("*error* Incorrect MAP version %i.", b);
    }

    vread(vsp0name, 1, 13, map);
    vread(musname, 1, 13, map);

    vread(&layerc, 1, 1, map);
    vread(&pmultx, 1, 1, map); pmulty = pmultx;
    vread(&pdivx, 1, 1, map);  pdivy = pdivx;

    vread(levelname, 1, 30, map);

    vread(&showname, 1, 1, map);
    vread(&saveflag, 1, 1, map);

    vread(&startx, 1, 2, map);
    vread(&starty, 1, 2, map);

    vread(&hide, 1, 1, map);
    vread(&warp, 1, 1, map);

    vread(&xsize, 1, 2, map);
    vread(&ysize, 1, 2, map);

    vread(&b, 1, 1, map);
    if (b) {
        err("*error* MAP compression not yet supported.");
    }

    vseek(map, 27, SEEK_CUR); // skip buffer

    // -- aen; 30/May/98 -- dynamic map mem allocation
    // free previously allocate layer/info map mem (if necessary)
    map0.resize(xsize * ysize);
    map1.resize(xsize * ysize);
    mapp.resize(xsize * ysize);

    vread(&map0[0], xsize, ysize * 2, map); // read in background layer
    vread(&map1[0], xsize, ysize * 2, map); // read in foreground layer
    vread(&mapp[0], xsize, ysize, map);   // read in zone info/obstruction layer

    vread(&zone, 1, sizeof(zone), map);
    vread(&chrlist, 13, 100, map);      // read in chr list

    LoadCHRList();

    vread(&entities, 1, 4, map);
    vread(&party[5], 88, entities, map);

    for (i = 5; i < entities + 5; i++) {
        party[i].cx = party[i].x;
        party[i].cy = party[i].y;
        party[i].x = party[i].x * 16;
        party[i].y = party[i].y * 16;
    }

    vread(&nummovescripts, 1, 1, map);
    vread(&i, 1, 4, map);
    vread(&msofstbl, 4, nummovescripts, map);
    vread(msbuf, 1, i, map);

    entities += 5;

    LoadVC(map);
    vclose(map);

    if (strlen(musname)) {
        playsong(musname);
    }

    // load the .VSP file
    vsp = vopen(vsp0name, "rb");
    if (!vsp) {
        err("Could not open specified VSP file.");
    }
    vseek(vsp, 2, 0);
    vread(pal, 1, 768, vsp);
    for (auto q = 0; q < 768; ++q) {
        pal[q] <<= 2;
    }
    vread(&numtiles, 1, 2, vsp);

    vspm = numtiles << 8;
    vsp0.resize(vspm);

    vread(vsp0.data(), 1, vspm, vsp);
    vread(&va0, 1, sizeof(va0), vsp);
    vclose(vsp);

    for (i = 0; i < 2048; i++) {
        tileidx[i] = i;
    }

    if (vspspeed) {
        CalcVSPMask();
    }

    if (usenxy) {
        startx = nx;
        starty = ny;
    }

    party[0].x = startx * 16;
    party[0].y = starty * 16;
    party[0].moving = 0;
    party[0].cx = startx;
    party[0].cy = starty;

    party[1].x = party[0].x;
    party[1].y = party[0].y;
    party[1].moving = 0;
    party[1].cx = startx;
    party[1].cy = starty;

    party[2].x = party[0].x;
    party[2].y = party[0].y;
    party[2].moving = 0;
    party[2].cx = startx;
    party[2].cy = starty;

    party[3].x = party[0].x;
    party[3].y = party[0].y;
    party[3].moving = 0;
    party[3].cx = startx;
    party[3].cy = starty;

    party[4].x = party[0].x;
    party[4].y = party[0].y;
    party[4].moving = 0;
    party[4].cx = startx;
    party[4].cy = starty;

    lastmoves[0] = 0;
    lastmoves[1] = 0;
    lastmoves[2] = 0;
    lastmoves[3] = 0;
    lastmoves[4] = 0;
    lastmoves[5] = 0;

    ExecuteScript(0);
}

void process_stepzone() {
    // Called each time the player steps in a new square. Looks up current zone
    // information, adjusts for delays and then makes a chance roll to see if
    // an event is called.

    const auto x = party[0].x / 16;
    const auto y = party[0].y / 16;
    const auto cz = mapp[y * xsize + x] >> 1; // low bit is obstruction data
    const auto& z = zone[cz];

    if (lz != cz) {
        zonedelay = 0;
        lz = cz;
    }
    if (!z.percent) {
        return;
    }
    if (zonedelay < z.delay) {
        zonedelay++;
        return;
    }

    const auto randomNumber = rand() & 255;
    if (randomNumber <= z.percent) {
        ExecuteScript(z.callevent);
        zonedelay = 0;
    }
}

void lastmove(char n) {
    // moves all the entrees in the lastmoves array down one, then places the
    // new direction at the front.

    lastmoves[5] = lastmoves[4];
    lastmoves[4] = lastmoves[3];
    lastmoves[3] = lastmoves[2];
    lastmoves[2] = lastmoves[1];
    lastmoves[1] = lastmoves[0];
    lastmoves[0] = n;
}

void startfollow() {
    for (int i = 1; i < numchars; i++) {
        if (lastmoves[i] == 1) {
            party[i].y++;
            party[i].facing = 0;
            party[i].cy++;
            party[i].moving = 1;
            party[i].movcnt = 15;
        }
        if (lastmoves[i] == 2) {
            party[i].y--;
            party[i].facing = 1;
            party[i].cy--;
            party[i].moving = 2;
            party[i].movcnt = 15;
        }
        if (lastmoves[i] == 3) {
            party[i].x++;
            party[i].facing = 2;
            party[i].cx++;
            party[i].moving = 3;
            party[i].movcnt = 15;
        }
        if (lastmoves[i] == 4) {
            party[i].x--;
            party[i].facing = 3;
            party[i].cx--;
            party[i].moving = 4;
            party[i].movcnt = 15;
        }
    }
}

int InvFace() {
    switch (party[0].facing) {
        case 0: return 1;
        case 1: return 0;
        case 2: return 3;
        case 3: return 2;
        default: return 0;
    }
}

void Activate() {
    int tx, ty;
    unsigned char cz, t;

    // First, determine what tile we're looking at.
    tx = party[0].x / 16;
    ty = party[0].y / 16;
    switch (party[0].facing) {
    case 0: {
            ty++;
            break;
        }
    case 1: {
            ty--;
            break;
        }
    case 2: {
            tx++;
            break;
        }
    case 3: {
            tx--;
            break;
        }
    }

    // Here is where we first check for entities in our way.

    t = EntityAt(tx, ty);
    if (t)
        if (!party[t].activmode && party[t].actscript) {
            if (party[t].face) {
                party[t].facing = InvFace();
            }
            ExecuteScript(party[t].actscript);
            return;
        }

    // Check adjacent zones.
    cz = mapp[((ty * xsize) + tx)] >> 1;
    if (zone[cz].aaa) {
        ExecuteScript(zone[cz].callevent);
        return;
    }
}

int ObstructionAt(int tx, int ty) {
    if ((mapp[((ty)*xsize) + tx] & 1) == 1) {
        return 1;
    }
    return 0;
}

void process_entities() {
    int i;

    for (i = 5; i < entities; i++) {
        ProcessSpeedAdjEntity(i);
    }

    if (autoent)
        for (i = 95; i < 95 + numchars; i++) {
            ProcessSpeedAdjEntity(i);
        }
}

void process_controls();

void ProcessControls() {
    const auto effectiveSpeed = goFastButton ? 6 : party[0].speed;

    if (effectiveSpeed < 4) {
        switch (effectiveSpeed) {
        case 1:
            if (party[0].speedct < 3) {
                party[0].speedct++;
                return;
            }
        case 2:
            if (party[0].speedct < 2) {
                party[0].speedct++;
                return;
            }
        case 3:
            if (party[0].speedct < 1) {
                party[0].speedct++;
                return;
            }
        }
    }
    if (effectiveSpeed < 5) {
        process_controls();
    }
    switch (effectiveSpeed) {
    case 5:
        process_controls();
        process_controls();
        return;
    case 6:
        process_controls();
        process_controls();
        process_controls();
        return;
    case 7:
        process_controls();
        process_controls();
        process_controls();
        process_controls();
        return;
    }
}

void process_controls() {
    party[0].speedct = 0;
    if (!party[0].moving) {
        xtc = party[0].x / 16;
        ytc = party[0].y / 16;

        auto cheatHax = 0 && keyboard_map[SCAN_TILDE];

        bool downOk = cheatHax || !ObstructionAt(xtc, ytc + 1);
        bool upOk = cheatHax || !ObstructionAt(xtc, ytc - 1);
        bool rightOk = cheatHax || !ObstructionAt(xtc + 1, ytc);
        bool leftOk  = cheatHax || !ObstructionAt(xtc - 1, ytc);

        if (xtc == 0) {
            leftOk = 0;
        }
        if (ytc == 0) {
            upOk = 0;
        }
        if (xtc == xsize - 1) {
            rightOk = 0;
        }
        if (ytc == ysize - 1) {
            downOk = 0;
        }

        readcontrols_noSleep();
        if (b1) {
            Activate();
        }
        if (b3) {
            SystemMenu();
        }
        if (b4) {
            MainMenu();
        }

        for (auto i = 0; i < 128; i++) { /* -- ric: 03/May/98 -- */
            if (key_map[i].pressed && key_map[i].boundscript) {
                ExecuteStartUpScript(key_map[i].boundscript);
            }
        }

        xtc = party[0].x / 16;
        ytc = party[0].y / 16;

        if (right) {
            party[0].facing = 2;
        }
        if (down) {
            party[0].facing = 0;
        }
        if (left) {
            party[0].facing = 3;
        }
        if (up) {
            party[0].facing = 1;
        }

        if (right && rightOk && !EntityAt(xtc + 1, ytc)) {
            party[0].x++;
            party[0].facing = 2;
            party[0].moving = 3;
            party[0].cx++;
            party[0].movcnt = 15;
            lastmove(3);
            startfollow();
            return;
        }
        if (down && downOk && !EntityAt(xtc, ytc + 1)) {
            party[0].y++;
            party[0].facing = 0;
            party[0].moving = 1;
            party[0].cy++;
            party[0].movcnt = 15;
            lastmove(1);
            startfollow();
            return;
        }
        if (left && leftOk && !EntityAt(xtc - 1, ytc)) {
            party[0].x--;
            party[0].facing = 3;
            party[0].moving = 4;
            party[0].cx--;
            party[0].movcnt = 15;
            lastmove(4);
            startfollow();
            return;
        }
        if (up && upOk && !EntityAt(xtc, ytc - 1)) {
            party[0].y--;
            party[0].facing = 1;
            party[0].moving = 2;
            party[0].cy--;
            party[0].movcnt = 15;
            lastmove(2);
            startfollow();
            return;
        }

        party[0].framectr = 0;
        party[1].framectr = 0;
        party[2].framectr = 0;
        party[3].framectr = 0;
        party[4].framectr = 0;
    }

    if (party[0].moving) {
        for (auto i = 0; i < numchars; i++) {
            if (party[i].moving == 1) {
                party[i].y++;
            }
            if (party[i].moving == 2) {
                party[i].y--;
            }
            if (party[i].moving == 3) {
                party[i].x++;
            }
            if (party[i].moving == 4) {
                party[i].x--;
            }
            party[i].movcnt--;
            party[i].framectr++;
            if (party[i].framectr == 80) {
                party[i].framectr = 0;
            }
        }
        if (!party[0].movcnt) {
            party[0].moving = 0;
            process_stepzone();
        }
    }
}

void check_tileanimation() {
    unsigned char i;

    for (i = 0; i < 100; i++) {
        if ((va0[i].delay) && (va0[i].delay < vadelay[i])) {
            animate(i);
        }
        vadelay[i]++;
    }
}

void UpdateEquipStats() {
    int i, j, a;

    // This function takes the base stats of all characters and re-calculates
    // current-stats by re-applying equipment modifiers.

    for (i = 0; i < 30; i++) {
        pstats[i].atk = pstats[i].str;
        pstats[i].def = pstats[i].end;
        pstats[i].magc = pstats[i].mag;
        pstats[i].mgrc = pstats[i].mgr;
        pstats[i].hitc = pstats[i].hit;
        pstats[i].dodc = pstats[i].dod;
        pstats[i].mblc = pstats[i].mbl;
        pstats[i].ferc = pstats[i].fer;
        pstats[i].reac = pstats[i].rea;

        for (j = 0; j < 6; j++) {
            a = items[pstats[i].inv[j]].equipidx;
            pstats[i].atk += equip[a].str;
            pstats[i].def += equip[a].end;
            pstats[i].magc += equip[a].mag;
            pstats[i].mgrc += equip[a].mgr;
            pstats[i].hitc += equip[a].hit;
            pstats[i].dodc += equip[a].dod;
            pstats[i].mblc += equip[a].mbl;
            pstats[i].ferc += equip[a].fer;
            pstats[i].reac += equip[a].rea;
        }
    }
}

void game_ai() {                     // this is THE main loop
    ProcessControls();
    process_entities();
    if (keyboard_map[SCAN_CTRL] && speed) {
        ProcessControls();
    }
    if (moneycheat) {
        gp = 100000;
    }
    check_tileanimation();
}

void CreateSaveImage(unsigned char* buf) {
    memcpy(buf, chrs, 512);
    if (numchars > 1) {
        memcpy(buf + 512, chrs + 15360, 512);
    } else {
        memset(buf + 512, 0, 512);
    }
    if (numchars > 2) {
        memcpy(buf + 1024, chrs + 30720, 512);
    } else {
        memset(buf + 1024, 0, 512);
    }
    if (numchars > 3) {
        memcpy(buf + 1536, chrs + 46080, 512);
    } else {
        memset(buf + 1536, 0, 512);
    }
    if (numchars > 4) {
        memcpy(buf + 2048, chrs + 61440, 512);
    } else {
        memset(buf + 2048, 0, 512);
    }
}

namespace verge {
    extern std::string saveGameRoot; // main.cpp
}

void SaveGame(char* fn) {
    FILE* f;
    unsigned char cz;
    unsigned char temp[2560];

    std::string realPath = verge::saveGameRoot + fn;
    f = fopen(realPath.c_str(), "wb");
    cz = mapp[(((party[0].y / 16) * xsize) + (party[0].x / 16))] >> 1;
    fwrite(&zone[cz].savedesc, 1, 30, f);
    cz = partyidx[0] - 1;
    fwrite(&pstats[cz].name, 1, 9, f);
    fwrite(&pstats[cz].lv, 1, 4, f);
    fwrite(&pstats[cz].curhp, 1, 8, f);
    fwrite(&gp, 1, 4, f);
    fwrite(&hr, 1, 1, f);
    fwrite(&min, 1, 1, f);
    fwrite(&sec, 1, 1, f);
    fwrite(&numchars, 1, 1, f);
    fwrite(&menuactive, 1, 1, f);
    CreateSaveImage((unsigned char*)&temp);
    fwrite(&temp, 1, 2560, f);
    fwrite(&mapname, 1, 13, f);
    fwrite(&party, 1, sizeof party, f);
    fwrite(&partyidx, 1, 5, f);
    fwrite(&flags, 1, 32000, f);
    fwrite(&tchars, 1, 1, f);
    fwrite(&pstats, 1, sizeof pstats, f);
    fclose(f);

    // WASM
    {
        EM_ASM(
            FS.syncfs(false, err => {
                if (err) {
                    console.error("SaveGame failed!!", err);
                }
            });
        );
    }

    // NaCl
    // {
    //     auto f = vopen(fn, "r");
    //     verge::plugin->persistSave(fn, f->getData());
    //     vclose(f);
    // }
}

void startmap(char* fname) {
    if (qabort) {
        return;
    }

    set_intensity(0);
    load_map(fname);
    drawmap();
    vgadump();
    fin();

    setTimerCount(0);
    lz = 0;
    tickctr = 0;
    zonedelay = 0;

    while (!qabort) {
        int q = getTimerCount();
        while (q > 0) {
            --q;
            game_ai();
        }
        setTimerCount(0);

        drawmap();
        vgadump();
    }
}
