// vga.h
extern initvga();
extern closevga();
extern quick_killgfx();
extern quick_restoregfx();
extern vgadump();
extern setpixel(int x, int y, char c);
extern hline(int x, int y, int dist, char c);
extern box(int x, int y, int x2, int y2, char color);
extern set_palette(unsigned char *pall);
extern set_intensity(unsigned int n);
extern screen, virscr;
extern unsigned char pal[768];

extern char menuxlatbl[256],greyxlatbl[256],scrnxlatbl[256],*transparencytbl;
