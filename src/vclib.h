#pragma once

extern char stringbuffer[100];

extern unsigned char storeinv[12];

void VCtcopysprite(int x, int y, int width, int height, unsigned char* spr);
void VCClear();
void VCClearRegion();

void VChline(int x, int y, int x2, char c);

void ExecLibFunc(unsigned char func);
