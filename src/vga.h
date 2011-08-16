// vga.h
extern void initvga();
extern void closevga();
extern void quick_killgfx();
extern void quick_restoregfx();
extern void vgadump();
extern void setpixel(int x, int y, char c);
extern void hline(int x, int y, int dist, char c);
extern void box(int x, int y, int x2, int y2, char color);
extern void set_palette(unsigned char *pall);
extern void set_intensity(unsigned int n);
void fin();
extern unsigned char* screen;
extern unsigned char* virscr;
extern unsigned char pal[768];

extern char menuxlatbl[256],greyxlatbl[256],scrnxlatbl[256],*transparencytbl;
