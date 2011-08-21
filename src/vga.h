
#pragma once

struct IFramebuffer;

void closevga();
void quick_killgfx();
void quick_restoregfx();
void vgadump();
void setpixel(int x, int y, char c);
void hline(int x, int y, int dist, char c);
void vline(int x, int y, int y2, char c);
void box(int x, int y, int x2, int y2, char color);
void set_intensity(unsigned int n);
void fin();
void fout();
void LoadFont();
void paletteMorph(int r, int g, int b, int percent, int intensity);
void gotoxy(int x, int y);
void printstring(const char* str);
void bigprintstring(const char* str);
void textwindow(char portrait, const char* str1, const char* str2, const char* str3);
void copytile(int x, int y, unsigned char* spr);
void tcopytile(int x, int y, unsigned char* spr, unsigned char* matte);
void copysprite(int x, int y, int width, int height, unsigned char* spr);
void tcopysprite(int x, int y, int width, int height, unsigned char* spr);
void Tcopysprite(int x, int y, int width, int height, unsigned char* spr);
void _Tcopysprite(int x, int y, int width, int height, unsigned char* spr);
void grabregion(int x, int y, int width, int height, unsigned char* spr);
void fontcolor(unsigned char c);
void ColorField(int x, int y, int x2, int y2, unsigned char* tbl);
void ColorScale(unsigned char* dest, int st, int fn, int inv);
void VCprintstring(int xx, int yy, char* str);

void dec_to_asciiz(int num, char* buf);

extern unsigned char screen[];
extern unsigned char virscr[];
extern unsigned char pal[768];

extern unsigned char menuxlatbl[256], greyxlatbl[256], scrnxlatbl[256], *transparencytbl;
extern char oc;

void dump_palette(unsigned char*);

const int XRES = 320;
const int YRES = 200;
const int BACKBUFFER_PITCH = 352;
inline unsigned char* getScreenPointer(int x, int y) {
    return virscr + (y + 16) * BACKBUFFER_PITCH + x + 16;
}
