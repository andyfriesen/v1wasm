// vga.c
// vga device driver

// This source code REALLY needs to be cleaned up. A lot.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <algorithm>
#include "ppapi/cpp/graphics_2d.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/completion_callback.h"
#include "control.h"
#include "render.h"
#include "timer.h"
#include "vclib.h"
#include "vga.h"
#include "main.h"
#include "fs.h"
#include "nacl.h"

using namespace verge;

#include "engine.h" // for valloc()

namespace verge {
    IFramebuffer* plugin = 0;
}

namespace {
    unsigned char realPalette[256 * 3];

    const int XRES = 320;
    const int YRES = 200;
    const int BACKBUFFER_SIZE = 90000; // More than we need, but random other things seem to depend on this figure.
}

unsigned char pal[768];
unsigned char pal2[768];
extern char waitvrt, fade, cancelfade, *strbuf;

unsigned char* fnt, *fnt2, *tbox, *n;
short int x1 = 17, y1 = 17;

unsigned char screen[BACKBUFFER_SIZE];
unsigned char virscr[BACKBUFFER_SIZE];

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

void set_palette(unsigned char* pall) {
    std::copy(pall, pall + sizeof(pal), realPalette);
}

void get_palette() {
}

void set_intensity(unsigned n) {
    n = std::min<unsigned>(n, 63);
    for (auto i = 0; i < 256 * 3; ++i) {
        pal2[i] = (pal[i] * n) / 63;
    }
    set_palette(pal2);
    vgadump();
}

void initvga(IFramebuffer* fb) {
    plugin = fb;
}

void closevga() {
}

void quick_killgfx() {
}

void quick_restoregfx() {
}

void vgadump() {
    if (plugin) {
        plugin->vgadump(virscr, realPalette);
    }
}

void setpixel(int x, int y, char c) {
    virscr[y * 320 + x] = c;
}

void vline(int x, int y, int y2, char c) {
    auto p = virscr + y * 320 + x;

    for (auto i = 0; i < (y2 - y); i++) {
        *p = c;
        p += 320;
    }
}

void hline(int x, int y, int x2, char c) {
    auto p = virscr + y * 320;
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

/*
void copytile(int x, int y, char *spr)
{ asm("movl $16, %%edx                  \n\t"
      "movl %2, %%esi                   \n\t"
      "movl %1, %%edi                   \n\t"
      "imul $352, %%edi                 \n\t"
      "addl %0, %%edi                   \n\t"
      "addl _virscr, %%edi              \n\t"
"ctl0:                                  \n\t"
      "movsl                            \n\t"
      "movsl                            \n\t"
      "movsl                            \n\t"
      "movsl                            \n\t"
      "addl $336, %%edi                 \n\t"
      "decl %%edx                       \n\t"
      "jnz ctl0                         \n\t"
      :
      : "m" (x), "m" (y), "m" (spr)
      : "edx","esi","edi","cc" );
}
*/

void copytile(int x, int y, unsigned char* spr) {
    const auto width = 16;
    auto height = 16;
    auto p = virscr + y * 320 + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *spr++;
            if (c) {
                p[w] = c;
            }
        }
        p += 320;
        --height;
    }
#if 0
    asm("movl $16, %%ecx                  \n\t"
        "movl %2, %%esi                   \n\t"
        "movl %1, %%edi                   \n\t"
        "imul $352, %%edi                 \n\t"
        "addl %0, %%edi                   \n\t"
        "addl _virscr, %%edi              \n\t"
        " ctl0:                                 \n\t"
        "movl (%%edi), %%eax              \n\t"
        "andl $0, %%eax                   \n\t"
        "orl  (%%esi), %%eax              \n\t"
        "movl %%eax, (%%edi)              \n\t"
        "movl 4(%%edi), %%eax             \n\t"
        "andl $0, %%eax                   \n\t"
        "orl  4(%%esi), %%eax             \n\t"
        "movl %%eax, 4(%%edi)             \n\t"
        "movl 8(%%edi), %%eax             \n\t"
        "andl $0, %%eax                   \n\t"
        "orl  8(%%esi), %%eax             \n\t"
        "movl %%eax, 8(%%edi)             \n\t"
        "movl 12(%%edi), %%eax            \n\t"
        "andl $0, %%eax                   \n\t"
        "orl  12(%%esi), %%eax            \n\t"
        "movl %%eax, 12(%%edi)            \n\t"
        "addl $16, %%esi                  \n\t"
        "addl $352, %%edi                 \n\t"
        "decl %%ecx                       \n\t"
        "jnz ctl0                         \n\t"
        :
        : "m" (x), "m" (y), "m" (spr)
        : "eax", "ecx", "esi", "edi", "cc" );
#endif
}

void copysprite(int x, int y, int width, int height, unsigned char* spr) {
#if 0
    asm("movl %3, %%edx                   \n\t"
        "movl %4, %%esi                   \n\t"
        "csl0:                                  \n\t"
        "movl %1, %%eax                   \n\t"
        "imul $352, %%eax                 \n\t"
        "addl %0, %%eax                   \n\t"
        "addl _virscr, %%eax              \n\t"
        "movl %%eax, %%edi                \n\t"
        "movl %2, %%ecx                   \n\t"
        "shrl $1, %%ecx                   \n\t"
        "jnc csl1                         \n\t"
        "movsb                            \n\t"
        "csl1:                                  \n\t"
        "repz                             \n\t"
        "movsw                            \n\t"
        "incl %1                          \n\t"
        "decl %%edx                       \n\t"
        "jnz csl0                         \n\t"
        :
        : "m" (x), "m" (y), "m" (width), "m" (height), "m" (spr)
        : "eax", "edx", "esi", "edi", "ecx", "cc" );
#endif
}

void grabregion(int x, int y, int width, int height, unsigned char* spr) {
#if 0
    asm("movl %3, %%edx                   \n\t"
        "movl %4, %%edi                   \n\t"
        "grl0:                                  \n\t"
        "movl %1, %%eax                   \n\t"
        "imul $352, %%eax                 \n\t"
        "addl %0, %%eax                   \n\t"
        "addl _virscr, %%eax              \n\t"
        "movl %%eax, %%esi                \n\t"
        "movl %2, %%ecx                   \n\t"
        "shrl $1, %%ecx                   \n\t"
        "jnc grl1                         \n\t"
        "movsb                            \n\t"
        "grl1:                                  \n\t"
        "repz                             \n\t"
        "movsw                            \n\t"
        "incl %1                          \n\t"
        "decl %%edx                       \n\t"
        "jnz grl0                         \n\t"
        :
        : "m" (x), "m" (y), "m" (width), "m" (height), "m" (spr)
        : "eax", "edx", "esi", "edi", "ecx", "cc" );
#endif
}

void tcopytile(int x, int y, unsigned char* spr, unsigned char* matte) {
    const auto width = 16;
    auto height = 16;
    auto p = virscr + y * 320 + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *spr++;
            if (c) {
                p[w] = c;
            }
        }
        p += 320;
        --height;
    }
#if 0
    asm("movl $16, %%ecx                  \n\t"
        "movl %2, %%esi                   \n\t"
        "movl %1, %%edi                   \n\t"
        "imul $352, %%edi                 \n\t"
        "addl %0, %%edi                   \n\t"
        "addl _virscr, %%edi              \n\t"
        "movl %3, %%edx                   \n\t"
        "tctl0:                                 \n\t"
        "movl (%%edi), %%eax              \n\t"
        "andl (%%edx), %%eax              \n\t"
        "orl  (%%esi), %%eax              \n\t"
        "movl %%eax, (%%edi)              \n\t"
        "movl 4(%%edi), %%eax             \n\t"
        "andl 4(%%edx), %%eax             \n\t"
        "orl  4(%%esi), %%eax             \n\t"
        "movl %%eax, 4(%%edi)             \n\t"
        "movl 8(%%edi), %%eax             \n\t"
        "andl 8(%%edx), %%eax             \n\t"
        "orl  8(%%esi), %%eax             \n\t"
        "movl %%eax, 8(%%edi)             \n\t"
        "movl 12(%%edi), %%eax            \n\t"
        "andl 12(%%edx), %%eax            \n\t"
        "orl  12(%%esi), %%eax            \n\t"
        "movl %%eax, 12(%%edi)            \n\t"
        "addl $16, %%esi                  \n\t"
        "addl $352, %%edi                 \n\t"
        "addl $16, %%edx                  \n\t"
        "decl %%ecx                       \n\t"
        "jnz tctl0                        \n\t"
        :
        : "m" (x), "m" (y), "m" (spr), "m" (matte)
        : "eax", "ecx", "edx", "esi", "edi", "cc" );
#endif
}

// TODO: Clipping?
void tcopysprite(int x, int y, int width, int height, unsigned char* spr) {
    auto p = virscr + y * 320 + x;
    while (height) {
        for (int w = 0; w < width; ++w) {
            auto c = *spr++;
            if (c) {
                p[w] = c;
            }
        }
        p += 320;
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

unsigned char match(char r, char g, char b) {
    int vr, vg, vb, tab, tabpt, i, thistab;
    // This routine provides the color matching to the VERGE palette.

    tab = 255;
    tabpt = 1;

    for (i = 1; i < 255; i++) {
        vr = vergepal[i * 3];
        vg = vergepal[(i * 3) + 1];
        vb = vergepal[(i * 3) + 2];

        thistab = abs(vr - r);
        thistab += abs(vg - g);
        thistab += abs(vb - b);
        if (thistab < tab) {
            tab = thistab;
            tabpt = i;
        }
    }

    return tabpt;
}

void BuildTable(char r, char g, char b, char* dest) {
    int i;
    unsigned char wr, wg, wb;

    for (i = 0; i < 256; i++) {
        wr = vergepal[(i * 3)];         // set wr,wg,wb with source colors
        wg = vergepal[(i * 3) + 1];
        wb = vergepal[(i * 3) + 2];

        wr = (wr + r) / 2;              // average values to compute final RGB
        wg = (wg + g) / 2;
        wb = (wb + b) / 2;

        dest[i] = match(wr, wg, wb);    // find closest match
    }
}

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
    int i;

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
/*
void ColorField(int x1, int y1, int x2, int y2, unsigned char *colortbl)
{ int x,y;
  unsigned char c;

  for (y=y1; y<y2; y++)
      for (x=x1; x<x2; x++)
          {
            c=virscr[(y*352)+x];
            c=((x-x1)*32)/(x2-x1);
            while(c>111 || c<88)
             {
              while(c>111)
                c-=(c-111)<<1;
              while(c<88)
                c+=(88-c)<<1;
             }
            virscr[(y*352)+x]=c;
          }
}
*/

void ColorField(int x, int y, int x2, int y2, unsigned char* tbl) {
#if 0
    asm( "movl %3, %%edx                   \n\t"
         "subl %1, %%edx                   \n\t"   // get height
         "movl %4, %%esi                   \n\t"
         "acf0:                                  \n\t"
         "movl %1, %%edi                   \n\t"
         "imul $352, %%edi                 \n\t"
         "addl %0, %%edi                   \n\t"
         "addl _virscr, %%edi              \n\t"
         "movl %2, %%ecx                   \n\t"
         "subl %0, %%ecx                   \n\t"  // get width
         "acf1:                                  \n\t"
         "movl $0, %%eax                   \n\t"
         "movb (%%edi), %%al               \n\t"
         "addl %%esi, %%eax                \n\t"
         "movb (%%eax), %%bl               \n\t"
         "movb %%bl, (%%edi)               \n\t"
         "incl %%edi                       \n\t"
         "decl %%ecx                       \n\t"
         "jnz acf1                         \n\t"
         "incl %1                          \n\t"
         "decl %%edx                       \n\t"
         "jnz acf0                         \n\t"
         :
         : "m" (x), "m" (y), "m" (x2), "m" (y2), "m" (tbl)
         : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc" );
#endif
}

void Tcopysprite(int x1, int y1, int width, int height, unsigned char* src) {
    unsigned int j, i, jz, iz;
    unsigned char c, d;

    for (j = 0; j < height; j++)
        for (i = 0; i < width; i++) {
            jz = j + y1;
            iz = i + x1;
            c = virscr[(jz * 352) + iz];
            d = src[(j * width) + i];
            if (d) {
                virscr[(jz * 352) + iz] = transparencytbl[(d * 256) + c];
            }
        }
}

void _Tcopysprite(int x1, int y1, int width, int height, unsigned char* src) {
    unsigned int j, i, jz, iz;
    unsigned char c, d;

    for (j = 0; j < height; j++)
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
    x1 = x;
    y1 = y;
}

void printstring(char* str) {
    int i;
    char c;

    i = 0;
    if (!str[0]) {
        return;
    }

mainloop:
    c = str[i];
    pchar(x1, y1, c);
    x1 = x1 + 8;
    i++;
    if (str[i] != 0) {
        goto mainloop;
    }
}

void VCprintstring(int xx, int yy, char* str) {
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

void bigprintstring(char* str) {
    int i;
    char c;

    i = 0;
    if (!str[0]) {
        return;
    }

mainloop:
    c = str[i];
    bigpchar(x1, y1, c);
    x1 = x1 + 10;
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
#if 0
    asm ("movl $10, %%ebx              \n\t"
         "movl %0, %%eax               \n\t"
         "movl %1, %%edi               \n\t"
         "xor %%ecx, %%ecx             \n\t"
         "dtal0:                               \n\t"
         "xor %%edx, %%edx             \n\t"
         "div %%ebx                    \n\t"
         "pushw %%dx                   \n\t"
         "incl %%ecx                   \n\t"
         "orl %%eax, %%eax             \n\t"
         "jnz dtal0                    \n\t"
         "dtal1:                               \n\t"
         "popw %%ax                    \n\t"
         "addb $48, %%al               \n\t"
         "stosb                        \n\t"
         "loop dtal1                   \n\t"
         "xor %%al, %%al               \n\t"
         "stosb                        \n\t"
         :
         : "m" (num), "m" (buf)
         : "eax", "ebx", "edi", "ecx", "cc" );
#endif
}

void textwindow(char portrait, char* str1, char* str2, char* str3) {
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
