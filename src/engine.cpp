// engine.c
// All core engine code
// Copyright (C)1997 BJ Eirich

#include <stdio.h>
#include <stdlib.h>
#include "control.h"
#include "entity.h"
#include "keyboard.h"
#include "menu.h"
#include "render.h"
#include "sound.h"
#include "timer.h"
#include "vga.h"
extern void err(char *ermsg);

// ============================ Data ============================

struct r_entity {                      // on-screen entities (chars)
    unsigned short x;                    // xwc position
    unsigned short y;                    // ywc position
    unsigned char facing;                // direction entity is facing
    unsigned char moving;                // direction entity is moving
    unsigned char movcnt;                // how far left to move in this tile
    unsigned char framectr;              // frame sequence counter
    unsigned char specframe;             // special-frame set thingo
    unsigned char chrindex, movecode;    // CHR index / movement pattern code
    unsigned char activmode, obsmode;    // activation mode, obstruction mode
    unsigned int actscript, movescript;  // script references
    unsigned char speed, speedct;        // entity speed, speedcount :)
    unsigned short step, delay,          // Misc data entries
             data1, data2,         // More misc data
             data3, data4,         // yet more crappy misc data.
             delayct,adjactv;      // yet more internal crap
    unsigned short x1,y1,x2,y2;          // bounding box coordinates
    unsigned char curcmd, cmdarg;        // Script commands/arguments
    unsigned char *scriptofs;            // offset in script parsing
    unsigned char face,chasing,          // face player when activated | chasing
             chasespeed, chasedist; // chasing variables
    unsigned short cx,cy;                // current-tile pos (moving adjusted)
    int expand1;                         // always room for improvement
    char entitydesc[20];                 // Editing description
};

struct vspanim {
    unsigned short int start;            // start tile index
    unsigned short int finish;           // end tile index
    unsigned short int delay;            // delay between cycles
    unsigned short int mode;
};          // animation mode

struct zoneinfo {                      // zone data
    char zonename[15];                   // zone description
    unsigned short int callevent;        // event number to call
    unsigned char percent;               // chance (in 255) of event occurance
    unsigned char delay;                 // step-delay before last occurance
    unsigned char aaa;                   // accept adjacent activation
    char savedesc[30];                   // savegame description
};

struct charstats {                     // Stat record for single character
    char name[9];                        // 8-character name + null
    char chrfile[13];                    // CHR file for this character
    int exp, rea;                        // experience points, REAction
    int curhp, maxhp;                    // current/max hit points
    int curmp, maxmp;                    // current/max magic points
    int atk, def;                        // ATtacK and DEFense stats
    int str, end;                        // STRength and ENDurance
    int mag, mgr;                        // MAGic and MaGic Resistance stats
    int hit, dod;                        // HIT% and DODge%
    int mbl, fer;                        // MoBiLity and FERocity (not related)
    int magc, mgrc;                      // MAGic and MaGic Resistance stats
    int hitc, dodc;                      // HIT% and DODge%
    int mblc, ferc;                      // MoBiLity and FERocity (not related)
    int reac;                            // reaction (all ***c stats are current)
    int nxt, lv;                         // exp to next level, current level
    char status;                         // 0=fine 1=dead 2=...
    unsigned char invcnt;                // number of items in inventory
    unsigned short int inv[24];          // inventory

    // *** NEW *** : Magic
    unsigned char magcnt;                // number of items in inventory
    unsigned short int maginv[24];          // inventory

};

struct itemdesc {
    unsigned char name[20];              // item name
    unsigned short int icon;             // icon index
    unsigned char desc[40];              // item description
    unsigned char useflag;               // Useable flag <see below>
    unsigned short int useeffect;        // effect list index
    unsigned char itemtype;              // item type index
    unsigned char equipflag;             // 0=nonequipable 1=equipable
    unsigned char equipidx;              // equip.dat index value
    unsigned char itmprv;                // item preview code
    unsigned int price;                  // Cost of the item in a store
};

// *** NEW *** MAGIC

struct magicdesc {
    unsigned char name[20];              // magic name
    unsigned short int icon;             // icon index
    unsigned char desc[40];              // magic description
    unsigned char useflag;               // Useable flag <see below>
    unsigned short int useeffect;        // magic effect list index
    unsigned char itemtype;              // magic type index
    unsigned char equipflag;             // 0=nonequipable 1=equipable
    unsigned char equipidx;              // magiceq.dat index value
    unsigned char itmprv;                // magic preview code
    unsigned int price;                  // Cost of the magic in a store
};

// END NEW

struct equipstruc {
    int str,end,mag,mgr,hit,dod;
    int mbl,fer,rea;
    char equipable[30];
    char onequip, ondeequip;
};

// Useflag values:   0 for nonusable.
//                   1 for usable once, then it goes away.
//                   2 for usable an infinite number of times.
//                   3 for usable ONLY camped, only once.
//                   4 for usable ONLY camped, infinitely
//                   5 for usable in combat only, once, then it goes away.
//                   6 for usable in combat only, indefinetly.

// Itemtype values:  0 for OS. (Own Self)
//                   1 for 1A. (One Ally)
//                   2 for AA. (All Allies)
//                   3 for 1E. (One Enemy)
//                   4 for AE. (All Enemy)

char lastmoves[6];                     // the party leader's last six moves
char partyidx[5];                      // current character-ref indexes
char numchars=0;                       // number of characters to draw
struct vspanim va0[100];               // tile animation data
struct r_entity party[101];            // party entities
struct zoneinfo zone[128];             // entity records
struct charstats pstats[30];           // party-stats record
struct itemdesc items[255];            // item definition array

// *** NEW *** MAGIC

struct mequipstruc {
    char equipable[30];
    char level[30];
    char onequip, ondeequip;
};

struct magicdesc magic[255];
unsigned char *magicicons;        // more graphic ptrs
struct mequipstruc mequip[255];          // equipment definition array

// END NEW

#include <string.h>

struct equipstruc equip[255];          // equipment definition array
char zonedelay,lz;                     // zone delay counter
char tchars;                           // total characters in PARTY.DAT
int gp;                                // party's gold (or meseta, s, $, etc)

unsigned short int vadelay[100];       // delay counter

unsigned short int *map0=NULL;        // map data pointers
unsigned short int *map1=NULL;
unsigned char *mapp=NULL;

unsigned char *vsp0,*chrs;       // graphic data pointers
unsigned char *itemicons,*chr2;        // more graphic ptrs
signed int flags[8000];              // misc flags

FILE *map,*vsp;                        // file handles
char mapname[13], musname[13];         // map/music filenames
char vsp0name[13], levelname[30];      // vsp filename / level name
char layerc,showname,saveflag;         // control flags

/* -- ric: 21/Apr/98 - moved to process_controls -- */
// char l,r,t,b;                          // wall status

int xwin, ywin;                        // window coordinates
int xtc,ytc,xofs,yofs;                 // variables for drawmap()
unsigned short int xsize,ysize;        // x/y map dimensions
unsigned short int startx,starty;      // default start location
unsigned char pmultx,pdivx;            // parallax speed X
unsigned char pmulty,pdivy;            // parallax speed Y
char warp, hide;                       // map warpable / hidable flags

char dontshowname=0,usenxy=0,qabort=0,autoent=0;
unsigned short int nx=0,ny=0,numtiles;

extern char *strbuf,*speech,*msbuf,menuactive;
extern unsigned char nummovescripts;
//extern int mapvcm; // -- aen; 31/May/98 -- no longer used; see LoadVC().
extern int msofstbl[100];
extern int vspm; // -- aen; 30/May/98 -- no longer used; see load_map().
//extern int mapm; // -- aen; 30/May/98 -- no longer used; see load_map().

void LoadVC(FILE* map);
void CalcVSPMask();
void ExecuteScript(int);
void ExecuteStartUpScript(unsigned short s);

// ============================ Code ============================

void *valloc(int amount, const char* whatfor) {
    /* -- ric: 10/May/98 --
     * Well actually this is aen's code to replace malloc. It'll help
     * with debugging :)
     */
    /* -- aen: 10/May-98--
     * Well actually this is vec's idea. I just extended it a little.
     */
    char *tmp;
    static char debug_buf[256];
    tmp = (char*)malloc(amount);
    if (!tmp) {
        sprintf(debug_buf,"Failed allocating %i bytes for %s.", amount, whatfor);
        err(debug_buf);
    }
    memset(tmp, 0, amount); // -- aen: 30/May/98; clear allocated mem
    return tmp;
}

void vfree(void *thismem) {
    if (thismem)
        free(thismem);
    thismem = NULL;
}

void InitVCMem();

void allocbuffers() {
    //map0 = valloc(mapm*2, "map0"); // -- aen; 30/May/98 -- removed; this is
    //map1 = valloc(mapm*2, "map1"); // -- taken care of automatically within
    //mapp = valloc(mapm, "mapp");   // -- load_map().

    //vsp0 = valloc(vspm, "vsp0");
    chrs = (unsigned char*)valloc(512000, "chrs");
    chr2 = (unsigned char*)valloc(46080, "chr2");

    // -- NichG; ??/??/?? --
    magicicons = (unsigned char*)valloc(50688, "magicicons");

    itemicons = (unsigned char*)valloc(50688, "itemicons");
    speech = (char*)valloc(450000, "speech");
    msbuf = (char*)valloc(20000, "msbuf");

    //memset(map0,0,mapm*2);     /* -- ric:28/Apr/98 --                 */
    //memset(map1,0,mapm*2);     /* Clear all the allocated memory just */
    //memset(mapp,0,mapm);       /* in case.                            */
    //memset(vsp0,0,vspm);
    //memset(chrs,0,512000);     // -- aen; 30/May/98 -- removed; this is
    //memset(chr2,0,46080);      // -- automatically taken care of by valloc()
    //memset(speech,0,450000);   // -- now.
    //memset(msbuf,0,20000);
    //memset(flags,0,32000);
    //memset(itemicons,0,50688);
    //memset(&party, 0, sizeof party);

    InitVCMem();
}

void addcharacter(int i) {
    FILE *chrf,*p;
    int b;

    numchars++;
    partyidx[numchars-1]=i;
    p=fopen("PARTY.DAT","r");
    if (!p) err("Fatal error: PARTY.DAT not found");
    fscanf(p,"%s",strbuf);
    tchars=atoi(strbuf);
    if (i>atoi(strbuf)) err("addcharacter(): index out of range");
    for (b=1; b<i; b++) {
        fscanf(p,"%s",strbuf);
        fscanf(p,"%s",strbuf);
        fscanf(p,"%s",strbuf);
    }
    fscanf(p,"%s",strbuf);

    party[0].chrindex=0;

    party[1].x=party[0].x;
    party[1].y=party[0].y;
    party[1].cx=party[0].cx;
    party[1].cy=party[0].cy;
    party[1].facing=0;
    party[1].moving=0;
    party[1].chrindex=1;

    party[2].x=party[0].x;
    party[2].y=party[0].y;
    party[2].cx=party[0].cx;
    party[2].cy=party[0].cy;
    party[2].facing=0;
    party[2].moving=0;
    party[2].chrindex=2;

    party[3].x=party[0].x;
    party[3].y=party[0].y;
    party[3].cx=party[0].cx;
    party[3].cy=party[0].cy;
    party[3].facing=0;
    party[3].moving=0;
    party[3].chrindex=3;

    party[4].x=party[0].x;
    party[4].y=party[0].y;
    party[4].cx=party[0].cx;
    party[4].cy=party[0].cy;
    party[4].facing=0;
    party[4].moving=0;
    party[4].chrindex=4;

    lastmoves[0]=0;
    lastmoves[1]=0;
    lastmoves[2]=0;
    lastmoves[3]=0;
    lastmoves[4]=0;
    lastmoves[5]=0;
    chrf=fopen(pstats[i-1].chrfile,"rb");
    if (!chrf) err("addcharacter(): CHR file not found");
    fread(chrs+((numchars-1)*15360), 1, 15360, chrf);
    fclose(chrf);

    fscanf(p,"%s",strbuf);
    fclose(p);
    chrf=fopen(strbuf,"rb");
    if (!chrf) err("addcharacter(): CR2 file not found");
    fread(chr2+((numchars-1)*9216), 1, 9216, chrf);
    fclose(chrf);
}

void LoadCHRList() {
    int i;
    FILE *f;

    for (i=0; i<20; i++)
        if (strlen(chrlist[i].fname)) {
            f=fopen(chrlist[i].fname,"rb");
            fread(chrs+((i+5)*15360),30,512,f);
            fclose(f);
        }
}

void load_map(char *fname) {
    unsigned char b;
    int i;

    memcpy(mapname, fname, 13);

    map = fopen(fname,"rb");
    if (!map)
        err("Could not open specified MAP file.");

    fread(&b, 1, 1, map);
    if (b != 4)
        err("*error* Incorrect MAP version.");

    fread(vsp0name, 1, 13, map);
    fread(musname, 1, 13, map);

    fread(&layerc, 1, 1, map);
    fread(&pmultx, 1, 1, map);
    pmulty=pmultx;
    fread(&pdivx, 1, 1, map);
    pdivy=pdivx;

    fread(levelname, 1, 30, map);

    fread(&showname, 1, 1, map);
    fread(&saveflag, 1, 1, map);

    fread(&startx, 1, 2, map);
    fread(&starty, 1, 2, map);

    fread(&hide, 1, 1, map);
    fread(&warp, 1, 1, map);

    fread(&xsize, 1, 2, map);
    fread(&ysize, 1, 2, map);

    fread(&b, 1, 1, map);
    if (b)
        err("*error* MAP compression not yet supported.");

    fread(map0, 1, 27, map); // skip buffer

    // -- aen; 30/May/98 -- dynamic map mem allocation
    // free previously allocate layer/info map mem (if necessary)
    vfree(map0);
    vfree(map1);
    vfree(mapp);
    // allocate exact amount of mem needed
    map0 = (unsigned short*)valloc((xsize * ysize) * 2, "load_map:map0");
    map1 = (unsigned short*)valloc((xsize * ysize) * 2, "load_map:map1");
    mapp = (unsigned char*)valloc((xsize * ysize), "load_map:mapp");

    fread(map0, xsize, ysize*2, map); // read in background layer
    fread(map1, xsize, ysize*2, map); // read in foreground layer
    fread(mapp, xsize, ysize, map);   // read in zone info/obstruction layer

    fread(&zone, 1, sizeof(zone), map);
    fread(&chrlist, 13, 100, map);      // read in chr list

    LoadCHRList();

    fread(&entities, 1, 4, map);
    fread(&party[5], 88, entities, map);

    for (i=5; i<entities+5; i++) {
        party[i].cx = party[i].x;
        party[i].cy = party[i].y;
        party[i].x = party[i].x * 16;
        party[i].y = party[i].y * 16;
    }

    fread(&nummovescripts, 1, 1, map);
    fread(&i, 1, 4, map);
    fread(&msofstbl, 4, nummovescripts, map);
    fread(msbuf, 1, i, map);

    entities += 5;

    LoadVC(map);
    fclose(map);

    if (strlen(musname)) playsong(musname);

    // load the .VSP file
    vsp = fopen(vsp0name,"rb");
    if (!vsp)
        err("Could not open specified VSP file.");
    fseek(vsp, 2, 0);
    fread(pal, 1, 768, vsp);
    fread(&numtiles, 1, 2, vsp);

    // -- aen; 31/May/98 -- dynamic map mem allocation
    vfree(vsp0);
    vspm = numtiles << 8;
    vsp0 = (unsigned char*)valloc(vspm, "load_map:vsp0");

    fread(vsp0, 1, vspm, vsp);
    fread(&va0, 1, sizeof(va0), vsp);
    fclose(vsp);

    for (i=0; i<2048; i++)
        tileidx[i]=i;

    if (vspspeed)
        CalcVSPMask();

    if (usenxy) {
        startx=nx;
        starty=ny;
    }

    party[0].x=startx*16;
    party[0].y=starty*16;
    party[0].moving=0;
    party[0].cx=startx;
    party[0].cy=starty;

    party[1].x=party[0].x;
    party[1].y=party[0].y;
    party[1].moving=0;
    party[1].cx=startx;
    party[1].cy=starty;

    party[2].x=party[0].x;
    party[2].y=party[0].y;
    party[2].moving=0;
    party[2].cx=startx;
    party[2].cy=starty;

    party[3].x=party[0].x;
    party[3].y=party[0].y;
    party[3].moving=0;
    party[3].cx=startx;
    party[3].cy=starty;

    party[4].x=party[0].x;
    party[4].y=party[0].y;
    party[4].moving=0;
    party[4].cx=startx;
    party[4].cy=starty;

    lastmoves[0]=0;
    lastmoves[1]=0;
    lastmoves[2]=0;
    lastmoves[3]=0;
    lastmoves[4]=0;
    lastmoves[5]=0;

    ExecuteScript(0);
}

void process_stepzone() {
    // Called each time the player steps in a new square. Looks up current zone
    // information, adjusts for delays and then makes a chance roll to see if
    // an event is called.

    unsigned char cz, t1;

    cz=mapp[(((party[0].y/16)*xsize)+(party[0].x/16))] >> 1;
    if (lz!=cz) {
        zonedelay=0;
        lz=cz;
    }
    if (!zone[cz].percent) return;
    if (zonedelay<zone[cz].delay) {
        zonedelay++;
        return;
    }

    t1=(rand()*255);
    if (t1<=zone[cz].percent) {
        ExecuteScript(zone[cz].callevent);
        zonedelay=0;
    }
}

void lastmove(char n) {
    // moves all the entrees in the lastmoves array down one, then places the
    // new direction at the front.

    lastmoves[5]=lastmoves[4];
    lastmoves[4]=lastmoves[3];
    lastmoves[3]=lastmoves[2];
    lastmoves[2]=lastmoves[1];
    lastmoves[1]=lastmoves[0];
    lastmoves[0]=n;
}

void startfollow() {
    char i;
    for (i=1; i<numchars; i++) {
        if (lastmoves[i]==1) {
            party[i].y++;
            party[i].facing=0;
            party[i].cy++;
            party[i].moving=1;
            party[i].movcnt=15;
        }
        if (lastmoves[i]==2) {
            party[i].y--;
            party[i].facing=1;
            party[i].cy--;
            party[i].moving=2;
            party[i].movcnt=15;
        }
        if (lastmoves[i]==3) {
            party[i].x++;
            party[i].facing=2;
            party[i].cx++;
            party[i].moving=3;
            party[i].movcnt=15;
        }
        if (lastmoves[i]==4) {
            party[i].x--;
            party[i].facing=3;
            party[i].cx--;
            party[i].moving=4;
            party[i].movcnt=15;
        }
    }
}

int InvFace() {
    switch(party[0].facing) {
    case 0:
        return 1;
    case 1:
        return 0;
    case 2:
        return 3;
    case 3:
        return 2;
    }
}

void Activate() {
    int tx,ty;
    unsigned char cz,t;

    // First, determine what tile we're looking at.
    tx=party[0].x/16;
    ty=party[0].y/16;
    switch (party[0].facing) {
    case 0: {
        ty++;
        break;
    }
    case 1: {
        ty--;
        break;
    }
    case 2: {
        tx++;
        break;
    }
    case 3: {
        tx--;
        break;
    }
    }

    // Here is where we first check for entities in our way.

    t=EntityAt(tx,ty);
    if (t)
        if (!party[t].activmode && party[t].actscript) {
            if (party[t].face) party[t].facing=InvFace();
            ExecuteScript(party[t].actscript);
            return;
        }

    // Check adjacent zones.
    cz=mapp[((ty*xsize)+tx)] >> 1;
    if (zone[cz].aaa) {
        ExecuteScript(zone[cz].callevent);
        return;
    }
}

int ObstructionAt(int tx,int ty) {
    if ((mapp[((ty)*xsize)+tx] & 1)==1) return 1;
    return 0;
}

void process_entities() {
    int i;

    for (i=5; i<entities; i++)
        ProcessSpeedAdjEntity(i);

    if (autoent)
        for (i=95; i<95+numchars; i++)
            ProcessSpeedAdjEntity(i);
}

void process_controls();

void ProcessControls() {
    if (party[0].speed<4) {
        switch (party[0].speed) {
        case 1:
            if (party[0].speedct<3) {
                party[0].speedct++;
                return;
            }
        case 2:
            if (party[0].speedct<2) {
                party[0].speedct++;
                return;
            }
        case 3:
            if (party[0].speedct<1) {
                party[0].speedct++;
                return;
            }
        }
    }
    if (party[0].speed<5) process_controls();
    switch (party[0].speed) {
    case 5:
        process_controls();
        process_controls();
        return;
    case 6:
        process_controls();
        process_controls();
        process_controls();
        return;
    case 7:
        process_controls();
        process_controls();
        process_controls();
        process_controls();
        return;
    }
}

void process_controls() {
    unsigned char i;
    char l,r,t,b;  /* -- ric: 21/Apr/98 - moved from top -- */
    party[0].speedct=0;
    if (!party[0].moving) {
        xtc=party[0].x/16;
        ytc=party[0].y/16;
        if ((mapp[((ytc+1)*xsize)+xtc] & 1)==1) b=0;
        else b=1;
        if ((mapp[((ytc-1)*xsize)+xtc] & 1)==1) t=0;
        else t=1;
        if ((mapp[(ytc*xsize)+((party[0].x+17)/16)] & 1)==1) r=0;
        else r=1;
        if ((mapp[(ytc*xsize)+((party[0].x-15)/16)] & 1)==1) l=0;
        else l=1;

        if (xtc==0) l=0;
        if (ytc==0) t=0;
        if (xtc==xsize-1) r=0; /* -- ric: 28/Apr/98 -- */
        if (ytc==ysize-1) b=0; /* -- ric: 28/Apr/98 -- */

        readcontrols();
        if (b1) Activate();
        if (b3) SystemMenu();
        if (b4) MainMenu();

        for(i=0; i<128; i++) { /* -- ric: 03/May/98 -- */
            if ((key_map[i].pressed) && (key_map[i].boundscript)) {
//         dec_to_asciiz(i,strbuf);
//         err(strbuf);
                ExecuteStartUpScript(key_map[i].boundscript);
            }
        }

        xtc=party[0].x/16;
        ytc=party[0].y/16;

        if (right) party[0].facing=2;
        if (down) party[0].facing=0;
        if (left) party[0].facing=3;
        if (up) party[0].facing=1;

        if ((right) && (r) && !EntityAt(xtc+1,ytc)) {
            party[0].x++;
            party[0].facing=2;
            party[0].moving=3;
            party[0].cx++;
            party[0].movcnt=15;
            lastmove(3);
            startfollow();
            return;
        }
        if ((down) && (b) && !EntityAt(xtc,ytc+1)) {
            party[0].y++;
            party[0].facing=0;
            party[0].moving=1;
            party[0].cy++;
            party[0].movcnt=15;
            lastmove(1);
            startfollow();
            return;
        }
        if ((left) && (l) && !EntityAt(xtc-1,ytc)) {
            party[0].x--;
            party[0].facing=3;
            party[0].moving=4;
            party[0].cx--;
            party[0].movcnt=15;
            lastmove(4);
            startfollow();
            return;
        }
        if ((up) && (t) && !EntityAt(xtc,ytc-1)) {
            party[0].y--;
            party[0].facing=1;
            party[0].moving=2;
            party[0].cy--;
            party[0].movcnt=15;
            lastmove(2);
            startfollow();
            return;
        }

        party[0].framectr=0;
        party[1].framectr=0;
        party[2].framectr=0;
        party[3].framectr=0;
        party[4].framectr=0;
    }

    if (party[0].moving) {
        for (i=0; i<numchars; i++) {
            if (party[i].moving==1) {
                party[i].y++;
                party[i].movcnt--;
                party[i].framectr++;
            }
            if (party[i].moving==2) {
                party[i].y--;
                party[i].movcnt--;
                party[i].framectr++;
            }
            if (party[i].moving==3) {
                party[i].x++;
                party[i].movcnt--;
                party[i].framectr++;
            }
            if (party[i].moving==4) {
                party[i].x--;
                party[i].movcnt--;
                party[i].framectr++;
            }
            if (party[i].framectr==80) party[i].framectr=0;
        }
        if (!party[0].movcnt) {
            party[0].moving=0;
            process_stepzone();
        }
    }
}

void check_tileanimation() {
    unsigned char i;

    for (i=0; i<100; i++) {
        if ((va0[i].delay) && (va0[i].delay<vadelay[i]))
            animate(i);
        vadelay[i]++;
    }
}

void UpdateEquipStats() {
    int i,j,a;

    // This function takes the base stats of all characters and re-calculates
    // current-stats by re-applying equipment modifiers.

    for (i=0; i<30; i++) {
        pstats[i].atk=pstats[i].str;
        pstats[i].def=pstats[i].end;
        pstats[i].magc=pstats[i].mag;
        pstats[i].mgrc=pstats[i].mgr;
        pstats[i].hitc=pstats[i].hit;
        pstats[i].dodc=pstats[i].dod;
        pstats[i].mblc=pstats[i].mbl;
        pstats[i].ferc=pstats[i].fer;
        pstats[i].reac=pstats[i].rea;

        for (j=0; j<6; j++) {
            a=items[pstats[i].inv[j]].equipidx;
            pstats[i].atk+=equip[a].str;
            pstats[i].def+=equip[a].end;
            pstats[i].magc+=equip[a].mag;
            pstats[i].mgrc+=equip[a].mgr;
            pstats[i].hitc+=equip[a].hit;
            pstats[i].dodc+=equip[a].dod;
            pstats[i].mblc+=equip[a].mbl;
            pstats[i].ferc+=equip[a].fer;
            pstats[i].reac+=equip[a].rea;
        }
    }
}

void game_ai() {                     // this is THE main loop
    ProcessControls();
    process_entities();
    if (keyboard_map[SCAN_CTRL] && speed) ProcessControls();
    if (moneycheat) gp=100000;
    check_tileanimation();
}

void CreateSaveImage(unsigned char *buf) {
    memcpy(buf,chrs,512);
    if (numchars>1) memcpy(buf+512,chrs+15360,512);
    else memset(buf+512,0,512);
    if (numchars>2) memcpy(buf+1024,chrs+30720,512);
    else memset(buf+1024,0,512);
    if (numchars>3) memcpy(buf+1536,chrs+46080,512);
    else memset(buf+1536,0,512);
    if (numchars>4) memcpy(buf+2048,chrs+61440,512);
    else memset(buf+2048,0,512);
}

void SaveGame(char *fn) {
    FILE *f;
    unsigned char cz;
    unsigned char temp[2560];

    f=fopen(fn,"wb");
    cz=mapp[(((party[0].y/16)*xsize)+(party[0].x/16))] >> 1;
    fwrite(&zone[cz].savedesc, 1, 30, f);
    cz=partyidx[0]-1;
    fwrite(&pstats[cz].name, 1, 9, f);
    fwrite(&pstats[cz].lv, 1, 4, f);
    fwrite(&pstats[cz].curhp, 1, 8, f);
    fwrite(&gp, 1, 4, f);
    fwrite(&hr, 1, 1, f);
    fwrite(&min, 1, 1, f);
    fwrite(&sec, 1, 1, f);
    fwrite(&numchars, 1, 1, f);
    fwrite(&menuactive, 1, 1, f);
    CreateSaveImage((unsigned char*)&temp);
    fwrite(&temp, 1, 2560, f);
    fwrite(&mapname, 1, 13, f);
    fwrite(&party, 1, sizeof party, f);
    fwrite(&partyidx, 1, 5, f);
    fwrite(&flags, 1, 32000, f);
    fwrite(&tchars, 1, 1, f);
    fwrite(&pstats, 1, sizeof pstats, f);
    fclose(f);
}

void startmap(char *fname) {
    if (qabort) return;

    set_intensity(0);
    load_map(fname);
    drawmap();
    vgadump();
    fin();

    timer_count=0;
    lz=0;
    tickctr=0;
    zonedelay=0;

main_loop:
    while (timer_count!=0) {
        timer_count--;
        game_ai();
    }

    drawmap();
    vgadump();

    while (!timer_count) {
        gp--;
        gp++;
    }

    if (!qabort) goto main_loop;
}
