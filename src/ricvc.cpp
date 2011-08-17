/*  -- Ric's extensions to VCLIB.C --
 * Copyright (C)1998 Richard Lau
 *
 * Added internal functions:
 *       grey(width, height, *src, *dest) (24/Apr/98)
 *       VCvline (21/Apr/98)
 *       VCborder (21/Apr/98)
 *       VCColorField(x1, y1, x2, y2, unsigned char *colortbl) (24/Apr/98)
 *       ExecuteStartUpScript(s)                   (25/Apr/98)
 *       VCAString(x1, y1, char *strng, align)     (30/Apr/98)
 * Added functions:
 *       VCBox(x1,y1,x2,y2); (21/Apr/98)
 *       VCCharName(x, y, party.dat index, align); (21/Apr/98)
 *       VCItemName(x, y, items.dat index, align); (21/Apr/98)
 *       VCItemDesc(x, y, items.dat index, align); (21/Apr/98)
 *       VCItemImage(x, y, items.dat index, greyflag); (22/Apr/98)
 *       VCATextNum(x, y, number, align);              (24/Apr/98) ANDY ALTERED THIS SLIGHTLY (May 27,1999)
 *       VCSpc(x, y, speech portrait, greyflag);       (24/Apr/98)
 *       CallEffect(event number, option var1...);     (24/Apr/98)
 *       CallScript(event number, option var1...);     (25/Apr/98)
 *       BindKey(key code, script);                    (03/Apr/98)
 *       TextMenu(x,y,flag,ptr,"opt1","opt2",..);      (04/May/98)
 *       ItemMenu(roster order index);                 (03/May/98)
 *       EquipMenu(roster order index);                (03/May/98)
 *       MagicMenu(roster order index);                (03/May/98)
 *       StatusScreen(roster order index);             (03/May/98)
 *       VCCr2(x, y, roster order index, greyflag);    (03/May/98)
 *       VCTextBox(x,y,ptr,"opt1","opt2",..);     (04/May/98)
 *       VCMagicImage(x, y, items.dat index, greyflag);(04/May/98)
 * Added var0:
 *       FontColor (RW) (21/Apr/98)
 * Added var1:
 *       Item.Use (R) (21/Apr/98)
 *       Item.Effect     (R) (24/Apr/98)
 *       Item.Type       (R) (24/Apr/98)
 *       Item.EquipType  (R) (24/Apr/98)
 *       Item.EquipIndex (R) (24/Apr/98)
 *       Item.Price      (R) (24/Apr/98)
 * Added var2:
 *       CanEquip(party.dat index, item.dat index) (R) (24/Apr/98)
 *       ChooseChar(x,y)                           (R) (25/Apr/98)
 */

#define bgcolor 154
#define grey1 14
#define grey2 26
extern unsigned char oc;
extern char *speech;
extern char *strbuf;
extern unsigned int effectofstbl[1024];
extern unsigned int startupofstbl[1024];
extern tvar[26], varl[10];
extern char *code,*basevc,*startupvc,*menuptr;

/* -- Function declarations -- */
static grey(int width, int height, unsigned char *src, unsigned char *dest);
static VChline(int x, int y, int x2, char c);
static VCvline (int x, int y, int y2, char c);
static VCColorField(int x1, int y1, int x2, int y2, unsigned char *colortbl);
static VCborder(int x1, int y1, int x2, int y2);
static VCBox();
static VCAString(int x1, int y1, char *strng, int align);
static VCCharName();
static VCItemName();
static VCItemDesc();
static VCItemImage();
static VCATextNum();
static VCSpc();
static CallEffect();
static CallScript();
static BindKey();
static TextMenu();
static itemMenu();
static equipMenu();
static magicMenu();
static statusScreen();
static VCCr2();
static int ChooseChar();

static grey(int width, int height, unsigned char *src, unsigned char *dest)
/* -- ric: 24/Apr/98 -- */
{
    int i,j;
    unsigned char r,g,b,c,newc;

    for (j=0; j<height; j++)
        for (i=0; i<width; i++) {
            c=src[(j*width)+i];
            r=pal[(c+(c<<1))];
            g=pal[(c+(c<<1))+1];
            b=pal[(c+(c<<1))+2];
            newc=(r+g+b)/6;
            if (!newc && c) newc=1;
            dest[(j*width)+i]=newc;
        }
}

static VCvline(int x, int y, int y2, char c) { /* -- ric: 30/Apr/98 -- */
    int i,j;
    i=y2-y;

    do {
        i--;
        j=y+i;
        vcscreen[(j<<8) + (j<<6)+x]=c;
    } while (i);
}

static VCColorField(int x1, int y1, int x2, int y2, unsigned char *colortbl)
/* -- ric: 22/Apr/98 -- */
/* Simulate the ColorField function on the VC layer.
 * Used wrong spelling so as to avoid confusing the Americans ;)
 * Note: the visible screen starts at (16,16) on the virscr buffer while
 *       vcscreen starts at (0,0). We need to redraw the screen before
 *       drawing the ColorField to achieve the translucency effect.
 */
{
    int j,i,k;
    unsigned char c;
    unsigned char *tmpscreen;

    tmpscreen=virscr+5632; /* Make tmpscreen point to the virscr buffer, */
    /* offsetting by 5632 (16*352, i.e. 16 lines) */
    drawmap();             /* Redraw the map                             */

    j=y1;
    do {
        i=x1;
        do {
            k=(j<<8)+(j<<6);              /* k=j*320                       */
            c=tmpscreen[k+(j<<5)+i+16];   /* k+(j<<5)=j*352                */
            vcscreen[k+i]=colortbl[c];
            i++;
        } while (i!=x2);
        j++;
    } while (j!=y2);

}

static VCborder(int x1, int y1, int x2, int y2)
/* -- ric: 21/Apr/98 -- */
/* Taken from MENU2.C and adapted for the VC layer
 */
{
    VCvline(x1+1,y1+1,y2-1,grey1);
    VCvline(x1+2,y1+2,y2-2,grey2);
    VCvline(x1+3,y1+2,y2-2,grey2);
    VCvline(x1+4,y1+3,y2-3,grey1);

    VCvline(x2-1,y1+1,y2-1,grey1);
    VCvline(x2-2,y1+2,y2-2,grey2);
    VCvline(x2-3,y1+2,y2-2,grey2);
    VCvline(x2-4,y1+3,y2-3,grey1);

    VChline(x1+1,y1+1,x2-1,grey1);
    VChline(x1+2,y1+2,x2-2,grey2);
    VChline(x1+4,y1+3,x2-4,grey1);

    VChline(x1+1,y2-1,x2,grey1);
    VChline(x1+2,y2-2,x2-1,grey2);
    VChline(x1+4,y2-3,x2-3,grey1);
}

static VCBox() { /* -- ric: 21/Apr/98 -- */
    int i, x1, y1, x2, y2;
    x1=ResolveOperand();
    y1=ResolveOperand();
    x2=ResolveOperand();
    y2=ResolveOperand();

//  for (i=y1; i<=y2; i++)
//      VChline(x1,i,x2+1,bgcolor);
    VCColorField(x1,y1,x2+1,y2+1,&menuxlatbl);
    VCborder(x1, y1, x2, y2);
}

static VCAString(int x1, int y1, char *strng, int align)
/* -- ric: 30/Apr/98 -- */
{
    if (align==1) x1-=(strlen(strng)<<2);
    if (align==2) x1-=(strlen(strng)<<3);
    VCprintstring(x1,y1,strng);
}

static VCCharName() { /* -- ric: 21/Apr/98 -- */
    int x1,y1,i,align;

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    align=ResolveOperand();
    VCAString(x1,y1,pstats[i-1].name,align);
}

static VCItemName() { /* -- ric: 21/Apr/98 -- */
    int x1,y1,i,align;

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    align=ResolveOperand();
    VCAString(x1,y1,items[i].name,align);
}

static VCItemDesc() { /* -- ric: 21/Apr/98 -- */
    int x1,y1,i,align;

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    align=ResolveOperand();
    VCAString(x1,y1,items[i].desc,align);
}

static VCItemImage() { /* -- ric: 22/Apr/98 -- */
    int x1,y1,i,gf;
    unsigned char gsimg[512];
    char *img;

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    gf=ResolveOperand();
    img=itemicons+(items[i].icon<<8);
    if (gf) {
        grey(16,16,img,&gsimg);
        img=&gsimg;
    }

    VCtcopysprite(x1,y1,16,16,img);
}

static VCATextNum() { /* -- ric: 24/Apr/98 -- */
    int x1,y1,i,align;
    char stringbuf[100];
    char a[100]; // ANDY

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    align=ResolveOperand();
// ANDY
    if (i<0) {
        strcpy(&stringbuf,"-");
        dec_to_asciiz(-i,&a);
        strcat(&stringbuf,&a);
    }
    if (i>=0)
// END OF ANDY'S CHANGED STUFF
        dec_to_asciiz(i,&stringbuf);
    VCAString(x1,y1,&stringbuf,align);
}

static VCSpc() { /* -- ric: 24/Apr/98 -- */
    int x1,y1,i,gf;
    unsigned char gsimg[1024];
    char *img;

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    gf=ResolveOperand();
    img=speech+(1024*i);
    if (gf) {
        grey(32,32,img,&gsimg);
        img=&gsimg;
    }

    VCtcopysprite(x1,y1,32,32,img);
}

static CallEffect () { /* -- ric: 24/Apr/98 -- */
    char varcnt,i,*buf,*basebuf;
    unsigned short int t;
    int savetvar[26];

    varcnt=GrabC();
    t=ResolveOperand();

    for (i=0; i<varcnt; i++)
        varl[i]=ResolveOperand();

    buf=code;
    basebuf=basevc;
    memcpy(&savetvar,&tvar,sizeof(tvar));
    ExecuteEffect(t);
    memcpy(&tvar,&savetvar,sizeof(tvar));
    code=buf;
    basevc=basebuf;
}

ExecuteStartUpScript(unsigned short int s) { /* -- ric: 25/Apr/98 -- */
    basevc=startupvc;
    code=startupvc+startupofstbl[s];

    ExecuteBlock();
}

static CallScript () { /* -- ric: 25/Apr/98 -- */
    char varcnt,i,*buf,*basebuf;
    unsigned short int t;
    int savetvar[26];

    varcnt=GrabC();
    t=ResolveOperand();

    for (i=0; i<varcnt; i++)
        varl[i]=ResolveOperand();

    buf=code;
    basebuf=basevc;
    memcpy(&savetvar,&tvar,sizeof(tvar));

    ExecuteStartUpScript(t);

    memcpy(&tvar,&savetvar,sizeof(tvar));
    code=buf;
    basevc=basebuf;
}

static BindKey() {    /* -- ric: 03/May/98 -- */
    int ky, scrpt;
    ky=ResolveOperand();
    scrpt=ResolveOperand();
    key_map[ky].boundscript=scrpt;
}

static TextMenu() {   /* -- ric: 03/May/98 -- */
    char *buf1, *buf2;
    char *opt;
    int first=1,nv,p,ptr=0,ansave;
    int x1,y1,flagidx, width=0;

    ansave=an;
    an=1;

    x1=ResolveOperand();
    y1=ResolveOperand();
    flagidx=ResolveOperand();
    ptr=ResolveOperand();
    if (ptr) ptr--;

    nv=GrabC();
    buf1=code;
    for (p=0; p<nv; p++) {
        opt=code;
        GrabString(strbuf);
        if (width<strlen(opt)) width=strlen(opt);
    }
    buf2=code;

drawloop:
    drawmap();
    tmenubox(x1+16,y1+16,x1+66+(width*8),y1+28+(nv*10));
    for (p=0; p<nv; p++) {
        opt=code;
        GrabString(strbuf);
        gotoxy(x1+41,23+y1+(p*10));
        printstring(opt);
    }
    buf2=code;
    code=buf1;
    tcopysprite(23+x1,21+y1+(ptr*10),16,16,&menuptr);
    vgadump();
    readcontrols();
    if (first==2) if (b1 || b2 || b4) goto drawloop;
        else {
            an=ansave;
            code=buf2;
            flags[flagidx]=0;
            return;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) first=0;
    else if (first) goto drawloop;

    if (down) {
        ptr++;
        if (ptr==nv) ptr=0;
        playeffect(0);
        first=1;
    }
    if (up)   {
        if (!ptr) ptr=nv-1;
        else ptr--;
        playeffect(0);
        first=1;
    }

    if (b1) {
        an=ansave;
        code=buf2;
        flags[flagidx]=ptr+1;
        return;
    }

    while (!b4 && !b2) goto drawloop;
    while (b4 || b2) {
        first=2;
        goto drawloop;
    }

    an=ansave;
    code=buf2;
    flags[flagidx]=0;
    return;
}

static itemMenu() { /* -- ric: 03/May/98 -- */
    int c;
    c=ResolveOperand();
    ItemMenu(c-1);
}

static equipMenu() { /* -- ric: 03/May/98 -- */
    int c;
    c=ResolveOperand();
    EquipMenu(c-1);
}

static magicMenu() { /* -- ric: 03/May/98 -- */
    int c;
    c=ResolveOperand();
    MagicMenu(c-1);
}

static statusScreen() { /* -- ric: 03/May/98 -- */
    int c;
    c=ResolveOperand();
    StatusScreen(c);
}

static VCCr2() {     /* -- ric: 03/May/98 -- */
    int x1,y1,i,gf;
    unsigned char gsimg[9216];
    char *img;

    x1=ResolveOperand();
    y1=ResolveOperand();
    i=ResolveOperand();
    gf=ResolveOperand();
    img=chr2+(9216*(i-1));
    if (gf) {
        grey(96,96,img,&gsimg);
        img=&gsimg;
    }

    VCtcopysprite(x1,y1,96,96,img);
}

static VCTextBox() {   /* -- ric: 04/May/98 -- */
    char *buf1, *buf2;
    char *opt;
    int nv,p,ptr=0;
    int x1,y1,flagidx, width=0;

    x1=ResolveOperand();
    y1=ResolveOperand();
    ptr=ResolveOperand();

    nv=GrabC();
    buf1=code;
    for (p=0; p<nv; p++) {
        opt=code;
        GrabString(strbuf);
        if (width<strlen(opt)) width=strlen(opt);
    }
    buf2=code;
    code=buf1;

    VCColorField(x1,y1,x1+51+(width*8),y1+13+(nv*10),&menuxlatbl);
    VCborder(x1,y1,x1+50+(width*8),y1+12+(nv*10));

    for (p=0; p<nv; p++) {
        opt=code;
        GrabString(strbuf);
        VCprintstring(x1+25,y1+7+(p*10),opt);
    }
    if (ptr) VCtcopysprite(7+x1,5+y1+((ptr-1)*10),16,16,&menuptr);
}

static int ChooseChar(int x1, int y1)  /* -- ric: 25/Apr/98 -- */
/* Returns roster order of selected character or zero if cancelled */
{
    int first=1, ptr=0,j,ansave;
    unsigned char l;

    ansave=an;
    an=1;

drawloop:
    drawmap();
    tmenubox(x1+16,y1+16,x1+111,y1+28+(numchars*10));

    for (j=0; j<numchars; j++) {
        l=partyidx[j]-1;
        gotoxy(x1+41,y1+22+(j*10));
        printstring(pstats[l].name);
    }

    tcopysprite(x1+23,y1+20+(ptr*10),16,16,&menuptr);
    vgadump();

    readcontrols();
    if (first==2) if (b1 || b2 || b4) goto drawloop;
        else {
            an=ansave;
            return 0;
        }
    if (first && !b1 && !b2 && !b4 && !down && !up) first=0;
    else if (first) goto drawloop;

    if (down) {
        ptr++;
        if (ptr==numchars) ptr=0;
        playeffect(0);
        first=1;
    }
    if (up)   {
        if (!ptr) ptr=numchars-1;
        else ptr--;
        playeffect(0);
        first=1;
    }

    if (b1) {
        an=ansave;
        return ptr+1;
    }

    while (!b4 && !b2) goto drawloop;
    while (b4 || b2) {
        first=2;
        goto drawloop;
    }

    an=ansave;
    return 0;
}

