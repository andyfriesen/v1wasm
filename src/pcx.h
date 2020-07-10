
#include <stdio.h>
#include "fs.h"

extern short int width, depth;
extern verge::VFILE* pcxf;

void loadpcx(char* fname, unsigned char* dest);
void LoadPCXHeaderNP(char* fname);
void ReadPCXLine(int vidoffset, unsigned char* dest);
