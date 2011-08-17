// menu2.c
// Save/loadgame menu, Item menu, Equip menu.
// Copyright (C)1997 BJ Eirich

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "control.h"
#include "engine.h"
#include "keyboard.h"
#include "timer.h"
#include "render.h"
#include "main.h"
#include "menu.h"
#include "menu2.h"
#include "vc.h"
#include "sound.h"
#include "pcx.h"
#include "vga.h"

char sg1[] = "SAVEDAT.000";
char sg2[] = "SAVEDAT.001";
char sg3[] = "SAVEDAT.002";
char sg4[] = "SAVEDAT.003";
char* savename[] =
{ sg1, sg2, sg3, sg4 };

struct menu {
    unsigned short int posx, posy;
    char linktype;
};

struct menu menus[4];
unsigned char itmptr[576], gsimg[512], iuflag = 0;
extern unsigned char menuptr[256];
extern short int varl[10];

void greyscale(int width, int height, unsigned char* src, unsigned char* dest) {
    int i, j;
    unsigned char r, g, b, c;

    //  for (j=0; j<height; j++)
    //      for (i=0; i<width; i++)
    //          { c=src[(j*width)+i];
    //            c=greyxlatbl[c];
    //            dest[(j*width)+i]=c;
    //          }

    for (j = 0; j < height; j++)
        for (i = 0; i < width; i++) {
            c = src[(j * width) + i];
            r = pal[(c * 3)];
            g = pal[(c * 3) + 1];
            b = pal[(c * 3) + 2];
            c = (r + g + b) / 6;
            dest[(j * width) + i] = c;
        }
}

void LoadSaveErase(char mode)
// The Save Game / Load Game / Erase Game menu interface. Since the three
// functions are almost identical in their interface, I crammed them all
// into one function, using the Mode variable to specify the intent.
// MODE: 0=Load Game, 1=Save Game, 2=Erase Game
{
    FILE* f;
    int i, j, r = 0;
    unsigned char tbuf[2560], buf1[2560], buf2[2560], buf3[2560], buf4[2560];
    unsigned char rbuf[256];
    unsigned char* img;
    char mnu1[] = "LOADGAME.MNU";
    char mnu2[] = "SAVEGAME.MNU";
    char mnu3[] = "DELGAME.MNU";
    char* mnuname[] = { mnu1, mnu2, mnu3 };

    playeffect(1);
    fout();
redraw:
    if (!(f = fopen(mnuname[mode], "r"))) {
        err("Fatal Error: Could not open specified MNU file.");
    }
parseloop:
    fscanf(f, "%s", strbuf);

    if (strcmp(strbuf, "background") == 0) {
        fscanf(f, "%s", strbuf);
        loadpcx(strbuf, virscr);
        goto parseloop;
    }

    if (strcmp(strbuf, "print") == 0) {
        fscanf(f, "%u", &i);
        i += 16;
        fscanf(f, "%u", &j);
        j += 16;
        gotoxy(i, j);
        fscanf(f, "%s", strbuf);
        printstring(strbuf);
        goto parseloop;
    }

    if (strcmp(strbuf, "printb") == 0) {
        fscanf(f, "%u", &i);
        i += 16;
        fscanf(f, "%u", &j);
        j += 16;
        gotoxy(i, j);
        fscanf(f, "%s", strbuf);
        bigprintstring(strbuf);
        goto parseloop;
    }

    if (strcmp(strbuf, ":selectors") == 0) {
        for (i = 0; i < 4; i++) {
            fscanf(f, "%s", strbuf);
            menus[i].posx = atoi(strbuf) + 16;
            fscanf(f, "%s", strbuf);
            menus[i].posy = atoi(strbuf) + 16;
        }
        goto parseloop;
    }
    fclose(f);

    for (i = 0; i < 4; i++) {
        if (!(f = fopen(savename[i], "rb"))) {
            gotoxy(menus[i].posx + 100, menus[i].posy + 10);
            menus[i].linktype = 0;
            printstring("- NOT USED -");
        } else {
            menus[i].linktype = 1;
            fread(strbuf, 1, 30, f);
            gotoxy(menus[i].posx + 90, menus[i].posy + 1);
            printstring(strbuf);
            fread(strbuf, 1, 9, f);
            gotoxy(menus[i].posx + 90, menus[i].posy + 11);
            printstring(strbuf);
            fread(&j, 1, 4, f);
            dec_to_asciiz(j, strbuf);
            gotoxy(menus[i].posx + 162, menus[i].posy + 11);
            printstring("LV ");
            printstring(strbuf);
            fread(&j, 1, 4, f);
            gotoxy(menus[i].posx + 230, menus[i].posy + 11);
            dec_to_asciiz(j, strbuf);
            printstring(strbuf);
            printstring("/");
            fread(&j, 1, 4, f);
            dec_to_asciiz(j, strbuf);
            printstring(strbuf);
            fread(&j, 1, 4, f);
            gotoxy(menus[i].posx + 90, menus[i].posy + 21);
            dec_to_asciiz(j, strbuf);
            printstring(strbuf);
            printstring(" G");
            char b;
            fread(&b, 1, 1, f);
            gotoxy(menus[i].posx + 162, menus[i].posy + 21);
            dec_to_asciiz(b, strbuf);
            if (b < 10) {
                printstring("0");
            }
            printstring(strbuf);
            printstring(":");
            fread(&b, 1, 1, f);
            dec_to_asciiz(b, strbuf);
            if (b < 10) {
                printstring("0");
            }
            printstring(strbuf);
            fread(&b, 1, 2, f);
            fread(&j, 1, 1, f);
            if (!i) {
                img = buf1;
            }
            if (i == 1) {
                img = buf2;
            }
            if (i == 2) {
                img = buf3;
            }
            if (i == 3) {
                img = buf4;
            }
            fread(img, 1, 2560, f);
            greyscale(80, 32, (unsigned char*)img, &tbuf[0]);

            tcopysprite(menus[i].posx, menus[i].posy, 16, 32, tbuf);
            tcopysprite(menus[i].posx + 16, menus[i].posy, 16, 32, tbuf + 512);
            tcopysprite(menus[i].posx + 32, menus[i].posy, 16, 32, tbuf + 1024);
            tcopysprite(menus[i].posx + 48, menus[i].posy, 16, 32, tbuf + 1536);
            tcopysprite(menus[i].posx + 64, menus[i].posy, 16, 32, tbuf + 2048);
        }
        fclose(f);
    }

    int mpos = 0;
    grabregion(menus[0].posx - 16, menus[0].posy + 10, 16, 16, rbuf);
    tcopysprite(menus[0].posx - 16, menus[0].posy + 10, 16, 16, menuptr);
    if (menus[0].linktype) {
        tcopysprite(menus[0].posx, menus[0].posy, 16, 32, buf1);
        tcopysprite(menus[0].posx + 16, menus[0].posy, 16, 32, buf1 + 512);
        tcopysprite(menus[0].posx + 32, menus[0].posy, 16, 32, buf1 + 1024);
        tcopysprite(menus[0].posx + 48, menus[0].posy, 16, 32, buf1 + 1536);
        tcopysprite(menus[0].posx + 64, menus[0].posy, 16, 32, buf1 + 2048);
    }
    vgadump();
    if (!r) {
        fin();
    }
    if (r) while (b1) {
            readcontrols();
        }

inputloop:
    readcontrols();

    if (down) {
        copysprite(menus[mpos].posx - 16, menus[mpos].posy + 10, 16, 16, rbuf);
        if (!mpos) {
            img = buf1;
        }
        if (mpos == 1) {
            img = buf2;
        }
        if (mpos == 2) {
            img = buf3;
        }
        if (mpos == 3) {
            img = buf4;
        }
        if (menus[mpos].linktype) {
            greyscale(80, 32, img, tbuf);
            tcopysprite(menus[mpos].posx, menus[mpos].posy, 16, 32, tbuf);
            tcopysprite(menus[mpos].posx + 16, menus[mpos].posy, 16, 32, tbuf +512);
            tcopysprite(menus[mpos].posx + 32, menus[mpos].posy, 16, 32, tbuf + 1024);
            tcopysprite(menus[mpos].posx + 48, menus[mpos].posy, 16, 32, tbuf + 1536);
            tcopysprite(menus[mpos].posx + 64, menus[mpos].posy, 16, 32, tbuf + 2048);
        }
        mpos++;
        if (mpos == 4) {
            mpos = 0;
        }
        grabregion(menus[mpos].posx - 16, menus[mpos].posy + 10, 16, 16, rbuf);
        tcopysprite(menus[mpos].posx - 16, menus[mpos].posy + 10, 16, 16, menuptr);
        if (menus[mpos].linktype) {
            if (!mpos) {
                img = buf1;
            }
            if (mpos == 1) {
                img = buf2;
            }
            if (mpos == 2) {
                img = buf3;
            }
            if (mpos == 3) {
                img = buf4;
            }
            tcopysprite(menus[mpos].posx, menus[mpos].posy, 16, 32, img);
            tcopysprite(menus[mpos].posx + 16, menus[mpos].posy, 16, 32, img + 512);
            tcopysprite(menus[mpos].posx + 32, menus[mpos].posy, 16, 32, img + 1024);
            tcopysprite(menus[mpos].posx + 48, menus[mpos].posy, 16, 32, img + 1536);
            tcopysprite(menus[mpos].posx + 64, menus[mpos].posy, 16, 32, img + 2048);
        }
        playeffect(0);
        vgadump();
        while (down) {
            readcontrols();
        }
    }

    if (up) {
        copysprite(menus[mpos].posx - 16, menus[mpos].posy + 10, 16, 16, rbuf);
        if (!mpos) {
            img = buf1;
        }
        if (mpos == 1) {
            img = buf2;
        }
        if (mpos == 2) {
            img = buf3;
        }
        if (mpos == 3) {
            img = buf4;
        }
        if (menus[mpos].linktype) {
            greyscale(80, 32, img, tbuf);
            tcopysprite(menus[mpos].posx, menus[mpos].posy, 16, 32, tbuf);
            tcopysprite(menus[mpos].posx + 16, menus[mpos].posy, 16, 32, tbuf + 512);
            tcopysprite(menus[mpos].posx + 32, menus[mpos].posy, 16, 32, tbuf + 1024);
            tcopysprite(menus[mpos].posx + 48, menus[mpos].posy, 16, 32, tbuf + 1536);
            tcopysprite(menus[mpos].posx + 64, menus[mpos].posy, 16, 32, tbuf + 2048);
        }
        if (!mpos) {
            mpos = 3;
        } else {
            mpos--;
        }
        grabregion(menus[mpos].posx - 16, menus[mpos].posy + 10, 16, 16, rbuf);
        tcopysprite(menus[mpos].posx - 16, menus[mpos].posy + 10, 16, 16, menuptr);
        if (menus[mpos].linktype) {
            if (!mpos) {
                img = buf1;
            }
            if (mpos == 1) {
                img = buf2;
            }
            if (mpos == 2) {
                img = buf3;
            }
            if (mpos == 3) {
                img = buf4;
            }
            tcopysprite(menus[mpos].posx, menus[mpos].posy, 16, 32, img);
            tcopysprite(menus[mpos].posx + 16, menus[mpos].posy, 16, 32, img + 512);
            tcopysprite(menus[mpos].posx + 32, menus[mpos].posy, 16, 32, img + 1024);
            tcopysprite(menus[mpos].posx + 48, menus[mpos].posy, 16, 32, img + 1536);
            tcopysprite(menus[mpos].posx + 64, menus[mpos].posy, 16, 32, img + 2048);
        }
        playeffect(0);
        vgadump();
        while (up) {
            readcontrols();
        }
    }

    if ((b1) && (menus[mpos].linktype) && (!mode)) {
        fout();
        LoadGame(savename[mpos]);
        return;
    }

    if ((b1) && (mode == 1)) {
        SaveGame(savename[mpos]);
        r++;
        goto redraw;
    }

    if ((b1) && (menus[mpos].linktype) && (mode == 2)) {
        remove(savename[mpos]);
        r++;
        goto redraw;
    }

    if (!b2) {
        goto inputloop;
    }
    fout();
    timer_count = 0;
}

void RemoveItem(char c, char i) {
    char j;

    for (j = i; j < pstats[c].invcnt - 1; j++) {
        pstats[c].inv[j] = pstats[c].inv[j + 1];
    }
    pstats[c].inv[pstats[c].invcnt - 1] = 0;
    pstats[c].invcnt--;
}

void DrawItemMenu(char c, char ptr) {
    unsigned char l, i, a, *img, z;
    int j, k;

    l = partyidx[c] - 1;
    drawmap();
    tcopysprite(20, 20, 96, 96, chr2 + (c * 9216)); // Status portrait

    tmenubox(120, 20, 224, 38);               // name box
    i = strlen(pstats[l].name) * 4;
    gotoxy(172 - i, 26);
    printstring(pstats[l].name);
    tmenubox(226, 20, 330, 38);
    gotoxy(262, 26);
    printstring("ITEM");

    tmenubox(120, 40, 330, 59);              // Item name
    a = pstats[l].inv[ptr];
    if (!items[a].useflag || items[a].useflag >= 5) {
        fontcolor(17);
    }
    j = strlen(items[a].name);
    gotoxy(225 - (j * 4), 46);
    if (ptr < pstats[l].invcnt) {
        printstring(items[a].name);
    }
    fontcolor(31);

    tmenubox(120, 61, 330, 80);              // Item desc
    j = strlen(items[a].desc);
    gotoxy(225 - (j * 4), 67);
    if (ptr < pstats[l].invcnt) {
        printstring(items[a].desc);
    }

    if (items[a].equipflag && !iuflag) {         // Show who it's equipable by
        tmenubox(20, 117, 115, 186);
        gotoxy(27, 123);
        printstring("Equip:");
        z = 0;
        for (j = 0; j < 5; j++) {
            i = partyidx[j] - 1;
            if (equip[items[a].equipidx].equipable[i] && j < numchars) {
                gotoxy(33, 133 + (z * 10));
                printstring(pstats[i].name);
                z++;
            }
        }
    }

    tmenubox(120, 82, 330, 115);

    for (j = 0; j < 6; j++) {
        a = pstats[l].inv[j];
        img = itemicons + (items[a].icon * 256);
        tcopysprite(137 + (j * 32), 91, 16, 16, img);
    }

    tmenubox(120, 117, 330, 210);
    for (k = 0; k < 3; k++)
        for (j = 0; j < 6; j++) {
            a = pstats[l].inv[((k + 1) * 6) + j];
            img = itemicons + (items[a].icon * 256);
            tcopysprite(137 + (j * 32), 130 + (k * 24), 16, 16, img);
        }
    a = ptr / 6;
    if (ptr < 6) {
        tcopysprite(133 + (ptr * 32), 87, 24, 24, itmptr);
    } else {
        tcopysprite(133 + ((ptr - (a * 6)) * 32), 102 + (a * 24), 24, 24, itmptr);
    }
}

void ItemGive(char c, char p) {
    int first = 1, ptr = 0, i, j;
    unsigned char l, t1;

    playeffect(1);
drawloop:
    DrawItemMenu(c, p);
    tmenubox(20, 117, 115, 139 + (numchars * 10));
    gotoxy(30, 123);
    printstring("Give to:");

    for (j = 0; j < numchars; j++) {
        l = partyidx[j] - 1;
        gotoxy(45, 133 + (j * 10));
        printstring(pstats[l].name);
    }

    tcopysprite(27, 131 + (ptr * 10), 16, 16, menuptr);
    vgadump();

    readcontrols();
    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            UpdateEquipStats();
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down) {
        ptr++;
        if (ptr == numchars) {
            ptr = 0;
        }
        playeffect(0);
        first = 1;
    }
    if (up)   {
        if (!ptr) {
            ptr = numchars - 1;
        } else {
            ptr--;
        }
        playeffect(0);
        first = 1;
    }

    if (b1)   {
        l = partyidx[c] - 1;
        t1 = pstats[l].inv[p];
        if (p < 6) {
            pstats[l].inv[p] = 0;
        } else {
            RemoveItem(l, p);
        }

        l = partyidx[ptr] - 1;
        j = pstats[l].invcnt;
        pstats[l].inv[j] = t1;
        pstats[l].invcnt++;
        playeffect(1);
        first = 2;
    }

    while (!b4 && !b2) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }
}

void ItemUse(char c, char p) {
    int first = 1, ptr = 0, i, j;
    unsigned char l, t1, a;

    playeffect(1);
    a = partyidx[c] - 1;
    t1 = pstats[a].inv[p];

    if (!items[t1].useflag || items[t1].useflag > 4) {
        return;
    }
    if (numchars == 1) {
        l = a;
        goto usesec;
    }
    if (!items[t1].itemtype || items[t1].itemtype == 2) {
        l = a;
        goto usesec;
    }

drawloop:
    DrawItemMenu(c, p);
    tmenubox(20, 117, 115, 139 + (numchars * 10));
    gotoxy(30, 123);
    printstring("Use item:");

    for (j = 0; j < numchars; j++) {
        l = partyidx[j] - 1;
        gotoxy(45, 133 + (j * 10));
        printstring(pstats[l].name);
    }

    tcopysprite(27, 131 + (ptr * 10), 16, 16, menuptr);
    vgadump();

    readcontrols();
    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down) {
        ptr++;
        if (ptr == numchars) {
            ptr = 0;
        }
        playeffect(0);
        first = 1;
    }
    if (up)   {
        if (!ptr) {
            ptr = numchars - 1;
        } else {
            ptr--;
        }
        playeffect(0);
        first = 1;
    }

    if (b1)   {
        l = partyidx[ptr] - 1;
        goto usesec;
    }

    while (!b4 && !b2) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }

usesec:
    varl[0] = l;
    ExecuteEffect(items[t1].useeffect);

    if (items[t1].useflag == 2 || items[t1].useflag == 4) {
        return;
    }

    RemoveItem(c, p);
}

void ItemActionSelect(char c, char p) {
    int first = 1, ptr = 0, i;
    unsigned char l, a;

    playeffect(1);
    iuflag = 1;
    l = partyidx[c] - 1;
drawloop:
    DrawItemMenu(c, p);

    tmenubox(20, 117, 115, 157);                   // Put Use|Give|Drop menu up
    gotoxy(45, 123);
    printstring("Use");
    gotoxy(45, 133);
    printstring("Give");
    gotoxy(45, 143);
    printstring("Drop");
    tcopysprite(26, 121 + (ptr * 10), 16, 16, menuptr);
    vgadump();

    readcontrols();
    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            iuflag = 0;
            UpdateEquipStats();
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down) {
        ptr++;
        if (ptr == 3) {
            ptr = 0;
        }
        playeffect(0);
        first = 1;
    }
    if (up)   {
        if (!ptr) {
            ptr = 2;
        } else {
            ptr--;
        }
        playeffect(0);
        first = 1;
    }

    if (b1)   {
        switch (ptr) {
            case 0:
                ItemUse(c, p);
                first = 2;
                break;
            case 1:
                if (numchars == 1) {
                    break;
                }
                ItemGive(c, p);
                first = 2;
                break;
            case 2:
                if (!items[pstats[l].inv[p]].price) {
                    playeffect(3);
                    first = 2;
                    break;
                }
                if (p > 5) {
                    RemoveItem(l, p);
                } else {
                    a = pstats[l].inv[p];
                    if (equip[items[a].equipidx].ondeequip) {
                        ExecuteEffect(equip[items[a].equipidx].ondeequip - 1);
                    }
                    pstats[l].inv[p] = 0;
                }
                UpdateEquipStats();
                first = 2;
                break;
        }
        playeffect(1);
    }

    while (!b4 && !b2) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }
}

void ItemMenu(char c) {
    int first = 1;
    unsigned char l, ptr = 6, mx = 0, my = 1;

    playeffect(1);
    l = partyidx[c] - 1;
drawloop:
    DrawItemMenu(c, ptr);
    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up && !right && !left) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down)  {
        if (my < 3) {
            my++;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }
    if (up)    {
        if (my) {
            my--;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }
    if (right) {
        if (mx < 5) {
            mx++;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }
    if (left)  {
        if (mx) {
            mx--;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }

    if (b1)    {
        if (ptr < pstats[l].invcnt) {
            ItemActionSelect(c, ptr);
        }
        first = 1;
        goto drawloop;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }
}

int atkp, defp, magp, mgrp, hitp, dodp, mblp, ferp, reap;

void CalcEquipPreview(int a, int i, int p) {
    int c, d;

    d = items[pstats[a].inv[p]].equipflag - 1;
    c = pstats[a].inv[d];
    pstats[a].inv[d] = 0;
    UpdateEquipStats();

    if (p > 5) {
        atkp = pstats[a].atk + equip[i].str;
        defp = pstats[a].def + equip[i].end;
        magp = pstats[a].magc + equip[i].mag;
        mgrp = pstats[a].mgrc + equip[i].mgr;
        hitp = pstats[a].hitc + equip[i].hit;
        dodp = pstats[a].dodc + equip[i].dod;
        mblp = pstats[a].mblc + equip[i].mbl;
        ferp = pstats[a].ferc + equip[i].fer;
        reap = pstats[a].reac + equip[i].rea;
    } else {
        atkp = pstats[a].atk;
        defp = pstats[a].def;
        magp = pstats[a].magc;
        mgrp = pstats[a].mgrc;
        hitp = pstats[a].hitc;
        dodp = pstats[a].dodc;
        mblp = pstats[a].mblc;
        ferp = pstats[a].ferc;
        reap = pstats[a].reac;
    }

    pstats[a].inv[d] = c;
    UpdateEquipStats();
}

void DrawEquipMenu(char c, char ptr) {
    unsigned char l, i, a, *img;
    int j, k;

    l = partyidx[c] - 1;
    drawmap();
    tcopysprite(20, 20, 96, 96, chr2 + (c * 9216)); // Status portrait

    tmenubox(120, 20, 224, 38);               // name box
    i = strlen(pstats[l].name) * 4;
    gotoxy(172 - i, 26);
    printstring(pstats[l].name);
    tmenubox(226, 20, 330, 38);
    gotoxy(258, 26);
    printstring("EQUIP");

    tmenubox(120, 40, 330, 59);              // Item name
    a = pstats[l].inv[ptr];
    if (!items[a].equipflag || !equip[items[a].equipidx].equipable[l]) {
        fontcolor(17);
    }
    j = strlen(items[a].name);
    gotoxy(225 - (j * 4), 46);
    if (ptr < pstats[l].invcnt) {
        printstring(items[a].name);
    }
    fontcolor(31);

    tmenubox(120, 61, 330, 80);              // Item desc
    j = strlen(items[a].desc);
    gotoxy(225 - (j * 4), 67);
    if (ptr < pstats[l].invcnt) {
        printstring(items[a].desc);
    }

    // If equipment, do effect preview box

    if (items[a].equipflag && equip[items[a].equipidx].equipable[l]) {
        tmenubox(20, 117, 115, 210);
        gotoxy(26, 124);
        printstring("ATK ");
        dec_to_asciiz(pstats[l].atk, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 124);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(atkp, strbuf);
        if (pstats[l].atk < atkp) {
            fontcolor(97);
        }
        if (atkp < pstats[l].atk) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 124);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 133);
        printstring("DEF ");
        dec_to_asciiz(pstats[l].def, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 133);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(defp, strbuf);
        if (pstats[l].def < defp) {
            fontcolor(97);
        }
        if (defp < pstats[l].def) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 133);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 142);
        printstring("HIT ");
        dec_to_asciiz(pstats[l].hitc, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 142);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(hitp, strbuf);
        if (pstats[l].hitc < hitp) {
            fontcolor(97);
        }
        if (hitp < pstats[l].hitc) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 142);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 151);
        printstring("DOD ");
        dec_to_asciiz(pstats[l].dodc, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 151);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(dodp, strbuf);
        if (pstats[l].dodc < dodp) {
            fontcolor(97);
        }
        if (dodp < pstats[l].dodc) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 151);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 160);
        printstring("MAG ");
        dec_to_asciiz(pstats[l].magc, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 160);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(magp, strbuf);
        if (pstats[l].magc < magp) {
            fontcolor(97);
        }
        if (magp < pstats[l].magc) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 160);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 169);
        printstring("MGR ");
        dec_to_asciiz(pstats[l].mgrc, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 169);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(mgrp, strbuf);
        if (pstats[l].mgrc < mgrp) {
            fontcolor(97);
        }
        if (mgrp < pstats[l].mgrc) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 169);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 178);
        printstring("REA ");
        dec_to_asciiz(pstats[l].reac, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 178);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(reap, strbuf);
        if (pstats[l].reac < reap) {
            fontcolor(97);
        }
        if (reap < pstats[l].reac) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 178);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 187);
        printstring("FER ");
        dec_to_asciiz(pstats[l].ferc, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 187);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(ferp, strbuf);
        if (pstats[l].ferc < ferp) {
            fontcolor(97);
        }
        if (ferp < pstats[l].ferc) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 187);
        printstring(strbuf);
        fontcolor(31);
        gotoxy(26, 196);
        printstring("MBL ");
        dec_to_asciiz(pstats[l].mblc, strbuf);
        gotoxy(80 - (strlen(strbuf) * 8), 196);
        printstring(strbuf);
        printstring(">");
        dec_to_asciiz(mblp, strbuf);
        if (pstats[l].mblc < mblp) {
            fontcolor(97);
        }
        if (mblp < pstats[l].mblc) {
            fontcolor(36);
        }
        gotoxy(110 - (strlen(strbuf) * 8), 196);
        printstring(strbuf);
        fontcolor(31);
    }

    tmenubox(120, 82, 330, 115);

    for (j = 0; j < 6; j++) {
        a = pstats[l].inv[j];
        img = itemicons + (items[a].icon * 256);
        tcopysprite(137 + (j * 32), 91, 16, 16, img);
    }

    tmenubox(120, 117, 330, 210);
    for (k = 0; k < 3; k++)
        for (j = 0; j < 6; j++) {
            a = pstats[l].inv[((k + 1) * 6) + j];
            img = itemicons + (items[a].icon * 256);
            if (!items[a].equipflag || !equip[items[a].equipidx].equipable[l]) {
                greyscale(16, 16, img, gsimg);
                img = gsimg;
            }
            tcopysprite(137 + (j * 32), 130 + (k * 24), 16, 16, img);
        }
    a = ptr / 6;
    if (ptr < 6) {
        tcopysprite(133 + (ptr * 32), 87, 24, 24, itmptr);
    } else {
        tcopysprite(133 + ((ptr - (a * 6)) * 32), 102 + (a * 24), 24, 24, itmptr);
    }
}

void Equip(char c, char ptr) {
    unsigned char a, l, b;

    l = partyidx[c] - 1;
    a = pstats[l].inv[ptr];
    if (!items[a].equipflag || !equip[items[a].equipidx].equipable[l]) {
        playeffect(3);
        return;
    }

    b = items[a].equipflag;
    if (!pstats[l].inv[b - 1]) {
        RemoveItem(l, ptr);
    } else {
        pstats[l].inv[ptr] = pstats[l].inv[b - 1];
    }
    pstats[l].inv[b - 1] = a;
    UpdateEquipStats();
    playeffect(5);
    if (equip[items[a].equipidx].onequip) {
        ExecuteEffect(equip[items[a].equipidx].onequip - 1);
    }
}

void DeEquip(char c, char ptr) {
    unsigned char a, l, b;

    l = partyidx[c] - 1;
    a = pstats[l].inv[ptr];

    if (pstats[l].invcnt == 23 || !items[a].equipflag) {
        playeffect(3);
        return;
    }

    pstats[l].inv[ptr] = 0;
    pstats[l].inv[pstats[l].invcnt] = a;
    pstats[l].invcnt++;
    UpdateEquipStats();
    playeffect(6);
    if (equip[items[a].equipidx].ondeequip) {
        ExecuteEffect(equip[items[a].equipidx].ondeequip - 1);
    }
}

void EquipMenu(char c) {
    int first = 1, a;
    unsigned char l, ptr = 6, mx = 0, my = 1;

    playeffect(1);
    l = partyidx[c] - 1;
    a = pstats[l].inv[ptr];
    CalcEquipPreview(l, items[a].equipidx, ptr);

drawloop:
    DrawEquipMenu(c, ptr);
    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up && !right && !left) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down)  {
        if (my < 3) {
            my++;
        }
        ptr = (my * 6) + mx;
        a = pstats[l].inv[ptr];
        CalcEquipPreview(l, items[a].equipidx, ptr);
        playeffect(0);
        first = 1;
    }
    if (up)    {
        if (my) {
            my--;
        }
        ptr = (my * 6) + mx;
        a = pstats[l].inv[ptr];
        CalcEquipPreview(l, items[a].equipidx, ptr);
        playeffect(0);
        first = 1;
    }
    if (right) {
        if (mx < 5) {
            mx++;
        }
        ptr = (my * 6) + mx;
        a = pstats[l].inv[ptr];
        CalcEquipPreview(l, items[a].equipidx, ptr);
        playeffect(0);
        first = 1;
    }
    if (left)  {
        if (mx) {
            mx--;
        }
        ptr = (my * 6) + mx;
        a = pstats[l].inv[ptr];
        CalcEquipPreview(l, items[a].equipidx, ptr);
        playeffect(0);
        first = 1;
    }

    if (b1)    {
        if (ptr >= pstats[l].invcnt) {
            first = 1;
            goto drawloop;
        }
        if (ptr < 6) {
            DeEquip(c, ptr);
        }
        if (ptr >= 6) {
            Equip(c, ptr);
        }
        a = pstats[l].inv[ptr];
        CalcEquipPreview(l, items[a].equipidx, ptr);
        first = 1;
        goto drawloop;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }
}


// Magic Menu? NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW

void DrawMagicMenu(char c, char ptr) {
    unsigned char l, i, a, *img, z;
    int j, k;

    l = partyidx[c] - 1;
    drawmap();
    tcopysprite(20, 20, 96, 96, chr2 + (c * 9216)); // Status portrait

    tmenubox(120, 20, 224, 38);               // name box
    i = strlen(pstats[l].name) * 4;
    gotoxy(172 - i, 26);
    printstring(pstats[l].name);
    tmenubox(226, 20, 330, 38);
    gotoxy(262, 26);
    printstring("MAGIC");

    tmenubox(120, 40, 330, 59);              // Item name
    a = pstats[l].maginv[ptr];
    if (!magic[a].useflag || magic[a].useflag >= 5) {
        fontcolor(17);
    }
    j = strlen(magic[a].name);
    gotoxy(225 - (j * 4), 46);
    if (ptr < pstats[l].magcnt) {
        printstring(magic[a].name);
    }
    fontcolor(31);

    tmenubox(120, 61, 330, 80);              // Item desc
    j = strlen(magic[a].desc);
    gotoxy(225 - (j * 4), 67);
    if (ptr < pstats[l].magcnt) {
        printstring(magic[a].desc);
    }

    if (magic[a].equipflag && !iuflag) {         // Show who it's equipable by
        tmenubox(20, 117, 115, 186);
        gotoxy(27, 123);
        printstring("Use:");
        z = 0;
        for (j = 0; j < 5; j++) {
            i = partyidx[j] - 1;
            if (mequip[magic[a].equipidx].equipable[i] && j < numchars) {
                gotoxy(33, 133 + (z * 10));
                printstring(pstats[i].name);
                z++;
            }
        }
    }

    //  tmenubox(120,82,330,115);

    tmenubox(120, 93, 330, 210);

    for (j = 0; j < 6; j++) {
        a = pstats[l].maginv[j];
        img = magicicons + (magic[a].icon * 256);
        tcopysprite(137 + (j * 32), 102, 16, 16, img);
    }

    for (k = 0; k < 3; k++)
        for (j = 0; j < 6; j++) {
            a = pstats[l].maginv[((k + 1) * 6) + j];
            img = magicicons + (magic[a].icon * 256);
            tcopysprite(137 + (j * 32), 130 + (k * 24), 16, 16, img);
        }
    a = ptr / 6;
    if (ptr < 6) {
        tcopysprite(133 + (ptr * 32), 98, 24, 24, itmptr);
    } else {
        tcopysprite(133 + ((ptr - (a * 6)) * 32), 102 + (a * 24), 24, 24, itmptr);
    }
}



void MagicUse(char c, char p) {
    int first = 1, ptr = 0, i, j;
    unsigned char l, t1, a;
    int timer_count, an;

    playeffect(1);
    a = partyidx[c] - 1;
    t1 = pstats[a].maginv[p];

    if (!magic[t1].useflag || magic[t1].useflag > 4) {
        return;
    }
    if (!pstats[a].curhp) {
        playeffect(3);
        return;
    }
    if (numchars == 1) {
        l = a;
        goto usesec;
    }
    if (!magic[t1].itemtype || magic[t1].itemtype == 2) {
        l = a;
        goto usesec;
    }

drawloop:
    DrawMagicMenu(c, p);
    tmenubox(20, 117, 115, 139 + (numchars * 10));
    gotoxy(30, 123);
    printstring("Cast on:");

    for (j = 0; j < numchars; j++) {
        l = partyidx[j] - 1;
        gotoxy(45, 133 + (j * 10));
        printstring(pstats[l].name);
    }

    tcopysprite(27, 131 + (ptr * 10), 16, 16, menuptr);
    vgadump();

    readcontrols();
    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down) {
        ptr++;
        if (ptr == numchars) {
            ptr = 0;
        }
        playeffect(0);
        first = 1;
    }
    if (up)   {
        if (!ptr) {
            ptr = numchars - 1;
        } else {
            ptr--;
        }
        playeffect(0);
        first = 1;
    }

    if (b1)   {
        l = partyidx[ptr] - 1;
        goto usesec;
    }

    while (!b4 && !b2) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }

usesec:
    /*
      while (b1 || b2 || b3 || b4)
      {
        readcontrols();
      }
    */
    varl[0] = l;
    if (magic[t1].price <= pstats[a].curmp) {
        pstats[a].curmp -= magic[t1].price;

        ExecuteMagicEffect(magic[t1].useeffect);
    } else {
        timer_count = 0;
        an = 1;
drawloop2:
        drawmap();
        tmenubox(120, 90, 240, 110);
        gotoxy(130, 96);
        printstring("Not Enough MP");
        vgadump();
        readcontrols();

        if (first == 2) if (b1 || b2 || b4) {
                goto drawloop2;
            } else {
                an = 0;
                timer_count = 0;
                return;
            }
        if (first && !b1 && !b2 && !b4 && !down && !up) {
            first = 0;
        } else if (first) {
            goto drawloop2;
        }


        while (!b4 && !b2 && !b1) {
            goto drawloop2;
        }
        while (b4 || b2 || b1) {
            first = 2;
            goto drawloop2;
        }
        timer_count = 0;
        an = 0;

    }

    if (magic[t1].useflag == 2 || magic[t1].useflag == 4) {
        return;
    }

    // RemoveItem(c,p);
}

void MagicActionSelect(char c, char p) {
    int first = 1, ptr = 0, i;
    unsigned char l, a;
    int t1;

    playeffect(1);
    iuflag = 1;
    l = partyidx[c] - 1;
drawloop:
    DrawMagicMenu(c, p);

    tmenubox(20, 117, 115, 137);                   // Put Use|Give|Drop menu up
    gotoxy(45, 123);
    printstring("Cast");
    tmenubox(20, 139, 115, 159);
    gotoxy(45, 145);
    printstring("MP Cost");
    tmenubox(20, 161, 115, 181);
    gotoxy(45, 167);
    t1 = pstats[l].maginv[p];
    dec_to_asciiz(magic[t1].price, strbuf);
    printstring(strbuf);

    tcopysprite(26, 121 + (ptr * 10), 16, 16, menuptr);
    vgadump();

    readcontrols();
    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            iuflag = 0;
            UpdateEquipStats();
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    /*
      if (down) { ptr++;
                  if (ptr==3) ptr=0;
                  playeffect(0);
                  first=1;
                }
      if (up)   { if (!ptr) ptr=2;
                  else ptr--;
                  playeffect(0);
                  first=1;
                }
    */

    if (b1)   {
        switch (ptr) {
            case 0:
                MagicUse(c, p);
                first = 2;
                break;
            case 1:
                if (numchars == 1) {
                    break;
                }
                ItemGive(c, p);
                first = 2;
                break;
            case 2:
                if (!magic[pstats[l].maginv[p]].price) {
                    playeffect(3);
                    first = 2;
                    break;
                }
                if (p > 5) {
                    RemoveItem(l, p);
                } else {
                    a = pstats[l].maginv[p];
                    if (equip[magic[a].equipidx].ondeequip) {
                        ExecuteMagicEffect(equip[magic[a].equipidx].ondeequip - 1);
                    }
                    pstats[l].maginv[p] = 0;
                }
                UpdateEquipStats();
                first = 2;
                break;
        }
        playeffect(1);
    }

    while (!b4 && !b2) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }
}

void MagicMenu(char c) {
    int first = 1;
    unsigned char l, ptr = 0, mx = 0, my = 0;

    playeffect(1);
    l = partyidx[c] - 1;
drawloop:
    DrawMagicMenu(c, ptr);
    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up && !right && !left) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down)  {
        if (my < 3) {
            my++;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }
    if (up)    {
        if (my) {
            my--;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }
    if (right) {
        if (mx < 5) {
            mx++;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }
    if (left)  {
        if (mx) {
            mx--;
        }
        ptr = (my * 6) + mx;
        playeffect(0);
        first = 1;
    }

    if (b1)    {
        if (ptr < pstats[l].magcnt) {
            MagicActionSelect(c, ptr);
        }
        first = 1;
        goto drawloop;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2) {
        first = 2;
        goto drawloop;
    }
}
