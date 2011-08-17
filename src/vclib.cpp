// vclib.c
// The VergeC standard function library
// Copyright (C)1997 BJ Eirich

/*  -- Added by Ric --
 * Added internal functions:
 *       VCvline (21/Apr/98)
 *       VCborder (21/Apr/98)
 *       VCColorField(x1, y1, x2, y2, unsigned char *colortbl) (22/Apr/98)
 * Added functions:
 *       VCBox(x1,y1,x2,y2); (21/Apr/98)
 *       VCCharName(x, y, party.dat index, center); (21/Apr/98)
 *       VCItemName(x, y, items.dat index, center); (21/Apr/98)
 *       VCItemDesc(x, y, items.dat index, center); (21/Apr/98)
 *       VCItemImage(x, y, items.dat index, greyflag); (22/Apr/98)
 *       VCATextNum(x, y, number, align);              (24/Apr/98)
 *       VCSpc(x, y, speech portrait, greyflag);       (24/Apr/98)
 *       CallEffect(event number, option var1...);     (24/Apr/98)
 *       CallScript(event number, option var1...);     (25/Apr/98)
 *       VCLine(x1, y1, x2, y2, color);                (??/???/??)
 *       GetMagic (character, spell);                  (??/???/??)
 *       BindKey(key code, script);                    (03/Apr/98)
 *       TextMenu(x,y,flag,ptr,"opt1","opt2",..);      (04/May/98)
 *       ItemMenu(roster order index);                 (03/May/98)
 *       EquipMenu(roster order index);                (03/May/98)
 *       MagicMenu(roster order index);                (03/May/98)
 *       StatusScreen(roster order index);             (03/May/98)
 *       VCCr2(x, y, roster order index, greyflag);    (03/May/98)
 *       VCSpellName(x, y, magic.dat index, center);   (??/???/??)
 *       VCSpellDesc(x, y, magic.dat index, center);   (??/???/??)
 *       VCSpellImage(x, y, magic.dat index, greyflag);(??/???/??)
 *       MagicShop(spell1, spell2, spell3, ... spell12); (??/??/??)
 *       VCTextBox(x,y,flag,"opt1","opt2",..);     (04/May/98)
 *       VCMagicImage(x, y, items.dat index, greyflag);(04/May/98)
 * Added var0:
 *       FontColor (RW) (21/Apr/98)
 *       KeepAZ    (RW) (03/May/98)
 * Added var1:
 *       Item.Use        (R) (21/Apr/98)
 *       Item.Effect     (R) (24/Apr/98)
 *       Item.Type       (R) (24/Apr/98)
 *       Item.EquipType  (R) (24/Apr/98)
 *       Item.EquipIndex (R) (24/Apr/98)
 *       Item.Price      (R) (24/Apr/98)
 *       Spell.Use       (R) (??/???/??)
 *       Spell.Effect    (R) (??/???/??)
 *       Spell.Type      (R) (??/???/??)
 *       Spell.Price     (R) (??/???/??)
 *       Spell.Cost      (R) (??/???/??)
 *       Lvl             (R) (03/May/98)
 *       Nxt             (R) (03/May/98)
 *       CharStatus      (R) (03/May/98)
 * Added var2:
 *       CanEquip(party.dat index, item.dat index) (R) (24/Apr/98)
 *       ChooseChar(x,y)                           (R) (25/Apr/98)
 *       Obs(x,y);                                 (R) (??/???/??)
 * Other:
 *       Made CallEvent preserve temp vars across call (03/May/98)
 *         if keepaz is set
 *       DestroyItem should now update stats when      (03/May/98)
 *         equipped items are destroyed
 */
/* -- Added by xBig_D == */
/*
 * Added var0:
 *       VCLayer2(RW)
 *       VCLayer2trans(RW)
 *       VCLayerWrite(RW)
 */


#include <stdio.h>
#include "control.h"
#include "engine.h"
#include "keyboard.h"
#include "pcx.h"
#include "render.h"
#include "timer.h"
#include "vga.h"
#include "mikmod.h"
#include "ricvc.c"

extern char* strbuf, killvc, *code, *menuptr, *mapvc, menuactive, drawparty, layer1trans, layervctrans, layervc2trans;
extern char cameratracking, *vcdatabuf, movesuccess, *msbuf, drawentities;
extern unsigned int scriptofstbl[1024];
extern int varl[10], tvar[26], msofstbl[100];
extern unsigned char mp_volume;

char fade = 1, cancelfade = 0, stringbuffer[100], keepaz = 0;
unsigned char storeinv[12];

#include "nichgvc.c"
#include "xbigdvc.c"
#include "andyvc.c"

MapSwitch() {
    char b;

    hookretrace = 0;
    hooktimer = 0;
    GrabString(strbuf);
    nx = ResolveOperand();
    ny = ResolveOperand();
    b = ResolveOperand();
    if ((nx) && (ny)) {
        usenxy = 1;
    } else {
        usenxy = 0;
    }
    if (!b) {
        fout();
    }

    load_map(strbuf);

    drawmap();
    vgadump();
    if (!b) {
        fin();
    }
    killvc = 1;
    timer_count = 0;
}

Warp() {
    unsigned short int wx, wy;
    int i;
    char b;

    wx = ResolveOperand();
    wy = ResolveOperand();
    b = ResolveOperand();
    if (!b) {
        fout();
    }

    for (i = 0; i < 5; i++) {
        party[i].x = wx * 16;
        party[i].y = wy * 16;
        party[i].cx = wx;
        party[i].cy = wy;
        party[i].moving = 0;
        party[i].movcnt = 0;
        party[i].framectr = 0;
        lastmoves[i] = 0;
    }
    lastmoves[5] = 0;

    if (!b) {
        drawmap();
        vgadump();
        fin();
    }
    timer_count = 0;
}

AddCharacter() {
    char c;

    c = ResolveOperand();
    if (numchars == 5) {
        return;
    }

    addcharacter(c);
    timer_count = 0;
}

SoundEffect() {
    char c;

    c = ResolveOperand();
    playeffect(c - 1);
}

GiveItem () {
    short int c, d;
    char* img, first = 1;
    int i, j;

    c = ResolveOperand();
    an = 1;
drawloop:
    drawmap();
    tmenubox(50, 90, 302, 132);
    i = strlen(items[c].name);
    j = 176 - (((i * 8) + 80) / 2);
    gotoxy(j, 100);
    printstring(items[c].name);
    printstring(" Obtained!");
    d = items[c].icon;
    img = itemicons + (256 * d);
    tcopysprite(168, 110, 16, 16, img);
    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            i = 0;
            while (pstats[partyidx[i] - 1].invcnt == 24) {
                i++;
            }
            j = pstats[partyidx[i] - 1].invcnt;
            pstats[partyidx[i] - 1].inv[j] = c;
            pstats[partyidx[i] - 1].invcnt++;
            an = 0;
            timer_count = 0;
            return;
        }
    if (first && !b1 && !b2 && !b4) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2 || b1) {
        first = 2;
        goto drawloop;
    }
}

Text() {
    char* str1, *str2, *str3, portrait, first = 1;
    portrait = ResolveOperand();
    str1 = code;
    GrabString(strbuf);
    str2 = code;
    GrabString(strbuf);
    str3 = code;
    GrabString(strbuf);
    an = 1;
drawloop:
    drawmap();
    textwindow(portrait, str1, str2, str3);

    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            an = 0;
            timer_count = 0;
            return;
        }
    if (first && !b1 && !b2 && !b4) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2 || b1) {
        first = 2;
        goto drawloop;
    }
}

AlterFTile() {
    unsigned short int tx, ty, tt;
    unsigned char o, buf;

    tx = ResolveOperand();
    ty = ResolveOperand();
    tt = ResolveOperand();
    o = ResolveOperand();

    map1[(ty * xsize) + tx] = tt;
    switch (o) {
    case 0:
        buf = mapp[(ty * xsize) + tx] >> 1;
        mapp[(ty * xsize) + tx] = buf << 1;
        break;
    case 1:
        mapp[(ty * xsize) + tx] = mapp[(ty * xsize) + tx] | 1;
        break;
    }
}

AlterBTile() {
    unsigned short int tx, ty, tt;
    unsigned char o, buf;

    tx = ResolveOperand();
    ty = ResolveOperand();
    tt = ResolveOperand();
    o = ResolveOperand();

    map0[(ty * xsize) + tx] = tt;
    switch (o) {
    case 0:
        buf = mapp[(ty * xsize) + tx] >> 1;
        mapp[(ty * xsize) + tx] = buf << 1;
        break;
    case 1:
        mapp[(ty * xsize) + tx] = mapp[(ty * xsize) + tx] | 1;
        break;
    }
}

FakeBattle() {
    battle();
}

PlayMusic() {
    GrabString(strbuf);
    playsong(strbuf);
    timer_count = 0;
}

StopMusic() {
    stopsound();
}

HealAll() {
    int i;
    char idx;

    for (i = 0; i < numchars; i++) {
        idx = partyidx[i] - 1;
        pstats[idx].curhp = pstats[idx].maxhp;
        pstats[idx].curmp = pstats[idx].maxmp;
        pstats[idx].status = 0;
    }
}

AlterParallax() {
    unsigned char t;

    t = ResolveOperand();
    layerc = t;
    t = ResolveOperand();
    pmultx = t;
    pmulty = t;
    t = ResolveOperand();
    pdivx = t;
    pdivy = t;
}

FadeIn() {
    int i, s;

    s = ResolveOperand();
    timer_count = 0;
inloop:
    i = (timer_count * 64) / s;
    set_intensity(i);
    if (timer_count < s) {
        goto inloop;
    }
    set_intensity(63);
    timer_count = 0;
}

FadeOut() {
    int i, s;

    s = ResolveOperand();
    timer_count = 0;
outloop:
    i = (timer_count * 64) / s;
    i = 64 - i;
    set_intensity(i);
    if (timer_count < s) {
        goto outloop;
    }
    set_intensity(0);
    timer_count = 0;
}

RemoveCharacter() {
    unsigned char c;
    char foundit = 0, foundat = 0;
    char i;

    c = ResolveOperand();

    for (i = 0; i < numchars; i++)
        if (c == partyidx[i]) {
            foundit = 1;
            foundat = i;
        }

    if (!foundit) {
        return;
    }

    for (i = foundat; i < (numchars - 1); i++) {
        partyidx[i] = partyidx[i + 1];
    }

    partyidx[numchars] = 0;
    foundat = numchars - 1;
    numchars = 0;
    for (i = 0; i < foundat; i++) {
        addcharacter(partyidx[i]);
    }
    for (i = numchars; i < 5; i++) {
        party[i].cx = -1;
        party[i].cy = -1;
    }
    timer_count = 0;
}

Banner() {
    char* str, first = 1;
    int duration;

    str = code;
    an = 1;
    GrabString(strbuf);
    duration = ResolveOperand() * 91;
    timer_count = 0;
drawloop:
    drawmap();
    tmenubox(106, 106, 246, 126);
    gotoxy(176 - (strlen(str) * 4), 112);
    printstring(strbuf);
    vgadump();
    readcontrols();


    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            an = 0;
            timer_count = 0;
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }


    while (!b4 && !b2 && !b1 && timer_count < duration) {
        goto drawloop;
    }
    while (b4 || b2 || b1) {
        first = 2;
        goto drawloop;
    }
    timer_count = 0;
    an = 0;
}

EnforceAnimation() {
    /* -- ric:10/May/98 --
     * Temporarily removed to see if the bug this "fixes" still exists
    FILE *f;
    short int i,z;

    z=0;
    while (!va0[0].delay || z<2)
       {  if ((keyboard_map[SCAN_ALT]) &&
             (keyboard_map[SCAN_CTRL]) &&
             (keyboard_map[SCAN_DEL]))
                err("Exiting: CTRL-ALT-DEL pressed.");

         f=fopen(&vsp0name,"rb");
         fseek(f, 770, 0);
         fread(&i, 1, 2, f);
         fread(vsp0, i, 256, f);
         fread(&va0, 1, 800, f);
         fclose(f);
         timer_count=0; z++;
       }
    */
}

WaitKeyUp() {
    an = 1;
drawloop:
    drawmap();
    vgadump();
    readcontrols();

    if (b1 || b2 || b4) {
        goto drawloop;
    }
    an = 0;
    timer_count = 0;
}

DestroyItemProcessChar(unsigned char i, unsigned char c) {
    unsigned char l, found = 0, foundat = 0;

    c--;
    if (!i) {
        pstats[c].invcnt = 0;
        for (l = 0; l < 24; l++) {
            if (l < 6) {
                /* -- ric:03/May/98 -- */
                if (equip[items[pstats[c].inv[l]].equipidx].ondeequip) {
                    ExecuteEffect(equip[items[pstats[c].inv[l]].equipidx].ondeequip - 1);
                }
            }
            pstats[c].inv[l] = 0;
        }
        UpdateEquipStats();
        return;
    }

    for (l = 0; l < pstats[c].invcnt; l++)
        if (pstats[c].inv[l] == i) {
            foundat = l;
            if (l < 6) {                  /* -- ric:03/May/98 -- */
                if (equip[items[pstats[c].inv[l]].equipidx].ondeequip) {
                    ExecuteEffect(equip[items[pstats[c].inv[l]].equipidx].ondeequip - 1);
                }
                pstats[c].inv[l] = 0;
                UpdateEquipStats();
                return;
            }
            for (l = foundat; l < pstats[c].invcnt; l++) {
                pstats[c].inv[l] = pstats[c].inv[l + 1];
            }
            pstats[c].invcnt--;
            pstats[c].inv[pstats[c].invcnt] = 0;
            UpdateEquipStats();
            return;
        }
}

DestroyItem() {
    unsigned char i, c, l;

    i = ResolveOperand();
    c = ResolveOperand();

    if (c) {
        DestroyItemProcessChar(i, c);
        return;
    }

    for (l = 1; l < 31; l++) {
        DestroyItemProcessChar(i, l);
    }
}

Prompt() {
    char* str1, *str2, *str3, *opt1, *opt2, portrait, first = 1, selptr = 0;
    unsigned short int flagidx;

    portrait = ResolveOperand();
    str1 = code;
    GrabString(strbuf);
    str2 = code;
    GrabString(strbuf);
    str3 = code;
    GrabString(strbuf);
    flagidx = ResolveOperand();
    opt1 = code;
    GrabString(strbuf);
    opt2 = code;
    GrabString(strbuf);
    an = 1;

drawloop:
    drawmap();
    textwindow(portrait, str1, str2, str3);
    tmenubox(190, 118, 331, 148);
    gotoxy(220, 124);
    printstring(opt1);
    gotoxy(220, 134);
    printstring(opt2);
    tcopysprite(200, 122 + (selptr * 10), 16, 16, &menuptr);
    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            an = 0;
            timer_count = 0;
            flags[flagidx] = selptr;
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (up || down) {
        selptr = selptr ^ 1;
        playeffect(0);
        first = 1;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2 || b1) {
        first = 2;
        goto drawloop;
    }
}

ChainEvent () {
    char varcnt, i;
    unsigned short int t;

    varcnt = GrabC();
    t = ResolveOperand();

    for (i = 0; i < varcnt; i++) {
        varl[i] = ResolveOperand();
    }

    code = mapvc + scriptofstbl[t];
}

CallEvent () { /* -- ric: 03/May/98 - Now saves temp vars across call -- */
    char varcnt, i, *buf, savetmpvar;
    unsigned short int t;
    int savetvar[26];    /* -- New -- */

    savetmpvar = keepaz;
    varcnt = GrabC();
    t = ResolveOperand();

    for (i = 0; i < varcnt; i++) {
        varl[i] = ResolveOperand();
    }

    buf = code;
    if (savetmpvar) {
        memcpy(&savetvar, &tvar, sizeof(tvar));    /* -- New -- */
    }
    ExecuteScript(t);
    if (savetmpvar) {
        memcpy(&tvar, &savetvar, sizeof(tvar));    /* -- New -- */
    }
    code = buf;
}

Heal() {
    unsigned short int chr, amt;

    chr = ResolveOperand();
    amt = ResolveOperand();

    if (pstats[chr].curhp > 0) {
        pstats[chr].curhp += amt;
        if (pstats[chr].curhp > pstats[chr].maxhp) {
            pstats[chr].curhp = pstats[chr].maxhp;
        }
    }
}

EarthQuake() {
    int i, j, k;
    int nxw, nyw;
    char switchflag = 0;

    k = ResolveOperand();
    j = ResolveOperand();
    i = ResolveOperand();

    timer_count = 0;
    while (timer_count <= i) {
        nxw = xwin;
        nyw = ywin;
        if (!switchflag) {
            if (k > xwin && k) {
                nxw = 0;
            } else if (k) {
                nxw = xwin - (rand() % k);
            }
            if (j > ywin && j) {
                nyw = 0;
            } else if (j) {
                nyw = ywin - (rand() % j);
            }
        } else {
            if (k) {
                nxw = xwin + (rand() % k);
            }
            if (j) {
                nyw = ywin + (rand() % j);
            }
        }

        switchflag = switchflag ^ 1;
        drawmaploc(nxw, nyw);
        vgadump();
    }
    vgadump();
}

SaveMenu() {
    LoadSaveErase(1);
    drawmap();
    vgadump();
    fin();
    timer_count = 0;
}

EnableSave() {
    saveflag = 1;
}

DisableSave() {
    saveflag = 0;
}

ReviveChar() {
    short int a;

    a = ResolveOperand();
    pstats[a].status = 0;
    if (!pstats[a].curhp) {
        pstats[a].curhp = 1;
    }
}

RestoreMP() {
    unsigned short int chr, amt;

    chr = ResolveOperand();
    amt = ResolveOperand();

    pstats[chr].curmp += amt;
    if (pstats[chr].curmp > pstats[chr].maxmp) {
        pstats[chr].curmp = pstats[chr].maxmp;
    }
}

Redraw() {
    drawmap();
    vgadump();
}

SText() {
    char* str1, *str2, *str3;
    char st1[31], st2[31], st3[31];
    char portrait, first = 1, line = 1, chr = 0;

    // Setup - Read stuff from the VC
    portrait = ResolveOperand();
    str1 = code;
    GrabString(strbuf);
    str2 = code;
    GrabString(strbuf);
    str3 = code;
    GrabString(strbuf);
    st1[0] = 0;
    st2[0] = 0;
    st3[0] = 0;
    an = 1;
    timer_count = 0;

drawloop:
    while (timer_count != 0) {
        switch (line) {
        case 1:
            st1[chr] = str1[chr];
            st1[chr + 1] = 0;
            if (chr == strlen(str1)) {
                chr = 0;
                line = 2;
            } else {
                chr++;
            }
            break;
        case 2:
            st2[chr] = str2[chr];
            st2[chr + 1] = 0;
            if (chr == strlen(str2)) {
                chr = 0;
                line = 3;
            } else {
                chr++;
            }
            break;
        case 3:
            st3[chr] = str3[chr];
            st3[chr + 1] = 0;
            if (chr < strlen(str3)) {
                chr++;
            }
            break;
        }
        timer_count--;
    }

    drawmap();
    textwindow(portrait, &st1, &st2, &st3);
    vgadump();

    while (!timer_count) {
        gp--;
        gp++;
    }

    readcontrols();

    if (first == 2) if (b1 || b2 || b4) {
            goto drawloop;
        } else {
            an = 0;
            timer_count = 0;
            return;
        }
    if (first && !b1 && !b2 && !b4) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    while (!b4 && !b2 && !b1) {
        goto drawloop;
    }
    while (b4 || b2 || b1) {
        first = 2;
        goto drawloop;
    }
}

DisableMenu() {
    menuactive = 0;
}

EnableMenu() {
    menuactive = 1;
}

Wait() {
    short int delaytime, ct2;

    delaytime = ResolveOperand();
    timer_count = 0;
    ct2 = 0;
main_loop:
    while (timer_count != 0) {
        timer_count--;
        ct2++;
        check_tileanimation();
        process_entities();
    }
    readcontrols();
    drawmap();
    vgadump();

    if (ct2 < delaytime) {
        goto main_loop;
    }
    timer_count = 0;
}

SetFace() {
    char c, d;

    c = ResolveOperand();
    d = ResolveOperand();

    party[c - 1].facing = d;
}

MapPaletteGradient() {
    int sc, fc, f, m;

    sc = ResolveOperand();
    fc = ResolveOperand();
    f = ResolveOperand();
    m = ResolveOperand();

    switch (m) {
    case 0:
        ColorScale(&scrnxlatbl, sc, fc, f);
        screengradient = 1;
        break;
    case 1:
        ColorScale(&menuxlatbl, sc, fc, f);
        break;
    case 2:
        ColorScale(&greyxlatbl, sc, fc, f);
        break;
    }
}

BoxFadeOut() {
    int duration, hd, vd;

    duration = ResolveOperand();
    timer_count = 0;
    an = 1;

dloop:
    drawmap();
    hd = (timer_count * 176 / duration);
    vd = (timer_count * 116 / duration);
    box(0, 0, hd, 216, 0);
    box(352 - hd, 16, 352, 232, 0);
    box(0, 0, 352, vd, 0);
    box(0, 232 - vd, 352, 232, 0);
    vgadump();
    if (timer_count <= duration) {
        goto dloop;
    }
    timer_count = 0;
    an = 0;
}

BoxFadeIn() {
    int duration, hd, vd;

    duration = ResolveOperand();
    timer_count = 0;
    an = 1;

dloop:
    drawmap();
    hd = (timer_count * 176 / duration);
    vd = (timer_count * 116 / duration);
    hd = 176 - hd;
    vd = 116 - vd;
    box(0, 0, hd, 216, 0);
    box(352 - hd, 16, 352, 232, 0);
    box(0, 0, 352, vd, 0);
    box(0, 232 - vd, 352, 232, 0);
    vgadump();
    if (timer_count <= duration) {
        goto dloop;
    }
    timer_count = 0;
    an = 0;
}

GiveGP() {
    gp += ResolveOperand();
}

TakeGP() {
    gp -= ResolveOperand();
}

ChangeZone() {
    int x, y;
    unsigned char nz, b;

    x = ResolveOperand();
    y = ResolveOperand();
    nz = ResolveOperand();

    b = mapp[(y * xsize) + x] & 1;
    mapp[(y * xsize) + x] = (nz << 1) | b;
}

GetItem() {
    short int c, d;
    int i, j;

    c = ResolveOperand() - 1;
    d = ResolveOperand();

    j = pstats[c].invcnt;
    if (j != 24) {
        pstats[c].inv[j] = d;
        pstats[c].invcnt++;
    } else {
        pstats[c].inv[j - 1] = d;
    }
}

ForceEquip() {
    int c, i, a, b;

    c = ResolveOperand() - 1;
    i = ResolveOperand();
    a = items[i].equipidx;
    b = items[i].equipflag - 1;

    if (pstats[c].inv[b]) {
        pstats[c].inv[b] = 0;
    }
    pstats[c].inv[b] = i;
    UpdateEquipStats();
}

GiveXP() {
    int c, amt, fa, nx;

    c = ResolveOperand() - 1;
    amt = ResolveOperand();
    fa = pstats[c].exp + amt;

    while (pstats[c].exp != fa) {
        nx = pstats[c].nxt - pstats[c].exp;
        if (amt >= nx) {
            pstats[c].exp += nx;
            amt -= nx;
            levelup(c);
        } else {
            pstats[c].exp += amt;
            amt = 0;
        }
    }
}

Shop() {
    int first = 1, nv, p;

    // Egad.
    playeffect(1);
    an = 1;
    nv = GrabC();
    memset(&storeinv, 0, 12);
    for (p = 0; p < nv; p++) {
        storeinv[p] = ResolveOperand();
    }
    p = 0;
drawloop:
    drawmap();
    PutBuySellBox(p);
    PutGPBox();
    PutCharBox(0, 0, 0, 0, 0, 0);
    vgadump();

    readcontrols();

    if (first == 2) if (b2 || b3) {
            goto drawloop;
        } else {
            an = 0;
            timer_count = 0;
            return;
        }
    if (first && !b1 && !b3 && !down && !up) {
        first = 0;
    } else if (first) {
        goto drawloop;
    }

    if (down || up) {
        p = p ^ 1;
        playeffect(0);
        first = 1;
    }

    if (b1) {
        if (!p) {
            BuyMenu();
        }
        if (p) {
            SellMenu();
        }
    }

    while (!b3 && !b2) {
        goto drawloop;
    }
    while (b3 || b2) {
        first = 2;
        goto drawloop;
    }
}

extern unsigned char pal[768], pal2[768];

PaletteMorph() {
    int r, g, b, percent, intensity, i, wr, wg, wb;

    r = ResolveOperand();
    g = ResolveOperand();
    b = ResolveOperand();
    percent = 100 - ResolveOperand();
    intensity = ResolveOperand();

    for (i = 0; i < 256; i++) {
        wr = pal[(i * 3)];
        wg = pal[(i * 3) + 1];
        wb = pal[(i * 3) + 2];

        wr = ((wr * percent) + (r * (100 - percent))) / 100;
        wg = ((wg * percent) + (g * (100 - percent))) / 100;
        wb = ((wb * percent) + (b * (100 - percent))) / 100;

        pal2[(i * 3)] = wr * intensity / 63;
        pal2[(i * 3) + 1] = wg * intensity / 63;
        pal2[(i * 3) + 2] = wb * intensity / 63;
    }
    set_palette(&pal2);
}

int CharPos(char p1) {
    char i;

    for (i = 0; i < numchars; i++)
        if (partyidx[i] == p1) {
            return i;
        }
}

ChangeCHR() {
    int l;
    char* img;
    FILE* f;

    l = ResolveOperand();
    GrabString(&pstats[l - 1].chrfile);
    img = chrs + (CharPos(l) * 15360);
    f = fopen(pstats[l - 1].chrfile, "rb");
    fread(img, 1, 15360, f);
    fclose(f);
}

VCPutPCX() {
    int x, y, i;

    GrabString(&stringbuffer);
    x = ResolveOperand();
    y = ResolveOperand();
    LoadPCXHeaderNP(&stringbuffer);

    for (i = 0; i < depth; i++) {
        vidoffset = ((i + y) * 320) + x;
        ReadPCXLine(vcscreen);
    }
    fclose(pcxf);
}

HookTimer() {
    int l;

    hooktimer = ResolveOperand();
}

HookRetrace() {
    hookretrace = ResolveOperand();
}

VCLoadPCX() {
    int ofs, i;

    GrabString(&stringbuffer);
    ofs = ResolveOperand();
    LoadPCXHeaderNP(&stringbuffer);

    for (i = 0; i < depth; i++) {
        vidoffset = (i * width) + ofs;
        ReadPCXLine(vcdatabuf);
    }
    fclose(pcxf);
}

VCcopysprite(int x, int y, int width, int height, char* spr) {
    asm("movl %3, %%edx                   \n\t"
        "movl %4, %%esi                   \n\t"
        "csl0:                                  \n\t"
        "movl %1, %%eax                   \n\t"
        "imul $320, %%eax                 \n\t"
        "addl %0, %%eax                   \n\t"
        "addl _vcscreen, %%eax            \n\t"
        "movl %%eax, %%edi                \n\t"
        "movl %2, %%ecx                   \n\t"
        "shrl $1, %%ecx                   \n\t"
        "jnc csl1                         \n\t"
        "movsb                            \n\t"
        "csl1:                                  \n\t"
        "repz                             \n\t"
        "movsw                            \n\t"
        "incl %1                          \n\t"
        "decl %%edx                       \n\t"
        "jnz csl0                         \n\t"
        :
        : "m" (x), "m" (y), "m" (width), "m" (height), "m" (spr)
        : "eax", "edx", "esi", "edi", "ecx", "cc" );
}

VCtcopysprite(int x, int y, int width, int height, char* spr) {
    asm("movl %3, %%ecx                   \n\t"
        "movl %4, %%esi                   \n\t"
        "tcsl0:                                 \n\t"
        "movl %1, %%eax                   \n\t"
        "imul $320, %%eax                 \n\t"
        "addl %0, %%eax                   \n\t"
        "addl _vcscreen, %%eax              \n\t"
        "movl %%eax, %%edi                \n\t"
        "movl %2, %%edx                   \n\t"
        "drawloop:                              \n\t"
        "lodsb                            \n\t"
        "orb %%al, %%al                   \n\t"
        "jz nodraw                        \n\t"
        "stosb                            \n\t"
        "decl %%edx                       \n\t"
        "orl %%edx, %%edx                 \n\t"
        "jz endline                       \n\t"
        "jmp drawloop                     \n\t"
        "nodraw:                                \n\t"
        "incl %%edi                       \n\t"
        "decl %%edx                       \n\t"
        "orl %%edx, %%edx                 \n\t"
        "jnz drawloop                     \n\t"
        "endline:                               \n\t"
        "incl %1                          \n\t"
        "decl %%ecx                       \n\t"
        "jnz tcsl0                        \n\t"
        :
        : "m" (x), "m" (y), "m" (width), "m" (height), "m" (spr)
        : "eax", "edx", "esi", "edi", "ecx", "cc" );
}

VCBlitImage() {
    int x1, y1, xs, ys, ofs;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    xs = ResolveOperand();
    ys = ResolveOperand();
    ofs = ResolveOperand();

    VCcopysprite(x1, y1, xs, ys, vcdatabuf + ofs);
}


VCClear() {
    memset(vcscreen, 0, 64000);
}

VChline(int x, int y, int x2, char c) {
    asm ("movl %2, %%ecx                 \n\t"
         "subl %0, %%ecx                 \n\t"
         "movl %1, %%eax                 \n\t"
         "imul $320, %%eax               \n\t"
         "addl %0, %%eax                 \n\t"
         "addl _vcscreen, %%eax          \n\t"
         "movl %%eax, %%edi              \n\t"
         "movb %3, %%al                  \n\t"
         "repz                           \n\t"
         "stosb                          \n\t"
         :
         : "m" (x), "m" (y), "m" (x2), "m" (c)
         : "eax", "edi", "ecx", "cc" );
}

VCClearRegion() {
    int x1, y1, x2, y2, i;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    x2 = ResolveOperand();
    y2 = ResolveOperand();

    for (i = y1; i <= y2; i++) {
        VChline(x1, i, x2, 0);
    }
}

VCText() {
    int x1, y1;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    GrabString(&stringbuffer);
    VCprintstring(x1, y1, &stringbuffer);
}

VCTBlitImage() {
    int x1, y1, xs, ys, ofs;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    xs = ResolveOperand();
    ys = ResolveOperand();
    ofs = ResolveOperand();

    VCtcopysprite(x1, y1, xs, ys, vcdatabuf + ofs);
}

extern char qabort;

Exit() {
    qabort = 1;
    killvc = 1;
}

Quit() {
    GrabString(strbuf);
    err(strbuf);
}

VCCenterText() {
    int x1, y1;

    y1 = ResolveOperand();
    GrabString(&stringbuffer);
    x1 = 160 - (strlen(&stringbuffer) * 4);
    VCprintstring(x1, y1, &stringbuffer);
}

ResetTimer() {
    timer_count = 0;
}

VCBlitTile() {
    int x1, y1, t;
    char* img;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    t = ResolveOperand();

    img = vsp0 + (t * 256);
    VCtcopysprite(x1, y1, 16, 16, img);
}

Sys_ClearScreen() {
    memset(virscr, 0, 90000);
}

Sys_DisplayPCX() {
    GrabString(&stringbuffer);
    loadpcx(&stringbuffer, virscr);
}

OldStartupMenu() {
    StartupMenu();
}

NewGame() {
    GrabString(&stringbuffer);
    StartNewGame(&stringbuffer);
}

Delay() {
    int s;

    s = ResolveOperand();
    timer_count = 0;
    while (timer_count < s)
        if ((keyboard_map[SCAN_ALT]) &&
                (keyboard_map[SCAN_CTRL]) &&
                (keyboard_map[SCAN_DEL])) {
            err("Exiting: CTRL-ALT-DEL pressed.");
        }
}

GetNextMove() {
    if (!party[0].scriptofs) {
        return;
    }
    if (!party[0].curcmd) {
        GetNextCommand(0);
    }

    switch (party[0].curcmd) {
    case 1:
        MoveUp(0);
        if (movesuccess) {
            party[0].cmdarg--;
            lastmove(2);
            startfollow();
        }
        break;
    case 2:
        MoveDown(0);
        if (movesuccess) {
            party[0].cmdarg--;
            lastmove(1);
            startfollow();
        }
        break;
    case 3:
        MoveLeft(0);
        if (movesuccess) {
            party[0].cmdarg--;
            lastmove(4);
            startfollow();
        }
        break;
    case 4:
        MoveRight(0);
        if (movesuccess) {
            party[0].cmdarg--;
            lastmove(3);
            startfollow();
        }
        break;
    case 5:
        party[0].speed = party[0].cmdarg;
        party[0].cmdarg = 0;
        break;
    case 6:
        party[0].cmdarg--;
        break;
    case 7:
        party[0].scriptofs = 0;
        party[0].curcmd = 0;
        return;
    case 8:
        err("Script spawning not supported in PartyMove.");
    case 9:
        err("Looping not supported in PartyMove.");
    case 10:
        if (party[0].cx < party[0].cmdarg) {
            MoveRight(0);
            lastmove(3);
            startfollow();
        }
        if (party[0].cx > party[0].cmdarg) {
            MoveLeft(0);
            lastmove(4);
            startfollow();
        }
        if (party[0].cx == party[0].cmdarg) {
            party[0].cmdarg = 0;
        }
        break;
        break;
    case 11:
        if (party[0].cy < party[0].cmdarg) {
            MoveDown(0);
            lastmove(1);
            startfollow();
        }
        if (party[0].cy > party[0].cmdarg) {
            MoveUp(0);
            lastmove(2);
            startfollow();
        }
        if (party[0].cy == party[0].cmdarg) {
            party[0].cmdarg = 0;
        }
        break;
        break;
    case 12:
        party[0].facing = party[0].cmdarg;
        party[0].cmdarg = 0;
        break;
    case 13:
        party[0].specframe = party[0].cmdarg;
        party[0].cmdarg = 0;
        break;
    }
    if (!party[0].cmdarg) {
        party[0].curcmd = 0;
    }
}

moveparty() {
    int i;

    party[0].speedct = 0;

    if (!party[0].moving) {
        GetNextMove();
        if (!party[0].scriptofs) {
            return;
        }
    }

    if (party[0].moving)
        for (i = 0; i < numchars; i++) {
            switch (party[i].moving) {
            case 1:
                party[i].y++;
                party[i].movcnt--;
                party[i].framectr++;
                break;
            case 2:
                party[i].y--;
                party[i].movcnt--;
                party[i].framectr++;
                break;
            case 3:
                party[i].x++;
                party[i].movcnt--;
                party[i].framectr++;
                break;
            case 4:
                party[i].x--;
                party[i].movcnt--;
                party[i].framectr++;
                break;
            }
            if (party[i].framectr == 80) {
                party[i].framectr = 0;
            }
        }
    if (!party[0].movcnt) {
        party[0].moving = 0;
    }
}

MoveParty() {
    if (party[0].speed < 4) {
        switch (party[0].speed) {
        case 1:
            if (party[0].speedct < 3) {
                party[0].speedct++;
                return;
            }
        case 2:
            if (party[0].speedct < 2) {
                party[0].speedct++;
                return;
            }
        case 3:
            if (party[0].speedct < 1) {
                party[0].speedct++;
                return;
            }
        }
    }
    if (party[0].speed < 5) {
        moveparty();
        return;
    }
    switch (party[0].speed) {
    case 5:
        moveparty();
        moveparty();
        return;
    case 6:
        moveparty();
        moveparty();
        moveparty();
        return;
    case 7:
        moveparty();
        moveparty();
        moveparty();
        moveparty();
        return;
    }
}

PartyMove() {
    party[0].scriptofs = code;
    GrabString(&stringbuffer);
    timer_count = 0;

main_loop:
    while (timer_count != 0) {
        timer_count--;
        check_tileanimation();
        process_entities();
        MoveParty();
    }
    readcontrols();
    drawmap();
    vgadump();

    if (party[0].scriptofs) {
        goto main_loop;
    }
}

EntityMove() {
    int i;

    i = ResolveOperand();
    party[i].moving = 0;
    party[i].movcnt = 0;
    party[i].speedct = 0;
    party[i].delayct = 0;
    party[i].curcmd = 0;
    party[i].cmdarg = 0;
    party[i].chasing = 0;

    party[i].scriptofs = code;
    GrabString(&stringbuffer);
    party[i].movecode = 4;
}

AutoOn() {
    int i;

    memset(&party[95].x, 0, 440);
    for (i = 0; i < numchars; i++) {
        party[i + 95].cx = party[i].cx;
        party[i + 95].x = party[i].cx * 16;
        party[i + 95].cy = party[i].cy;
        party[i + 95].y = party[i].cy * 16;
        party[i + 95].moving = 0;
        party[i + 95].facing = party[i].facing;
        party[i + 95].specframe = party[i].specframe;
        party[i + 95].speed = party[0].speed;
        party[i + 95].chrindex = i;
    }
    autoent = 1;
    drawparty = 0;
}

AutoOff() {
    autoent = 0;
    drawparty = 1;
}

EntityMoveScript() {
    int i;

    i = ResolveOperand();
    party[i].movescript = ResolveOperand();
    party[i].scriptofs = msbuf + msofstbl[party[i].movescript];
    party[i].movecode = 4;
    party[i].curcmd = 0;
}

VCTextNum() {
    int x1, y1, i;
    x1 = ResolveOperand();
    y1 = ResolveOperand();
    i = ResolveOperand();
    if (i < 0) {
        i = -i;
        VCprintstring(x1, y1, "-");
        x1 += 8;
    }
    dec_to_asciiz(i, &stringbuffer);
    VCprintstring(x1, y1, &stringbuffer);
}

VCLoadRaw() {
    int vcofs, fofs, flen;
    FILE* f;

    GrabString(&stringbuffer);
    vcofs = ResolveOperand();
    fofs = ResolveOperand();
    flen = ResolveOperand();

    f = fopen(&stringbuffer, "rb");
    fseek(f, fofs, 0);
    fread(vcdatabuf + vcofs, flen, 1, f);
    fclose(f);
}

ExecLibFunc(unsigned char func) {
    switch (func) {
    case 1:
        MapSwitch();
        break;
    case 2:
        Warp();
        break;
    case 3:
        AddCharacter();
        break;
    case 4:
        SoundEffect();
        break;
    case 5:
        GiveItem();
        break;
    case 6:
        Text();
        break;
    case 7:
        AlterFTile();
        break;
    case 8:
        AlterBTile();
        break;
    case 9:
        FakeBattle();
        break;
    case 10:
        break;
    case 11:
        PlayMusic();
        break;
    case 12:
        StopMusic();
        break;
    case 13:
        HealAll();
        break;
    case 14:
        AlterParallax();
        break;
    case 15:
        FadeIn();
        break;
    case 16:
        FadeOut();
        break;
    case 17:
        RemoveCharacter();
        break;
    case 18:
        Banner();
        break;
    case 19:
        EnforceAnimation();
        break;
    case 20:
        WaitKeyUp();
        break;
    case 21:
        DestroyItem();
        break;
    case 22:
        Prompt();
        break;
    case 23:
        ChainEvent();
        break;
    case 24:
        CallEvent();
        break;
    case 25:
        Heal();
        break;                                  // effect
    case 26:
        EarthQuake();
        break;
    case 27:
        SaveMenu();
        break;
    case 28:
        EnableSave();
        break;
    case 29:
        DisableSave();
        break;
    case 30:
        ReviveChar();
        break;                            // effect
    case 31:
        RestoreMP();
        break;                             // effect
    case 32:
        Redraw();
        break;
    case 33:
        SText();
        break;
    case 34:
        DisableMenu();
        break;
    case 35:
        EnableMenu();
        break;
    case 36:
        Wait();
        break;
    case 37:
        SetFace();
        break;
    case 38:
        MapPaletteGradient();
        break;
    case 39:
        BoxFadeOut();
        break;
    case 40:
        BoxFadeIn();
        break;
    case 41:
        GiveGP();
        break;
    case 42:
        TakeGP();
        break;
    case 43:
        ChangeZone();
        break;
    case 44:
        GetItem();
        break;
    case 45:
        ForceEquip();
        break;
    case 46:
        GiveXP();
        break;
    case 47:
        Shop();
        break;
    case 48:
        PaletteMorph();
        break;
    case 49:
        ChangeCHR();
        break;
    case 50:
        readcontrols();
        break;
    case 51:
        VCPutPCX();
        break;
    case 52:
        HookTimer();
        break;
    case 53:
        HookRetrace();
        break;
    case 54:
        VCLoadPCX();
        break;
    case 55:
        VCBlitImage();
        break;
    case 57:
        VCClear();
        break;
    case 58:
        VCClearRegion();
        break;
    case 59:
        VCText();
        break;
    case 60:
        VCTBlitImage();
        break;
    case 61:
        Exit();
        break;
    case 62:
        Quit();
        break;
    case 63:
        VCCenterText();
        break;
    case 64:
        ResetTimer();
        break;
    case 65:
        VCBlitTile();
        break;
    case 66:
        Sys_ClearScreen();
        break;
    case 67:
        Sys_DisplayPCX();
        break;
    case 68:
        OldStartupMenu();
        break;
    case 69:
        vgadump();
        break;
    case 70:
        NewGame();
        break;
    case 71:
        LoadSaveErase(0);
        break;
    case 72:
        Delay();
        break;
    case 73:
        PartyMove();
        break;
    case 74:
        EntityMove();
        break;
    case 75:
        AutoOn();
        break;
    case 76:
        AutoOff();
        break;
    case 77:
        EntityMoveScript();
        break;
    case 78:
        VCTextNum();
        break;
    case 79:
        VCLoadRaw();
        break;

    case 80:
        VCBox();
        break;       /* -- ric: 21/Apr/98 -- */
    case 81:
        VCCharName();
        break;  /* -- ric: 21/Apr/98 -- */
    case 82:
        VCItemName();
        break;  /* -- ric: 21/Apr/98 -- */
    case 83:
        VCItemDesc();
        break;  /* -- ric: 21/Apr/98 -- */
    case 84:
        VCItemImage();
        break; /* -- ric: 22/Apr/98 -- */
    case 85:
        VCATextNum();
        break;  /* -- ric: 24/Apr/98 -- */
    case 86:
        VCSpc();
        break;       /* -- ric: 24/Apr/98 -- */
    case 87:
        CallEffect();
        break;  /* -- ric: 24/Apr/98 -- */
    case 88:
        CallScript();
        break;  /* -- ric: 25/Apr/98 -- */

    case 89:
        VCLine();
        break;
    case 90:
        GetMagic();
        break;
    case 91:
        BindKey();
        break;     /* -- ric: 03/May/98 -- */
    case 92:
        TextMenu();
        break;    /* -- ric: 03/May/98 -- */
    case 93:
        itemMenu();
        break;    /* -- ric: 03/May/98 -- */
    case 94:
        equipMenu();
        break;   /* -- ric: 03/May/98 -- */
    case 95:
        magicMenu();
        break;   /* -- ric: 03/May/98 -- */
    case 96:
        statusScreen();
        break;/* -- ric: 03/May/98 -- */
    case 97:
        VCCr2();
        break;       /* -- ric: 03/May/98 -- */
    case 98:
        VCSpellName();
        break;
    case 99:
        VCSpellDesc();
        break;
    case 100:
        VCSpellImage();
        break;
    case 101:
        MagicShop();
        break;
    case 102:
        VCTextBox();
        break;   /* -- ric: 04/May/98 -- */
    case 103:
        PlayVAS();
        break;
    case 104:
        SmallText();
        break;       // ANDY 03/Mar/99
    case 105:
        VCEllipse();
        break;       // ANDY 30/Jul/99
        //    case 104: VCMagicImage(); break;/* -- ric: 04/May/98 -- */
        //    case 105: vcwritelayer(); break; /* -- xBig_D: 05/May/98 */

    default:
        err("*error* Unknown library function in VC code");
    }
}

int ReadVar0(int var) {
    switch (var) {
    case 0:
        return tvar[0];
    case 1:
        return tvar[1];
    case 2:
        return tvar[2];
    case 3:
        return tvar[3];
    case 4:
        return tvar[4];
    case 5:
        return tvar[5];
    case 6:
        return tvar[6];
    case 7:
        return tvar[7];
    case 8:
        return tvar[8];
    case 9:
        return tvar[9];
    case 10:
        return tvar[10];
    case 11:
        return tvar[11];
    case 12:
        return tvar[12];
    case 13:
        return tvar[13];
    case 14:
        return tvar[14];
    case 15:
        return tvar[15];
    case 16:
        return tvar[16];
    case 17:
        return tvar[17];
    case 18:
        return tvar[18];
    case 19:
        return tvar[19];
    case 20:
        return tvar[20];
    case 21:
        return tvar[21];
    case 22:
        return tvar[22];
    case 23:
        return tvar[23];
    case 24:
        return tvar[24];
    case 25:
        return tvar[25];
    case 26:
        return numchars;
    case 27:
        return gp;
    case 28:
        return party[0].x / 16;
    case 29:
        return party[0].y / 16;
    case 30:
        return timer;
    case 31:
        return drawparty;
    case 32:
        return cameratracking;
    case 33:
        return xwin;
    case 34:
        return ywin;
    case 35:
        return b1;
    case 36:
        return b2;
    case 37:
        return b3;
    case 38:
        return b4;
    case 39:
        return up;
    case 40:
        return down;
    case 41:
        return left;
    case 42:
        return right;
    case 43:
        return an;
    case 44:
        return fade;
    case 45:
        return layer0;
    case 46:
        return layer1;
    case 47:
        return layervc;
    case 48:
        return quakex;
    case 49:
        return quakey;
    case 50:
        return quake;
    case 51:
        return screengradient;
    case 52:
        return pmultx;
    case 53:
        return pmulty;
    case 54:
        return pdivx;
    case 55:
        return pdivy;
    case 56:
        return mp_volume;
    case 57:
        return layerc;
    case 58:
        return cancelfade;
    case 59:
        return drawentities;
    case 60:
        return mapp[((party[0].cy) * xsize) + (party[0].cx)] >> 1;
    case 61:
        return map0[((party[0].cy) * xsize) + (party[0].cx)];
    case 62:
        return map1[((party[0].cy) * xsize) + (party[0].cx)];
    case 63:
        return foregroundlock;
    case 64:
        return xwin1;
    case 65:
        return ywin1;
    case 66:
        return layer1trans;
    case 67:
        return layervctrans;

    case 68:
        return oc; /* -- ric: 21/Apr/98 -- */
    case 69:
        return keepaz; /* -- ric: 03/May/98 -- */

    case 70:
        return layervc2; /* -- xBig_D: 05/May/98 */
    case 71:
        return layervc2trans; /* -- xBig_D: 05/May/98 */
    case 72:
        return layervcwrite; /* -- xBig_D: 05/May/98 */
    case 73:
        return mp_sngpos; /* -- xBig_D: 10/May/98 */
    }
}

WriteVar0(int var, int value) {
    switch (var) {
    case 0:
        tvar[0] = value;
        return;
    case 1:
        tvar[1] = value;
        return;
    case 2:
        tvar[2] = value;
        return;
    case 3:
        tvar[3] = value;
        return;
    case 4:
        tvar[4] = value;
        return;
    case 5:
        tvar[5] = value;
        return;
    case 6:
        tvar[6] = value;
        return;
    case 7:
        tvar[7] = value;
        return;
    case 8:
        tvar[8] = value;
        return;
    case 9:
        tvar[9] = value;
        return;
    case 10:
        tvar[10] = value;
        return;
    case 11:
        tvar[11] = value;
        return;
    case 12:
        tvar[12] = value;
        return;
    case 13:
        tvar[13] = value;
        return;
    case 14:
        tvar[14] = value;
        return;
    case 15:
        tvar[15] = value;
        return;
    case 16:
        tvar[16] = value;
        return;
    case 17:
        tvar[17] = value;
        return;
    case 18:
        tvar[18] = value;
        return;
    case 19:
        tvar[19] = value;
        return;
    case 20:
        tvar[20] = value;
        return;
    case 21:
        tvar[21] = value;
        return;
    case 22:
        tvar[22] = value;
        return;
    case 23:
        tvar[23] = value;
        return;
    case 24:
        tvar[24] = value;
        return;
    case 25:
        tvar[25] = value;
        return;
    case 26:
        return;
    case 27:
        gp = value;
        return;
    case 28:
        return;
    case 29:
        return;
    case 30:
        timer = value;
        return;
    case 31:
        drawparty = value;
        return;
    case 32:
        cameratracking = value;
        return;
    case 33:
        if (value < 0) {
            value = 0;
        }
        if (value > (xsize << 4) - 320) {
            value = (xsize << 4) - 320;
        }
        xwin = value;
        return;
    case 34:
        if (value < 0) {
            value = 0;
        }
        if (value > (ysize << 4) - 200) {
            value = (ysize << 4) - 200;
        }
        ywin = value;
        return;
    case 35:
        keyboard_map[kb1] = value;
        b1 = value;
        return;
    case 36:
        keyboard_map[kb2] = value;
        b2 = value;
        return;
    case 37:
        keyboard_map[kb3] = value;
        b3 = value;
        return;
    case 38:
        keyboard_map[kb4] = value;
        b4 = value;
        return;
    case 39:
        keyboard_map[SCAN_UP] = value;
        up = value;
        return;
    case 40:
        keyboard_map[SCAN_DOWN] = value;
        down = value;
        return;
    case 41:
        keyboard_map[SCAN_LEFT] = value;
        left = value;
        return;
    case 42:
        keyboard_map[SCAN_RIGHT] = value;
        right = value;
        return;
    case 43:
        an = value;
        return;
    case 44:
        fade = value;
        return;
    case 45:
        layer0 = value;
        return;
    case 46:
        layer1 = value;
        return;
    case 47:
        layervc = value;
        return;
    case 48:
        quakex = value;
        return;
    case 49:
        quakey = value;
        return;
    case 50:
        quake = value;
        return;
    case 51:
        screengradient = value;
        return;
    case 52:
        pmultx = value;
        return;
    case 53:
        pmulty = value;
        return;
    case 54:
        pdivx = value;
        return;
    case 55:
        pdivy = value;
        return;
    case 56:
        mp_volume = value;
        return;
    case 57:
        layerc = value;
        return;
    case 58:
        cancelfade = value;
        return;
    case 59:
        drawentities = value;
        return;
    case 60:
        return;
    case 61:
        return;
    case 62:
        return;
    case 63:
        foregroundlock = value;
        return;
    case 64:
        if (value < 0) {
            value = 0;
        }
        if (value > (xsize << 4) - 320) {
            value = (xsize << 4) - 320;
        }
        xwin1 = value;
        return;
    case 65:
        if (value < 0) {
            value = 0;
        }
        if (value > (ysize << 4) - 200) {
            value = (ysize << 4) - 200;
        }
        ywin1 = value;
        return;
    case 66:
        layer1trans = value;
        return;
    case 67:
        layervctrans = value;
        return;

    case 68:
        fontcolor(value);
        return; /* -- ric: 21/Apr/98 -- */
    case 69:
        keepaz = value;
        return; /* -- ric: 03/May/98 -- */

    case 70:
        layervc2 = value;
        return; /* -- xBig_D: 05/May/98 */
    case 71:
        layervc2trans = value;
        return; /* -- xBig_D: 05/May/98 */
    case 72:
        vcwritelayer(value);
        return; /* -- xBig_D: 05/May/98 */
    case 73:
        MP_SetPosition(value);
        return; /* -- xBig_D: 10/May/98 */
    }
}

int ReadVar1(int var, int arg1) {
    int i, j, l;

    switch (var) {
    case 0:
        return flags[arg1];
    case 1:
        if (party[0].facing == arg1) {
            return 1;
        } else {
            return 0;
        }
    case 2:
        for (i = 0; i < numchars; i++)
            if (partyidx[i] == arg1) {
                return i + 1;
            }
        return 0;
    case 3:
        for (j = 0; j < numchars; j++) {
            l = partyidx[j] - 1;
            for (i = 0; i < pstats[l].invcnt; i++)
                if (pstats[l].inv[i] == arg1) {
                    return 1;
                }
        }
        return 0;
    case 4:
        return varl[arg1];
    case 5:
        return partyidx[arg1 - 1] - 1;
    case 6:
        return pstats[arg1 - 1].exp;
    case 7:
        return pstats[arg1 - 1].curhp;
    case 8:
        return pstats[arg1 - 1].maxhp;
    case 9:
        return pstats[arg1 - 1].curmp;
    case 10:
        return pstats[arg1 - 1].maxmp;
    case 11:
        return keyboard_map[arg1];
    case 12:
        return (char) vcdatabuf[arg1];
    case 13:
        return party[arg1].specframe;
    case 14:
        return party[arg1].facing;
    case 15:
        return party[arg1].speed;
    case 16:
        return party[arg1].moving;
    case 17:
        return party[arg1].chrindex;
    case 18:
        return party[arg1].movecode;
    case 19:
        return party[arg1].activmode;
    case 20:
        return party[arg1].obsmode;
    case 21:
        return party[arg1].step;
    case 22:
        return party[arg1].delay;
    case 23:
        return party[arg1].cx;
    case 24:
        return party[arg1].cy;
    case 25:
        return party[arg1].x1;
    case 26:
        return party[arg1].x2;
    case 27:
        return party[arg1].y1;
    case 28:
        return party[arg1].y2;
    case 29:
        return party[arg1].face;
    case 30:
        return party[arg1].chasing;
    case 31:
        return party[arg1].chasedist;
    case 32:
        return party[arg1].chasespeed;
    case 33:
        return party[arg1].scriptofs;
    case 34:
        return pstats[arg1 - 1].atk;
    case 35:
        return pstats[arg1 - 1].def;
    case 36:
        return pstats[arg1 - 1].hitc;
    case 37:
        return pstats[arg1 - 1].dodc;
    case 38:
        return pstats[arg1 - 1].magc;
    case 39:
        return pstats[arg1 - 1].mgrc;
    case 40:
        return pstats[arg1 - 1].reac;
    case 41:
        return pstats[arg1 - 1].mblc;
    case 42:
        return pstats[arg1 - 1].ferc;

    case 43:
        return items[arg1].useflag;   /* -- ric: 21/Apr/98 -- */
    case 44:
        return items[arg1].useeffect; /* -- ric: 24/Apr/98 -- */
    case 45:
        return items[arg1].itemtype;  /* -- ric: 24/Apr/98 -- */
    case 46:
        return items[arg1].equipflag; /* -- ric: 24/Apr/98 -- */
    case 47:
        return items[arg1].equipidx;  /* -- ric: 24/Apr/98 -- */
    case 48:
        return items[arg1].price;     /* -- ric: 24/Apr/98 -- */
    case 49:
        return magic[arg1].useflag;
    case 50:
        return magic[arg1].useeffect;
    case 51:
        return magic[arg1].itemtype;
    case 52:
        return magic[arg1].cost;
    case 53:
        return magic[arg1].price;
    case 54:
        return pstats[arg1 - 1].lv;   /* -- ric: 03/May/98 -- */
    case 55:
        return pstats[arg1 - 1].nxt;  /* -- ric: 03/May/98 -- */
    case 56:
        return pstats[arg1 - 1].status; /* -- ric: 03/May/98 -- */

    case 57:
        for (j = 0; j < numchars; j++) {
            l = partyidx[j] - 1;
            for (i = 0; i < pstats[l].magcnt; i++)
                if (pstats[l].maginv[i] == arg1) {
                    return 1;
                }
        }
        return 0;

    }
}

WriteVar1(int var, int arg1, int value) {
    switch (var) {
    case 0:
        flags[arg1] = value;
        return;
    case 1:
        return;
    case 2:
        return;
    case 3:
        return;
    case 4:
        varl[arg1] = value;
        return;
    case 5:
        return;
    case 6:
        return;
    case 7:
        pstats[arg1 - 1].curhp = value;
        return;
    case 8:
        pstats[arg1 - 1].maxhp = value;
        return;
    case 9:
        pstats[arg1 - 1].curmp = value;
        return;
    case 10:
        pstats[arg1 - 1].maxmp = value;
        return;
    case 11:
        keyboard_map[arg1] = value;
        return;
    case 12:
        vcdatabuf[arg1] = (char) value;
        return;
    case 13:
        party[arg1].specframe = value;
        return;
    case 14:
        party[arg1].facing = value;
        return;
    case 15:
        party[arg1].speed = value;
        return;
    case 16:
        party[arg1].moving = value;
        return;
    case 17:
        party[arg1].chrindex = value;
        return;
    case 18:
        party[arg1].movecode = value;
        return;
    case 19:
        party[arg1].activmode = value;
        return;
    case 20:
        party[arg1].obsmode = value;
        return;
    case 21:
        party[arg1].step = value;
        return;
    case 22:
        party[arg1].delay = value;
        return;
    case 23:
        party[arg1].cx = value;
        party[arg1].x = value * 16;
        party[arg1].moving = 0;
        party[arg1].movcnt = 0;
        return;
    case 24:
        party[arg1].cy = value;
        party[arg1].y = value * 16;
        party[arg1].moving = 0;
        party[arg1].movcnt = 0;
        return;
    case 25:
        party[arg1].x1 = value;
        return;
    case 26:
        party[arg1].x2 = value;
        return;
    case 27:
        party[arg1].y1 = value;
        return;
    case 28:
        party[arg1].y2 = value;
        return;
    case 29:
        party[arg1].face = value;
        return;
    case 30:
        party[arg1].chasing = value;
        return;
    case 31:
        party[arg1].chasedist = value;
        return;
    case 32:
        party[arg1].chasespeed = value;
        return;
    case 33:
        return;

        // ADDED BY ANDY FRIESEN!!!
    case 56:
        pstats[arg1 - 1].status = value;
        return;
        // END OF STUFF ANDY FRIESEN ADDED!!!
    }
}

int ReadVar2(int var, int arg1, int arg2) {
    int i, j, l;

    switch (var) {
    case 0:
        arg2++;
        while (1) {
            if (arg2 < arg1) {
                i = arg2;
                arg2 = arg1;
                arg1 = i;
            }
            j = (rand() % (arg2 - arg1));
            j += arg1;
            return j;
        }
    case 1:
        return (unsigned char) vcscreen[(arg2 * 320) + arg1];
    case 2:
        return pstats[arg1].inv[arg2];

        /* -- ric: 24/Apr/98 --
         * CanEquip(party.dat index, item.dat index) (R) (24/Apr/98)
         * ChooseChar(x,y)                           (R) (25/Apr/98) -- */
    case 3:
        return (items[arg2].equipflag && equip[items[arg2].equipidx].equipable[arg1]);
    case 4:
        return ChooseChar(arg1, arg2);

        /* NichG: Whenever */
    case 5:
        return ObstructionAt(arg1, arg2);
    case 6:
        return pstats[arg1].maginv[arg2];
        // ANDY May 21/99
    case 7:
        return ((flags[arg1] >> arg2) & 1);
    }
}

WriteVar2(int var, int arg1, int arg2, int value) {
    unsigned int mask;
    switch (var) {
    case 0:
        return;
    case 1:
        vcscreen[(arg2 * 320) + arg1] = value;
        return;
    case 2:
        pstats[arg1].inv[arg2] = (char) value;
        break;
        // ANDY May 21/99
    case 7:
        mask = (value & 1);
        if (mask) {
            flags[arg1] = (flags[arg1] | (mask << arg2));
            return;
        }
        mask = 1 << arg2;
        mask = 0 - 1 - mask;
        flags[arg1] = (flags[arg1] & mask);
        return;
    }
}
