
#include <cassert>
#include <stdlib.h>
#include <string.h>
#include "engine.h"
#include "main.h"
#include "menu.h"
#include "vga.h"
#include "control.h"
#include "render.h"
#include "ricvc.h"
#include "sound.h"
#include "timer.h"
#include "vc.h"
#include "vclib.h"

/* THE FOLLOWING CODE IS MOTHBALLED
PlayFli()
{
   int i, i2, frames;
 // play_fli (&stringbuffer);


  GrabString(&stringbuffer);

  LoadPCXHeaderNP(&stringbuffer);


  frames = 10;
  i = 0;
  while (i < frames)
  {
  for (i2=0; i2<depth; i2++)
  {
     vidoffset=(i2*width);
     ReadPCXLine(vcdatabuf);
  }
  VCtcopysprite(0,0,320,200,vcdatabuf+(i*64000));
  drawmap();
  vgadump();
  i++;
  }
  fclose(pcxf);

}

PixelBlit3d()
{
int x, y, z, xnew, ynew, color;

x=ResolveOperand();
y=ResolveOperand();
z=ResolveOperand();
color=ResolveOperand();
*/
/*
if (x > 16384) x = -1*(32768-x);
if (y > 16384) y = -1*(32768-y);
if (z > 16384) z = -1*(32768-z);
*/
/*
if (z == 0) z++;

xnew = x+160+(x/(z/256));
ynew = y+100+(y/(z/256));

vcscreen[(ynew*320)+xnew] = color;
}
*/



int sgn (long a) {
    if (a > 0) {
        return +1;
    } else if (a < 0) {
        return -1;
    } else {
        return 0;
    }
}

void Line2d(int a, int b, int c, int d, int col) {

    long u, s, v, d1x, d1y, d2x, d2y, m, n;
    int  i;

    u   = c - a;
    v   = d - b;
    d1x = sgn(u);
    d1y = sgn(v);
    d2x = sgn(u);
    d2y = 0;
    m   = abs(u);
    n   = abs(v);

    if (m <= n) {
        d2x = 0;
        d2y = sgn(v);
        m   = abs(v);
        n   = abs(u);
    }

    s = (int)(m / 2);

    for (i = 0; i < (int)(m); i++) {
        vcscreen[(b * 320) + a] = col;
        s += n;
        if (s >= m) {
            s -= m;
            a += d1x;
            b += d1y;
        } else {
            a += d2x;
            b += d2y;
        }
    }

}

void VCLine() {
    int x1, y1, x2, y2, color, slope;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    x2 = ResolveOperand();
    y2 = ResolveOperand();
    color = ResolveOperand();

    Line2d (x1, y1, x2, y2, color);

}

void Line3d (int x1, int y1, int z1, int x2, int y2, int z2, int color) {
    int x1new, y1new, x2new, y2new;

    if (x1 > 2000) {
        x1 = -1 * (4000 - x1);
    }

    if (y1 > 2000) {
        y1 = -1 * (4000 - y1);
    }

    if (z1 > 2000) {
        z1 = -1 * (4000 - z1);
    }

    if (x2 > 2000) {
        x2 = -1 * (4000 - x2);
    }

    if (y2 > 2000) {
        y2 = -1 * (4000 - y2);
    }

    if (z2 > 2000) {
        z2 = -1 * (4000 - z2);
    }

    if (z1 == 0) {
        z1++;
    }

    x1new = x1 + 160 + (x1 / (z1 / 256));
    y1new = y1 + 100 + (y1 / (z1 / 256));

    if (z2 == 0) {
        z2++;
    }

    x2new = x2 + 160 + (x2 / (z2 * 256));
    y2new = y2 + 100 + (y2 / (z2 * 256));

    Line2d (x1new, y1new, x2new, y2new, color);
}

void VCLine3d() {
    int x1, y1, z1, x2, y2, z2, color;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    z1 = ResolveOperand();
    x2 = ResolveOperand();
    y2 = ResolveOperand();
    z2 = ResolveOperand();

    /*
    if (x1 > 16384) x1 = -1*(32768-x1);
    if (y1 > 16384) y1 = -1*(32768-y1);
    if (z1 > 16384) z1 = -1*(32768-z1);
    if (x2 > 16384) x2 = -1*(32768-x2);
    if (y2 > 16384) y2 = -1*(32768-y2);
    if (z2 > 16384) z2 = -1*(32768-z2);
    */

    color = ResolveOperand();

    Line3d (x1, y1, z1, x2, y2, z2, color);
}

void Exc() {
    char* str1, *str2, *str3;

    str1 = code;
    GrabString(strbuf);
    str2 = code;
    GrabString(strbuf);
    assert(!"execl doesn't work in NaCl");//execl (str1, str1, str2);
}

// NEW (MAGIC)

void GetMagic() {
    short int c, d;
    int i, j;
    int alreadyhave = 0;

    c = ResolveOperand() - 1;
    d = ResolveOperand();

    i = 0;

    while (i < 24) {
        if (d == pstats[c].maginv[i]) {
            alreadyhave = 1;
        }
        i++;
    }

    j = pstats[c].magcnt;
    if (!alreadyhave) {
        if (j != 24) {
            pstats[c].maginv[j] = d;
            pstats[c].magcnt++;
        } else {
            pstats[c].maginv[j - 1] = d;
        }
    }

}

void VCSpellName() { /* -- adapted from ric: ??/???/?? -- */
    int x1, y1, i, align;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    i = ResolveOperand();
    align = ResolveOperand();
    VCAString(x1, y1, magic[i].name, align);
}

void VCSpellDesc() { /* -- adapted from ric: ??/???/?? --  */
    int x1, y1, i, align;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    i = ResolveOperand();
    align = ResolveOperand();
    VCAString(x1, y1, magic[i].desc, align);
}

void VCSpellImage() { /* -- adapted from ric: ??/???/?? -- */
    int x1, y1, i, gf;
    unsigned char gsimg[512];
    unsigned char* img;

    x1 = ResolveOperand();
    y1 = ResolveOperand();
    i = ResolveOperand();
    gf = ResolveOperand();
    img = magicicons + (magic[i].icon << 8);
    if (gf) {
        grey(16, 16, img, gsimg);
        img = gsimg;
    }

    VCtcopysprite(x1, y1, 16, 16, img);
}

void MagicShop() {
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
            MBuyMenu();
        }
        if (p) {
            MSellMenu();
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

// Play VAS?
PlayVAS() {
    int i, i2, frames, speed, ansave, sizex, sizey, sizeoff, wherex, wherey;
    // play_fli (&stringbuffer);
    ansave = an;
    an = 1;

    GrabString(&stringbuffer);
    speed = ResolveOperand();
    sizex = ResolveOperand();
    sizey = ResolveOperand();
    wherex = ResolveOperand();
    wherey = ResolveOperand();

    LoadPCXHeaderNP(&stringbuffer);


    frames = depth / sizey;
    i = 0;

    sizeoff = sizex * sizey; //calc offset vals

    for (i2 = 0; i2 < depth; i2++) {
        vidoffset = (i2 * width);
        ReadPCXLine(vcdatabuf);
    }
    fclose(pcxf);

    timer_count = 0;
    while (i < frames) {
        memset(vcscreen, 0, 64000);
        VCtcopysprite(wherex, wherey, sizex, sizey, vcdatabuf + (i * sizeoff));
        drawmap();
        vgadump();

        i2 = 0;

        while (timer_count < (speed)) {
            i2++;
            i2--;
        }
        timer_count = 0;
        i++;
    }
    timer_count = 0;
    an = ansave;

}

