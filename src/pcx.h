
#include <stdio.h>

extern short int width, depth;
extern FILE* pcxf;
extern int vidoffset;

void loadpcx(char* fname, unsigned char* dest);
void LoadPCXHeaderNP(char* fname);
void ReadPCXLine(unsigned char* dest);
