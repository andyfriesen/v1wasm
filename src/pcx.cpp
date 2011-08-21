// pcx.c
// PCX read/write routines
// A bit more flexible this time around, for variant destinations, image sizes,
// etc.

#include <stdio.h>
#include <string.h>
#include "timer.h"
#include "vga.h"
#include "main.h"
#include "fs.h"

using namespace verge;

char manufacturer;                     // pcx header
char version;
char encoding;
char bits_per_pixel;
short int xmin, ymin;
short int xmax, ymax;
short int hres;
short int vres;
char palette[48];
char reserved;
char color_planes;
short int bytes_per_line;
short int palette_type;
char filler[58];

unsigned short int width, depth;
unsigned short int bytes, i;
unsigned char c, run, ss = 0;
unsigned int vidoffset;
VFILE* pcxf;

void ReadPCXLine(unsigned char* dest) {
    int n = 0;

    do {
        c = vgetc(pcxf) & 0xff;
        if ((c & 0xc0) == 0xc0) {
            run = c & 0x3f;
            c = vgetc(pcxf);
            for (int j = 0; j < run; j++) {
                dest[vidoffset + n + j] = c;
            }
            n += run;
        } else {
            dest[vidoffset + n] = c;
            n++;
        }
    } while (n < bytes);
}

void LoadPCXHeader(char* fname) {
    if (!(pcxf = vopen(fname, "rb"))) {
        err("Could not open PCX file '%s'", fname);
    }
    vread(&manufacturer, 1, 1, pcxf);
    vread(&version, 1, 1, pcxf);
    vread(&encoding, 1, 1, pcxf);
    vread(&bits_per_pixel, 1, 1, pcxf);
    vread(&xmin, 1, 2, pcxf);
    vread(&ymin, 1, 2, pcxf);
    vread(&xmax, 1, 2, pcxf);
    width = xmax - xmin + 1;

    vread(&ymax, 1, 2, pcxf);
    depth = ymax - ymin + 1;

    vread(&hres, 1, 2, pcxf);
    vread(&vres, 1, 2, pcxf);
    vread(&palette, 1, 48, pcxf);
    vread(&reserved, 1, 1, pcxf);
    vread(&color_planes, 1, 1, pcxf);
    vread(&bytes_per_line, 1, 2, pcxf);
    vread(&palette_type, 1, 2, pcxf);
    vread(&filler, 1, 58, pcxf);
    vseek(pcxf, -768L, SEEK_END);
    vread(pal, 1, 768, pcxf);
    vseek(pcxf, 128L, SEEK_SET);
    bytes = bytes_per_line;
}

void LoadPCXHeaderNP(char* fname) {
    if (!(pcxf = vopen(fname, "rb"))) {
        err("Could not open PCX file '%s'", fname);
    }
    vread(&manufacturer, 1, 1, pcxf);
    vread(&version, 1, 1, pcxf);
    vread(&encoding, 1, 1, pcxf);
    vread(&bits_per_pixel, 1, 1, pcxf);
    vread(&xmin, 1, 2, pcxf);
    vread(&ymin, 1, 2, pcxf);
    vread(&xmax, 1, 2, pcxf);
    vread(&ymax, 1, 2, pcxf);
    vread(&hres, 1, 2, pcxf);
    vread(&vres, 1, 2, pcxf);
    vread(&palette, 1, 48, pcxf);
    vread(&reserved, 1, 1, pcxf);
    vread(&color_planes, 1, 1, pcxf);
    vread(&bytes_per_line, 1, 2, pcxf);
    vread(&palette_type, 1, 2, pcxf);
    vread(&filler, 1, 58, pcxf);
    width = xmax - xmin + 1;
    depth = ymax - ymin + 1;
    bytes = bytes_per_line;
}


void loadpcx(char* fname, unsigned char* dest) {
    LoadPCXHeader(fname);

    for (auto i = 0; i < depth; i++) {
        vidoffset = 5648 + (i * 352);
        ReadPCXLine(dest);
    }

    vclose(pcxf);
}

void WritePCXLine(unsigned char* p) {
    int i;
    unsigned char byte, samect, repcode;

    i = 0;
    do {
        byte = p[i++];
        samect = 1;
        while (samect < (unsigned) 63 && i < 320 && byte == p[i]) {
            samect++;
            i++;
        }
        if (samect > 1 || (byte & 0xC0) != 0) {
            repcode = 0xC0 | samect;
            vwrite(&repcode, 1, 1, pcxf);
        }
        vwrite(&byte, 1, 1, pcxf);
    } while (i < 320);
}

void WritePalette() {
    char b;
    int i;

    for (i = 0; i < 768; i++) {
        pal[i] = pal[i] << 2;
    }

    b = 12;
    vwrite(&b, 1, 1, pcxf);
    vwrite(pal, 1, 768, pcxf);

    for (i = 0; i < 768; i++) {
        pal[i] = pal[i] >> 2;
    }
}

void ScreenShot() {
    unsigned char b1;
    unsigned short int w1;
    char fnamestr[13];

    // Takes a snapshot of the current screen.

    dec_to_asciiz(ss, fnamestr);
    b1 = strlen(fnamestr);
    fnamestr[b1++] = '.';
    fnamestr[b1++] = 'P';
    fnamestr[b1++] = 'C';
    fnamestr[b1++] = 'X';
    fnamestr[b1++] = 0;

    pcxf = vopen(fnamestr, "wb");
    ss++;

    // Write PCX header

    b1 = 10;
    vwrite(&b1, 1, 1, pcxf); // manufacturer always = 10
    b1 = 5;
    vwrite(&b1, 1, 1, pcxf);  // version = 3.0, >16 colors
    b1 = 1;
    vwrite(&b1, 1, 1, pcxf);  // encoding always = 1
    b1 = 8;
    vwrite(&b1, 1, 1, pcxf);  // 8 bits per pixel, for 256 colors
    w1 = 0;
    vwrite(&w1, 1, 2, pcxf);  // xmin = 0;
    w1 = 0;
    vwrite(&w1, 1, 2, pcxf);  // ymin = 0;
    w1 = 319;
    vwrite(&w1, 1, 2, pcxf);  // xmax = 319;
    w1 = 199;
    vwrite(&w1, 1, 2, pcxf);  // ymax = 199;
    w1 = 320;
    vwrite(&w1, 1, 2, pcxf);  // hres = 320;
    w1 = 200;
    vwrite(&w1, 1, 2, pcxf);  // vres = 200;

    vwrite(virscr, 1, 48, pcxf);  // 16-color palette data. Who knows what's
    // actually in here. It doesn't matter since
    // the 256-color palette is stored elsewhere.

    b1 = 0;
    vwrite(&b1, 1, 1, pcxf);   // reserved always = 0.
    b1 = 1;
    vwrite(&b1, 1, 1, pcxf);   // number of color planes. Just 1 for 8bit.
    w1 = 320;
    vwrite(&w1, 1, 2, pcxf); // number of bytes per line

    w1 = 0;
    vwrite(&w1, 1, 1, pcxf);
    vwrite(virscr, 1, 59, pcxf);          // filler

    for (w1 = 0; w1 < 200; w1++) {
        WritePCXLine(screen + (w1 * 320));
    }

    WritePalette();
    vclose(pcxf);
    setTimerCount(0);
}
