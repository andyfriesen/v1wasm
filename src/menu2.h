#pragma once

extern unsigned char gsimg[512];

void LoadSaveErase(char mode);
void ItemMenu(char c);
void EquipMenu(char c);
void MagicMenu(char c);
void RemoveItem(char c, char i);

void greyscale(int width, int height, unsigned char* src, unsigned char* dest);
