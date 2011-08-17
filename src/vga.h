// vga.h
void initvga();
void closevga();
void quick_killgfx();
void quick_restoregfx();
void vgadump();
void setpixel(int x, int y, char c);
void hline(int x, int y, int dist, char c);
void box(int x, int y, int x2, int y2, char color);
void set_palette(unsigned char *pall);
void set_intensity(unsigned int n);
void fin();
void fout();
void LoadFont();
void get_palette();
void gotoxy(int x, int y);
void printstring(char* str);
void bigprintstring(char* str);
void copysprite(int x, int y, int width, int height, unsigned char* spr);
void tcopysprite(int x, int y, int width, int height, unsigned char* spr);

void dec_to_asciiz(int num, char* buf);

extern unsigned char* screen;
extern unsigned char* virscr;
extern unsigned char pal[768];

extern char menuxlatbl[256],greyxlatbl[256],scrnxlatbl[256],*transparencytbl;
