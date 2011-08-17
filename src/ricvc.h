#pragma once

void grey(int width, int height, unsigned char* src, unsigned char* dest);
void VChline(int x, int y, int x2, char c);
void VCvline (int x, int y, int y2, char c);
void VCColorField(int x1, int y1, int x2, int y2, unsigned char* colortbl);
void VCborder(int x1, int y1, int x2, int y2);
void VCBox();
void VCAString(int x1, int y1, char* strng, int align);
void VCCharName();
void VCItemName();
void VCItemDesc();
void VCItemImage();
void VCATextNum();
void VCSpc();
void CallEffect();
void CallScript();
void BindKey();
void TextMenu();
void itemMenu();
void equipMenu();
void magicMenu();
void statusScreen();
void VCCr2();
void VCTextBox();
int ChooseChar(int x1, int y1);
