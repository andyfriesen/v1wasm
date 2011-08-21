
#include <stdio.h>

extern short int width, depth;
extern FILE* pcxf;

void loadpcx(char* fname, unsigned char* dest);
void LoadPCXHeaderNP(char* fname);
void ReadPCXLine(int vidoffset, unsigned char* dest);
