#pragma once

extern unsigned char menuptr[256];

void PutBuySellBox(char p);
void PutGPBox();
void PutCharBox(char a, char b, char c, char d, char e, char p);
void StatusScreen(char cz);
void SellCharItem(char c);
void ConfirmSell(char c, char ptr);
void MConfirmSell(char c, char ptr);
void BuyItem(char ptr);
void PurchaseItem(char c, char i);
void BuyOrBuyEquip(char c, char ptr);
void SellCharMagic(char c);
void BuyMagic(char ptr);
void PurchaseMagic(char c, char i);
void SystemMenu();
void MainMenu();
void MBuyMenu();
void MSellMenu();

void menubox(int x, int y, int x2, int y2);
void tmenubox(int x, int y, int x2, int y2);
