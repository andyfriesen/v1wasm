// #define numfuncs 90
// #define numvars0 68
// #define numvars1 43
// #define numvars2 6

/* -- Added by Ric -- */
/* Added functions:
 *       VCBox(x1,y1,x2,y2);                           (21/Apr/98)
 *       VCCharName(x, y, party.dat index, align);     (21/Apr/98)
 *       VCItemName(x, y, items.dat index, align);     (21/Apr/98)
 *       VCItemDesc(x, y, items.dat index, align);     (21/Apr/98)
 *       VCItemImage(x, y, items.dat index, greyflag); (22/Apr/98)
 *       VCATextNum(x, y, number, align);              (24/Apr/98)
 *       VCSpc(x, y, speech portrait, greyflag);       (24/Apr/98)
 *       CallEffect(event number, option var1...);     (24/Apr/98)
 *       CallScript(event number, option var1...);     (25/Apr/98)
 *       VCLine(x1, y1, x2, y2, color);                (??/???/??):NichG
 *       GetMagic (character, spell);                  (??/???/??):NichG
 *       BindKey(key code, script);                    (03/May/98)
 *       TextMenu(x,y,flag,ptr,"opt1","opt2",..);      (04/May/98)
 *       ItemMenu(roster order index);                 (03/May/98)
 *       EquipMenu(roster order index);                (03/May/98)
 *       MagicMenu(roster order index);                (03/May/98)
 *       StatusScreen(roster order index);             (03/May/98)
 *       VCCr2(x, y, roster order index, greyflag);    (03/May/98)
 *       VCSpellName(x, y, magic.dat index, center);   (??/???/??):NichG
 *       VCSpellDesc(x, y, magic.dat index, center);   (??/???/??):NichG
 *       VCSpellImage(x, y, magic.dat index, greyflag);(??/???/??):NichG
 *       MagicShop(spell1, spell2, spell3, ... spell12); (??/??/??):NichG
 *       VCTextBox(x,y,ptr,"opt1","opt2",..);          (04/May/98)
 *       PlayVAS("filename",speed);                    (04/May/98):NichG
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
 */
//#define numfuncs 104
//#define numvars0 70
//#define numvars1 57
//#define numvars2 6
/* -- -- */

/* -- Added by xBig_D == */
/*
   Added functions:
//         VCLayerWrite(layer) 05/may/98
   Added var0:
         VCLayer2(RW)  05/may/98
         VCLayer2trans(RW)  05/may/98
         VCLayerWrite(RW)   05/may/98

         ModPosition(RW)
 */
#define numfuncs 103
#define numvars0 74
#define numvars1 58
#define numvars2 7
/* -- -- */


char *funcs[]=
{ "MAPSWITCH","WARP","ADDCHARACTER","SOUNDEFFECT","GIVEITEM","TEXT",
  "ALTERFTILE","ALTERBTILE","FAKEBATTLE","RETURN","PLAYMUSIC","STOPMUSIC",
  "HEALALL","ALTERPARALLAX","FADEIN","FADEOUT","REMOVECHARACTER","BANNER",
  "ENFORCEANIMATION","WAITKEYUP","DESTROYITEM","PROMPT","CHAINEVENT",
  "CALLEVENT","HEAL","EARTHQUAKE","SAVEMENU","ENABLESAVE","DISABLESAVE",
  "REVIVECHAR","RESTOREMP","REDRAW","STEXT","DISABLEMENU","ENABLEMENU","WAIT",
  "SETFACE","MAPPALETTEGRADIENT","BOXFADEOUT","BOXFADEIN","GIVEGP","TAKEGP",
  "CHANGEZONE","GETITEM","FORCEEQUIP","GIVEXP","SHOP","PALETTEMORPH",
  "CHANGECHR","READCONTROLS","VCPUTPCX","HOOKTIMER","HOOKRETRACE",
  "VCLOADPCX","VCBLITIMAGE","PLAYFLI","VCCLEAR","VCCLEARREGION","VCTEXT",
  "VCTBLITIMAGE","EXIT","QUIT","VCCENTERTEXT","RESETTIMER","VCBLITTILE",
  "SYS_CLEARSCREEN","SYS_DISPLAYPCX","OLDSTARTUPMENU","VGADUMP","NEWGAME",
  "LOADMENU","DELAY","PARTYMOVE","ENTITYMOVE","AUTOON","AUTOOFF",
  "ENTITYMOVESCRIPT","VCTEXTNUM","VCLOADRAW","VCBOX","VCCHARNAME",
  "VCITEMNAME","VCITEMDESC","VCITEMIMAGE","VCATEXTNUM","VCSPC","CALLEFFECT",
  "CALLSCRIPT", "VCLINE", "GETMAGIC","BINDKEY","TEXTMENU","ITEMMENU",
  "EQUIPMENU","MAGICMENU","STATUSSCREEN","VCCR2", "VCSPELLNAME", "VCSPELLDESC",
  "VCSPELLIMAGE", "MAGICSHOP","VCTEXTBOX","PLAYVAS"};
  // 103

char *vars0[]=
{ "A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S",
  "T","U","V","W","X","Y","Z","NUMCHARS","GP","LOCX","LOCY","TIMER",
  "DRAWPARTY","CAMERATRACKING","XWIN","YWIN","B1","B2","B3","B4","UP","DOWN",
  "LEFT","RIGHT","TIMERANIMATE","FADE","LAYER0","LAYER1","LAYERVC","QUAKEX",
  "QUAKEY","QUAKE","SCREENGRADIENT","PMULTX","PMULTY","PDIVX","PDIVY","VOLUME",
  "PARALLAXC","CANCELFADE","DRAWENTITIES","CURZONE","TILEB","TILEF",
  "FOREGROUNDLOCK","XWIN1","YWIN1","LAYER1TRANS","LAYERVCTRANS","FONTCOLOR",
  "KEEPAZ","LAYERVC2","LAYERVC2TRANS", "VCLAYERWRITE", "MODPOSITION" };

char *vars1[]=
{ "FLAGS","FACING","CHAR","ITEM","VAR","PARTYINDEX","XP","CURHP","MAXHP",
  "CURMP","MAXMP","KEY","VCDATABUF","SPECIALFRAME","FACE","SPEED",
  "ENTITY.MOVING","ENTITY.CHRINDEX","MOVECODE","ACTIVEMODE","OBSMODE",
  "ENTITY.STEP","ENTITY.DELAY","ENTITY.LOCX","ENTITY.LOCY","ENTITY.X1",
  "ENTITY.X2","ENTITY.Y1","ENTITY.Y2","ENTITY.FACE","ENTITY.CHASING",
  "ENTITY.CHASEDIST","ENTITY.CHASESPEED","ENTITY.SCRIPTOFS","ATK","DEF","HIT",
  "DOD","MAG","MGR","REA","MBL","FER","ITEM.USE","ITEM.EFFECT","ITEM.TYPE",
  "ITEM.EQUIPTYPE","ITEM.EQUIPINDEX","ITEM.PRICE", "SPELL.USE", "SPELL.EFFECT",
  "SPELL.TYPE", "SPELL.PRICE", "SPELL.COST","LVL","NXT","CHARSTATUS", "SPELL"};

char *vars2[]=
{ "RANDOM","SCREEN","ITEMS","CANEQUIP","CHOOSECHAR", "OBS", "SPELLS" };

char write0[]=
{ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1 }; // 74

char write1[]=
{ 1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // 58

char write2[]=
{ 0,1,1,0,0,1,0 }; // um.. 7 :)
