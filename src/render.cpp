// render.c
// Main render engine code
// Copyright (C)1997 BJ Eirich

#include <stdio.h>
#include "engine.h"
#include "entity.h"
#include "vga.h"
extern err(char *ermsg);
extern char *strbuf;

// ============================ Data ============================

char drawparty=1,cameratracking=1;     // Render control flags
char drawentities=1;                   // Render control flags
char layer0=1,layer1=1,layervc=0,layervc2=0;      // Render control flags
char layervcwrite=1;
unsigned char *vcscreen,hookretrace,*vcscreen2,*vcscreen1;   // VClayer buffer ptr, rtr hook
char flipped[2048];                    // If animation bi-dir loop has flipped
int tileidx[2048];                     // Animation-adjusted tile index
int vspspeed;                          // vsp-speed up trick on/off flag
char *vspmask;                         // vspspeed precalc transparency mask
int foregroundlock=1,xwin1=0,ywin1=0;  // front layer independent scrolling stuff
unsigned char draworder[100],numdraw=0;// y-sorted draw order buffer
unsigned char layer1trans=0, layervctrans=0, layervc2trans=0;

int quake=0,quakex,quakey,qswitch=0;   // render-quake flags
int screengradient=0;                  // fullscreen mappalettegradient
int (*DrawLayer1) (int xw, int yw);    // DrawLayer1 "driver"-style ptr

DrawLayer1NoSpeed(int xw, int yw);     // DrawLayer1 funcs pre-defines
DrawLayer1Speed(int xw, int yw);       // DrawLayer1 funcs pre-defines

extern int vspm;                       // external def-amount of VSP memory

// ============================ Code ============================

InitRenderSystem() {
    vcscreen1=(char *) valloc(64000,"vcscreen1");
    //memset(vcscreen1, 0, 64000);
    vcscreen2=(char *) valloc(64000,"vcscreen2");
    //memset(vcscreen2, 0, 64000);
    vcscreen=vcscreen1;
    if (vspspeed) {
        vspmask=(char *) valloc(vspm,"vspmask");
        DrawLayer1=&DrawLayer1Speed;
    } else DrawLayer1=&DrawLayer1NoSpeed;
}

CalcVSPMask() {
    int i;

    for (i=0; i<(numtiles*256); i++) {
        if (vsp0[i]) vspmask[i]=0;
        else vspmask[i]=255;
    }
}

drawchar(int i, int xw, int yw) {
    char *img, fr;
    int dx,dy,drawmode=0;

    dx=party[i].x-xw+16;
    dy=party[i].y-yw;

    if (dx<0 || dx>336) return;
    if (dy<-16 || dy>216) return;
    if (dy<0) drawmode=1;
    if (dy>200) drawmode=2;

    img=chrs+(party[i].chrindex*15360);

    switch (party[i].facing) {
    case 0:
        fr=0;
        break;
    case 1:
        fr=5;
        break;
    case 2:
        fr=10;
        break;
    case 3:
        fr=15;
        break;
    }
    if (party[i].moving && !party[i].specframe) {
        if ((party[i].framectr>10) && (party[i].framectr<21)) fr+=1;
        if ((party[i].framectr>20) && (party[i].framectr<31)) fr+=2;
        if ((party[i].framectr>30) && (party[i].framectr<41)) fr+=1;
        if ((party[i].framectr>50) && (party[i].framectr<61)) fr+=3;
        if ((party[i].framectr>60) && (party[i].framectr<71)) fr+=4;
        if ((party[i].framectr>70) && (party[i].framectr<81)) fr+=3;
    }

    if (party[i].specframe) fr=party[i].specframe;
    img+=(fr*512);

    switch (drawmode) {
    case 0:
        tcopysprite(dx,dy,16,32,img);
        break;
    case 1:
        tcopysprite(dx,dy+16,16,16,img+256);
        break;
    case 2:
        tcopysprite(dx,dy,16,16,img);
        break;
    }
}

SwitchOrder(int i, int j) {
    unsigned char c;

    c=draworder[i];
    draworder[i]=draworder[j];
    draworder[j]=c;
}

SortDrawOrder() {
    int i,j;

    for (i=1; i<numdraw; i++)
        for (j=numdraw-1; j>=i; j--)
            if (party[draworder[j-1]].y > party[draworder[j]].y)
                SwitchOrder(j,j-1);
}

setdrawchar(unsigned char i) {
    draworder[numdraw]=i;
    numdraw++;
}

drawcharacters(int xw,int yw) {
    int i;

    memset(&draworder,0,100);
    numdraw=0;
    if (drawparty)
        for (i=0; i<numchars; i++)
            setdrawchar(i);

    if (drawentities)
        for (i=5; i<entities; i++)
            setdrawchar(i);

    if (autoent && drawentities)
        for (i=95; i<95+numchars; i++)
            setdrawchar(i);

    SortDrawOrder();
    drawchars(xw,yw);
}

drawchars(int xw, int yw) {
    int i;

    for (i=0; i<numdraw; i++)
        drawchar(draworder[i],xw,yw);
}

ProcessEarthQuake() {
    int nx,ny;

    nx=xwin;
    ny=ywin;
    if (!qswitch) {
        if (quakex>xwin && quakex) nx=0;
        else if (quakex) nx=xwin-(rand() % quakex);
        if (quakey>ywin && quakey) ny=0;
        else if (quakey) ny=ywin-(rand() % quakey);
    } else {
        if (quakex) nx=xwin+(rand() % quakex);
        if (quakey) ny=ywin+(rand() % quakey);
    }

    qswitch=qswitch ^ 1;
    drawmaploc(nx,ny);
}

drawvclayer() {
    if (layervctrans==1)  Tcopysprite(16,16,320,200,vcscreen1);
    else if (layervctrans==2) _Tcopysprite(16,16,320,200,vcscreen1);
    else tcopysprite(16,16,320,200,vcscreen1);
}
drawvclayer2() {
    if (layervc2trans==1)  Tcopysprite(16,16,320,200,vcscreen2);
    else if (layervc2trans==2) _Tcopysprite(16,16,320,200,vcscreen2);
    else tcopysprite(16,16,320,200,vcscreen2);
}


void drawmap() {
    if (cameratracking) {
        if (party[0].x>155) xwin=(party[0].x-155);
        else xwin=0;
        if (party[0].y>95) ywin=(party[0].y-95);
        else ywin=0;
        if (xwin>((xsize*16)-320)) xwin=((xsize*16)-320);
        if (ywin>((ysize*16)-208)) ywin=((ysize*16)-208); /* -- ric: 28/Apr/98 -- */
    }

    if (quake) {
        ProcessEarthQuake();
        return;
    }
    if (hookretrace) ExecuteHookedScript(hookretrace);
    if (layer0) DrawLayer0(xwin,ywin);
    else memset(virscr+5648, 0, 70368);
    if (layervc==3) drawvclayer();
    if ((!layerc) || (layerc==3)) drawcharacters(xwin,ywin);
    if (layervc==2) drawvclayer();
    if (layer1 && foregroundlock) DrawLayer1(xwin,ywin);
    else if (layer1) DrawLayer1(xwin1,ywin1);
    if ((layerc==1) || (layerc==2)) drawcharacters(xwin,ywin);
    if (layervc==1) drawvclayer();
    if (layervc2==1) drawvclayer2();
    if (screengradient) ColorField(16,16,336,216,&scrnxlatbl);
}

drawmaploc(int xw, int yw) {
    if (hookretrace) ExecuteScript(hookretrace);
    if (layer0) DrawLayer0(xw,yw);
    else memset(virscr+5648, 0, 70368);
    if ((!layerc) || (layerc==3) && drawparty) drawcharacters(xw,yw);
    if (layer1) DrawLayer1(xw,yw);
    if ((layerc==1) || (layerc==2) && drawparty) drawcharacters(xw,yw);
    if (layervc) drawvclayer();
    if (layervc2) drawvclayer2();
    if (screengradient) ColorField(16,16,336,216,&scrnxlatbl);
}

DrawLayer0(int xw, int yw) {
    char *img;
    int i,j,oxw,oyw;

    if ((!layerc) || (layerc==1) || (layerc==3)) {
        oxw=xw;
        oyw=yw;
        xtc=xw>>4;
        ytc=yw>>4;
    } else {
        oxw=xw*pmultx/pdivx;
        oyw=yw*pmulty/pdivy;
        xtc=oxw>>4;
        ytc=oyw>>4;
    }
    xofs=(16-(oxw&15));
    yofs=(16-(oyw&15));

    for (i=0; i<14; i++)
        for (j=0; j<21; j++) {
            img=vsp0+(tileidx[map0[(((ytc+i)%ysize)*xsize)+((xtc+j)%xsize)]]<<8);
            copytile((j<<4)+xofs,(i<<4)+yofs,img);
        }
}

DrawLayer1Trans(int xw, int yw) {
    char *img;
    int i,j,oxw,oyw;

    if (layerc<3) {
        oxw=xw;
        oyw=yw;
        xtc=xw>>4;
        ytc=yw>>4;
    } else {
        oxw=xw*pmultx/pdivx;
        oyw=yw*pmulty/pdivy;
        xtc=oxw>>4;
        ytc=oyw>>4;
    }
    xofs=(16-(oxw&15));
    yofs=(16-(oyw&15));

    for (i=0; i<14; i++)
        for (j=0; j<21; j++) {
            img=vsp0+(tileidx[map1[(((ytc+i) % ysize)*xsize)+((xtc+j)%xsize)]]<<8);
            if (img!=vsp0) Tcopysprite((j<<4)+xofs,(i<<4)+yofs,16,16,img);
        }
}

_DrawLayer1Trans(int xw, int yw) {
    char *img;
    int i,j,oxw,oyw;

    if (layerc<3) {
        oxw=xw;
        oyw=yw;
        xtc=xw>>4;
        ytc=yw>>4;
    } else {
        oxw=xw*pmultx/pdivx;
        oyw=yw*pmulty/pdivy;
        xtc=oxw>>4;
        ytc=oyw>>4;
    }
    xofs=(16-(oxw&15));
    yofs=(16-(oyw&15));

    for (i=0; i<14; i++)
        for (j=0; j<21; j++) {
            img=vsp0+(tileidx[map1[(((ytc+i) % ysize)*xsize)+((xtc+j)%xsize)]]<<8);
            if (img!=vsp0) _Tcopysprite((j<<4)+xofs,(i<<4)+yofs,16,16,img);
        }
}


DrawLayer1NoSpeed(int xw, int yw) {
    char *img;
    int i,j,oxw,oyw;

    if (layer1trans==1) {
        DrawLayer1Trans(xw, yw);
        return;
    }

    if (layer1trans==2) {
        _DrawLayer1Trans(xw, yw);
        return;
    }

    if (layerc<3) {
        oxw=xw;
        oyw=yw;
        xtc=xw>>4;
        ytc=yw>>4;
    } else {
        oxw=xw*pmultx/pdivx;
        oyw=yw*pmulty/pdivy;
        xtc=oxw>>4;
        ytc=oyw>>4;
    }
    xofs=(16-(oxw&15));
    yofs=(16-(oyw&15));

    for (i=0; i<14; i++)
        for (j=0; j<21; j++) {
            img=vsp0+(tileidx[map1[(((ytc+i) % ysize)*xsize)+((xtc+j)%xsize)]]<<8);
            if (img!=vsp0) tcopysprite((j<<4)+xofs,(i<<4)+yofs,16,16,img);
        }
}

DrawLayer1Speed(int xw, int yw) {
    char *img;
    int i,j,oxw,oyw,a;

    if (layer1trans==1) {
        DrawLayer1Trans(xw, yw);
        return;
    }

    if (layer1trans==2) {
        _DrawLayer1Trans(xw, yw);
        return;
    }

    if (layerc<3) {
        oxw=xw;
        oyw=yw;
        xtc=xw>>4;
        ytc=yw>>4;
    } else {
        oxw=xw*pmultx/pdivx;
        oyw=yw*pmulty/pdivy;
        xtc=oxw>>4;
        ytc=oyw>>4;
    }
    xofs=(16-(oxw&15));
    yofs=(16-(oyw&15));

    for (i=0; i<14; i++)
        for (j=0; j<21; j++) {
            a=(tileidx[map1[(((ytc+i) % ysize)*xsize)+((xtc+j)%xsize)]]<<8);
            img=vsp0+a;
            if (img!=vsp0) tcopytile((j<<4)+xofs,(i<<4)+yofs,img,vspmask+a);
        }
}

// ============================= Animation code ==========================

AnimateTile(char i, int l) {
    switch (va0[i].mode) {
    case 0:
        if (tileidx[l]<va0[i].finish) tileidx[l]++;
        else tileidx[l]=va0[i].start;
        break;
    case 1:
        if (tileidx[l]>va0[i].start) tileidx[l]--;
        else tileidx[l]=va0[i].finish;
        break;
    case 2:
        tileidx[l]=random(va0[i].start,va0[i].finish);
        break;
    case 3:
        if (flipped[l]) {
            if (tileidx[l]!=va0[i].start) tileidx[l]--;
            else {
                tileidx[l]++;
                flipped[l]=0;
            }
        } else {
            if (tileidx[l]!=va0[i].finish) tileidx[l]++;
            else {
                tileidx[l]--;
                flipped[l]=1;
            }
        }
    }
}

animate(char i) {
    int l;

    vadelay[i]=0;
    for (l=va0[i].start; l<=va0[i].finish; l++)
        AnimateTile(i,l);
}
