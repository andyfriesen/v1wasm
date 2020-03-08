// vga.c
// vga device driver

// This source code REALLY needs to be cleaned up. A lot.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <algorithm>

#include <emscripten.h>
#include "control.h"
#include "render.h"
#include "timer.h"
#include "vclib.h"
#include "vga.h"
#include "main.h"
#include "fs.h"

using namespace verge;

#include "engine.h" // for valloc()

void PreCalc_TransparencyFields();

namespace {
    unsigned char realPalette[256 * 3];

    const int BACKBUFFER_SIZE = 90000; // More than we need, but random other things seem to depend on this figure.
}

unsigned char pal[768];
extern char waitvrt, fade, cancelfade, *strbuf;

unsigned char* fnt, *fnt2, *tbox, *n;

namespace vga {
    short int x1 = 17, y1 = 17;
}

unsigned char screen[BACKBUFFER_SIZE];
unsigned char virscr[BACKBUFFER_SIZE];

EM_JS(void, webgl_initvga, (), {
    window.vergeCanvas = document.getElementById('vergeCanvas');
    window.vergeImageData = new ImageData(320, 200);
    window.vergeImageArray = window.vergeImageData.data;
});

EM_JS(void, webgl_vgadump, (unsigned char* frameBuffer, size_t frameBufferSize, unsigned char* palette), {
    const pal = HEAP8.subarray(palette, palette + 768);
    const fb = HEAP8.subarray(frameBuffer, frameBuffer + frameBufferSize);

    const stride = 0;

    for (let y = 0; y < 200; ++y) {
        for (let x = 0; x < 320; ++x) {
            c = fb[srcIndex++];
            const c = 0xFF000000
                | pal[c * 3] << 16
                | pal[c * 3 + 1] << 8
                | pal[c * 3 + 2];

            window.vergeImageArray[destIndex++] = c;
        }
        srcIndex += stride;
    }

    window.vergeCanvas.putImageData(window.vergeImageData, 0, 0);
});

void wait() {
    // vsync?
}

void dump_palette(unsigned char* palette) {
    for (auto q = 0; q < 256; ++q) {
        auto p = palette + q * 3;
        printf("%02X%02X%02X", p[0], p[1], p[2]);
        if ((q % 16) == 15) {
            printf("\n");
        } else {
            printf(", ");
        }
    }
    printf("\n");
}

namespace {
    void set_palette(unsigned char* pall) {
        std::copy(pall, pall + sizeof(pal), realPalette);
        vgadump();
    }

    void get_palette() {
        std::copy(realPalette, realPalette + 768, pal);
    }
}

void paletteMorph(int r, int g, int b, int percent, int intensity) {
    unsigned char pal2[768];

    for (auto i = 0; i < 256; i++) {
        auto wr = pal[(i * 3)];
        auto wg = pal[(i * 3) + 1];
        auto wb = pal[(i * 3) + 2];

        wr = ((wr * percent) + (r * (100 - percent))) / 100;
        wg = ((wg * percent) + (g * (100 - percent))) / 100;
        wb = ((wb * percent) + (b * (100 - percent))) / 100;

        pal2[(i * 3)] = wr * intensity / 63;
        pal2[(i * 3) + 1] = wg * intensity / 63;
        pal2[(i * 3) + 2] = wb * intensity / 63;
    }

    set_palette(pal2);
}

void set_intensity(unsigned n) {
    unsigned char pal2[256 * 3];

    n = std::min<unsigned>(n, 63);
    for (auto i = 0; i < 256 * 3; ++i) {
        pal2[i] = (pal[i] * n) / 63;
    }
    set_palette(pal2);
}

void initvga() {
    webgl_initvga();
    PreCalc_TransparencyFields();
}

void closevga() {
}

void quick_killgfx() {
}

void quick_restoregfx() {
}

void vgaclear() {
    memset(virscr, 0, BACKBUFFER_SIZE);
}

void vgadump() {
    unsigned char bb[BACKBUFFER_SIZE];
    auto dest = bb;
    auto src = getScreenPointer(0, 0);
    for (auto y = 0; y < YRES; ++y) {
        for (auto x = 0; x < XRES; ++x) {
            dest[x] = src[x];
        }
        dest += XRES;
        src += BACKBUFFER_PITCH;
    }

    webgl_vgadump(bb, BACKBUFFER_SIZE, realPalette);
}

void setpixel(int x, int y, char c) {
    virscr[y * BACKBUFFER_PITCH + x] = c;
}

void vline(int x, int y, int y2, char c) {
    auto p = virscr + y * BACKBUFFER_PITCH + x;

    for (auto i = 0; i < (y2 - y); i++) {
        *p = c;
        p += BACKBUFFER_PITCH;
    }
}

void hline(int x, int y, int x2, char c) {
    auto p = virscr + y * BACKBUFFER_PITCH;
    for (auto i = x; i < x2; ++i) {
        p[i] = c;
    }
}

void box(int x, int y, int x2, int y2, char color) {
    int i;

    if (x2 < x) {
        i = x2;
        x2 = x;
        x = i;
    }
    if (y2 < y) {
        i = y2;
        y2 = y;
        y = i;
    }

    for (i = 0; i <= (y2 - y); i++) {
        hline(x, (y + i), x2 + 1, color);
    }
}

void copytile(int x, int y, unsigned char* spr) {
    const auto width = 16;
    auto height = 16;
    auto p = virscr + y * BACKBUFFER_PITCH + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *spr++;
            if (c) {
                p[w] = c;
            }
        }
        p += BACKBUFFER_PITCH;
        --height;
    }
}

void copysprite(int x, int y, int width, int height, unsigned char* spr) {
    auto p = virscr + y * BACKBUFFER_PITCH + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            p[w] = *spr++;
        }
        p += BACKBUFFER_PITCH;
        --height;
    }
}

void grabregion(int x, int y, int width, int height, unsigned char* spr) {
    auto src = getScreenPointer(x - 16, y - 16);
    auto dest = spr;

    while (height--) {
        for (auto i = 0; i < width; ++i) {
            *dest++ = src[i];
        }
        src += BACKBUFFER_PITCH;
    }
}

void tcopytile(int x, int y, unsigned char* spr, unsigned char* matte) {
    const auto width = 16;
    auto height = 16;
    auto p = virscr + y * BACKBUFFER_PITCH + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *spr++;
            if (c) {
                p[w] = c;
            }
        }
        p += BACKBUFFER_PITCH;
        --height;
    }
}

// TODO: Clipping?
void tcopysprite(int x, int y, int width, int height, unsigned char* spr) {
    auto p = virscr + y * BACKBUFFER_PITCH + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *spr++;
            if (c) {
                p[w] = c;
            }
        }
        p += BACKBUFFER_PITCH;
        --height;
    }
}

void fin() {
    int i;

    if (!fade) {
        return;
    }
    if (cancelfade) {
        cancelfade--;
        return;
    }

    setTimerCount(0);
inloop:
    i = (getTimerCount() * 64) / 30;
    set_intensity(i);
    if (getTimerCount() < 30) {
        goto inloop;
    }
    set_intensity(63);
}

void fout() {
    int i;

    if (!fade) {
        return;
    }
    if (cancelfade) {
        cancelfade--;
        return;
    }

    setTimerCount(0);
outloop:
    i = (getTimerCount() * 64) / 30;
    i = 64 - i;
    set_intensity(i);
    if (getTimerCount() < 30) {
        goto outloop;
    }
    set_intensity(0);
}

// Tranparency routines

unsigned char vergepal[768];          // VERGE main palette
unsigned char menuxlatbl[256];        // Menu transparencyfield (blue)
unsigned char greyxlatbl[256];        // Grey transparencyfield
unsigned char scrnxlatbl[256];        // screen transparencyfield
unsigned char* transparencytbl;       // full transparency table (64k)

void ColorScale(unsigned char* dest, int st, int fn, int inv) {
    int i, intensity;

    for (i = 0; i < 256; i++) {
        intensity = vergepal[(i * 3)];
        intensity += vergepal[(i * 3) + 1];
        intensity += vergepal[(i * 3) + 2];
        intensity = intensity * (fn - st) / 192;
        if (inv) {
            dest[i] = fn - intensity;
        } else {
            dest[i] = st + intensity;
        }
    }
}

void PreCalc_TransparencyFields() {
    // First read the VERGE palette from verge.pal
    auto f = vopen("VERGE.PAL", "rb");
    vread(vergepal, 1, 768, f);
    vclose(f);
    transparencytbl = (unsigned char*)valloc(65536, "transparencytbl");

    // Precompute some common translation tables.

    ColorScale(menuxlatbl, 141, 159, 1);
    ColorScale(greyxlatbl, 0, 31, 0);

    // Load in the 64k bitmap-on-bitmap transparency table (precomputed)

    f = vopen("TRANS.TBL", "rb");
    vread(transparencytbl, 1, 65535, f);
    vclose(f);
}

void ColorField(int x, int y, int x2, int y2, unsigned char* tbl) {
    auto height = y2 - y;
    const auto width = x2 - x;

    auto ptr = getScreenPointer(x - 16, y - 16);
    while (height--) {
        for (auto i = 0; i < width; i++) {
            ptr[i] = tbl[ptr[i]];
        }
        ptr += BACKBUFFER_PITCH;
    }
}

void Tcopysprite(int x1, int y1, int width, int height, unsigned char* src) {
    auto p = virscr + y1 * BACKBUFFER_PITCH + x1;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *src++;
            if (c) {
                p[w] = transparencytbl[c * 256 + p[w]];;
            }
        }
        p += BACKBUFFER_PITCH;
        --height;
    }
}

void _Tcopysprite(int x1, int y1, int width, int height, unsigned char* src) {
    unsigned int j, i, jz, iz;
    unsigned char c, d;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            jz = j + y1;
            iz = i + x1;
            c = virscr[(jz * 352) + iz];
            d = src[(j * width) + i];
            if (d) {
                virscr[(jz * 352) + iz] = transparencytbl[(c * 256) + d];
            }
        }
    }
}

// Font routines

char oc = 31;

void LoadFont() {
    VFILE* f;

    fnt = (unsigned char*)valloc(6000, "fnt");
    fnt2 = (unsigned char*)valloc(14000, "fnt2");
    tbox = (unsigned char*)valloc(30000, "tbox");
    if (!(f = vopen("SMALL.FNT", "rb"))) {
        err("FATAL ERROR: Could not open SMALL.FNT.");
    }
    vread(fnt, 63, 95, f);
    vclose(f);
    if (!(f = vopen("MAIN.FNT", "rb"))) {
        err("FATAL ERROR: Could not open MAIN.FNT.");
    }
    vread(fnt2, 144, 95, f);
    vclose(f);
    if (!(f = vopen("BOX.RAW", "rb"))) {
        err("FATAL ERROR: Could not open BOX.RAW.");
    }
    vread(tbox, 320, 66, f);
    vclose(f);
}

void pchar(int x, int y, char c) {
    if ((c < 32) || (c > 126)) {
        return;
    }
    unsigned char* img = fnt + ((c - 32) * 63);
    if ((c == 103) || (c == 106) || (c == 112) || (c == 113) || (c == 121)) {
        tcopysprite(x, y + 2, 7, 9, img);
    } else {
        tcopysprite(x, y, 7, 9, img);
    }
}

void VCpchar(int x, int y, char c) {
    if ((c < 32) || (c > 126)) {
        return;
    }
    unsigned char* img = fnt + ((c - 32) * 63);
    if ((c == 103) || (c == 106) || (c == 112) || (c == 113) || (c == 121)) {
        VCtcopysprite(x, y + 2, 7, 9, img);
    } else {
        VCtcopysprite(x, y, 7, 9, img);
    }
}

void bigpchar(int x, int y, char c) {
    if ((c < 32) || (c > 126)) {
        return;
    }
    unsigned char* img = fnt2 + ((c - 32) * 144);
    if ((c == 103) || (c == 106) || (c == 112) || (c == 113) || (c == 121)) {
        tcopysprite(x, y + 2, 9, 16, img);
    } else {
        tcopysprite(x, y, 9, 16, img);
    }
}

void gotoxy(int x, int y) {
    vga::x1 = x;
    vga::y1 = y;
}

void printstring(const char* str) {
    int i;
    char c;

    i = 0;
    if (!str[0]) {
        return;
    }

mainloop:
    c = str[i];
    pchar(vga::x1, vga::y1, c);
    vga::x1 = vga::x1 + 8;
    i++;
    if (str[i] != 0) {
        goto mainloop;
    }
}

void VCprintstring(int xx, int yy, const char* str) {
    int i;
    char c;

    i = 0;
    if (!str[0]) {
        return;
    }

mainloop:
    c = str[i];
    VCpchar(xx, yy, c);
    xx = xx + 8;
    i++;
    if (str[i] != 0) {
        goto mainloop;
    }
}

void bigprintstring(const char* str) {
    int i;
    char c;

    i = 0;
    if (!str[0]) {
        return;
    }

mainloop:
    c = str[i];
    bigpchar(vga::x1, vga::y1, c);
    vga::x1 = vga::x1 + 10;
    i++;
    if (str[i] != 0) {
        goto mainloop;
    }
}

void putbox() {
    ColorField(18, 151, 334, 213, menuxlatbl);
    tcopysprite(16, 149, 320, 66, tbox);

    //  border(18,149,333,213);
}

void dec_to_asciiz(int num, char* buf) {
    sprintf(buf, "%i", num);
}

void textwindow(char portrait, const char* str1, const char* str2, const char* str3) {
    tcopysprite(20, 114, 32, 32, speech + (portrait * 1024));
    putbox();
    gotoxy(25, 155);
    bigprintstring(str1);
    gotoxy(25, 174);
    bigprintstring(str2);
    gotoxy(25, 193);
    bigprintstring(str3);
}

void fontcolor(unsigned char c) {
    int i;

    unsigned char* ptr = fnt;
    for (i = 0; i < 5985; i++) {
        if (*ptr == oc) {
            *ptr = c;
        }
        ptr++;
    }
    oc = c;
}
