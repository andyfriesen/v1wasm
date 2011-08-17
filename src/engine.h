#pragma once

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
             delayct, adjactv;     // yet more internal crap
    unsigned short x1, y1, x2, y2;       // bounding box coordinates
    unsigned char curcmd, cmdarg;        // Script commands/arguments
    char* scriptofs;            // offset in script parsing
    unsigned char face, chasing,         // face player when activated | chasing
             chasespeed, chasedist; // chasing variables
    unsigned short cx, cy;               // current-tile pos (moving adjusted)
    int expand1;                         // always room for improvement
    char entitydesc[20];                 // Editing description
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

    // NEW: MAGIC VARIABLES

    unsigned char magcnt;                // number of items in inventory
    unsigned short int maginv[24];          // inventory

};

struct itemdesc {
    char name[20];              // item name
    unsigned short int icon;             // icon index
    char desc[40];              // item description
    unsigned char useflag;               // Useable flag <see below>
    unsigned short int useeffect;        // effect list index
    unsigned char itemtype;              // item type index
    unsigned char equipflag;             // 0=nonequipable 1=equipable
    unsigned char equipidx;              // equip.dat index value
    unsigned char itmprv;                // item preview code
    unsigned int price;                  // Cost of the item in a store
};

// NEW MAGIC

struct magicdesc {
    char name[20];              // magic name
    unsigned short int icon;             // icon index
    char desc[40];              // magic description
    unsigned char useflag;               // Useable flag <see below>
    unsigned short int useeffect;        // magic effect list index
    unsigned char itemtype;              // magic type index
    unsigned char equipflag;             // 0=nonequipable 1=equipable
    unsigned char equipidx;              // magiceq.dat index value
    unsigned char itmprv;                // magic preview code
    unsigned int cost;                   // Cost of the magic in a store
    unsigned int price;                  // Cost of the magic to cast
};

// END NEW

struct equipstruc {
    int str, end, mag, mgr, hit, dod;
    int mbl, fer, rea;
    char equipable[30];
    char onequip, ondeequip;
};

struct vspanim {
    unsigned short int start;            // start tile index
    unsigned short int finish;           // end tile index
    unsigned short int delay;            // delay between cycles
    unsigned short int mode;
};          // animation mode

extern struct vspanim va0[100];
extern char mapname[13], vsp0name[13];
extern char musname[13];
extern char numchars, tchars;
extern char lastmoves[6];
extern char usenxy;
extern struct r_entity party[101];
extern struct charstats pstats[30];
extern struct itemdesc items[255];

// NEW MAGIC

struct mequipstruc {
    char equipable[30];
    char level[30];
    char onequip, ondeequip;
};

extern struct magicdesc magic[255];
extern unsigned char* magicicons;
extern struct mequipstruc mequip[255];

// END NEW

extern struct equipstruc equip[255];
extern unsigned short int nx, ny;
extern short int flags[16000];
extern short int numtiles;


extern char partyidx[5];
extern unsigned char* itemicons, *chrs, *chr2;
extern int gp, xwin, ywin, xtc, ytc, xofs, yofs;
extern unsigned short int* map0, *map1, xsize, ysize, vadelay[100];
extern unsigned char* mapp, layerc, pmultx, pdivx, pmulty, pdivy, saveflag;
extern unsigned char* vsp0, autoent;

void* valloc(int amount, const char* whatfor);
void vfree(void* thismem);
void ProcessControls();
void ExecuteScript(unsigned short);
void UpdateEquipStats();
void startfollow();
void lastmove(char n);

int ObstructionAt(int tx, int ty);

void addcharacter(int i);

void startmap(char* fname);
void load_map(char* fname);

void check_tileanimation();
void process_entities();

void SaveGame(char* fn);

void allocbuffers();
