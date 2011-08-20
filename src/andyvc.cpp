/* ANDY ADDED ALL THIS STUFF   *
 * Well, OK it's not that much *
 *******************************
 * SmallText     Mar  3, '99   *
 * VCEllipse     Jul 30, '99   */

#include <stdlib.h>
#include <string.h>
#include "control.h"
#include "main.h"
#include "menu.h"
#include "render.h"
#include "timer.h"
#include "vc.h"
#include "vga.h"

void SmallText() {
    char* buf1, *buf2;
    char* opt;
    int  x1, y1, x2, y2;
    int  p, nv;
    int  first = 1;
    char len = 0;
    char numberoflines = 3;

    x1 = ResolveOperand() + 16;
    y1 = ResolveOperand() + 16;

    nv = GrabC();
    buf1 = code;
    for (p = 0; p < nv; p++) {
        opt = code;
        GrabString(strbuf);
        if (len < strlen(opt)) { len = strlen(opt); }
    }
    buf2 = code;
    numberoflines = nv;
    x2 = x1 + 8 + len * 8;
    y2 = y1 + 8 + numberoflines * 10;

drawloop:
    drawmap();
    tmenubox(x1, y1, x2, y2);
    for (p = 0; p < nv; p++) {
        opt = code;
        GrabString(strbuf);
        gotoxy(x1 + 5, 4 + y1 + (p * 10));
        printstring(opt);
    }
    buf2 = code;
    code = buf1;

    vgadump();
    readcontrols();

    if (first == 2) if (b1 || b2 || b4) { goto drawloop; }
        else { an = 0; setTimerCount(0); return; }
    if (first && !b1 && !b2 && !b4) { first = 0; }
    else if (first) { goto drawloop; }

    while (!b4 && !b2 && !b1) { goto drawloop; }
    while (b4 || b2 || b1) { first = 2; goto drawloop; }
}

void VCEllipse() {
    int mx, my, a, b, color;

    int x, mx1, mx2, my1, my2;
    long aq, bq, dx, dy, r, rx, ry;

    mx    = ResolveOperand();
    my    = ResolveOperand();
    a     = ResolveOperand();
    b     = ResolveOperand();
    color = ResolveOperand();

    vcscreen[my * 320 + mx + a] = color;
    vcscreen[my * 320 + mx - a] = color;

    mx1 = mx - a;
    my1 = my;
    mx2 = mx + a;
    my2 = my;

    aq = a * a;
    bq = b * b;
    dx = aq << 1;
    dy = bq << 1;
    r = a * bq;
    rx = r << 1;
    ry = 0;
    x = a;

    while (x > 0) {
        if (r > 0) {
            my1++;
            my2--;
            ry += dx;
            r -= ry;
        }
        if (r <= 0) {
            x--;
            mx1++;
            mx2--;
            rx -= dy;
            r += rx;
        }
        vcscreen[my1 * 320 + mx1] = color;
        vcscreen[my1 * 320 + mx2] = color;
        vcscreen[my2 * 320 + mx1] = color;
        vcscreen[my2 * 320 + mx2] = color;
    }
}

