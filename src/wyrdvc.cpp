#include <string.h>

#include "control.h"
#include "engine.h"
#include "fs.h"
#include "menu.h"
#include "pcx.h"
#include "render.h"
#include "sound.h"
#include "timer.h"
#include "timer.h"
#include "vc.h"
#include "vclib.h"
#include "vga.h"

char charvar[32][33];           // string variables - Wyrdwad 7/26/98
FILE *vcinfile, *vcoutfile;     // infile and outfile - Wyrdwad 7/26/98
int intinflag;                  // IntIn flag -- Wyrdwad 7/31/98

extern char* strbuf;

void StringMenu()     /*-ric: 03/May/98, Wyrdwad: 30/July/98-*/
{ char *buf1, *buf2;
  char *opt;
  int first=1,p,ptr=0,ansave;
  int x1,y1,flagidx, width=0, fpos, lpos;

  ansave=an;
  an=1;

  x1=ResolveOperand();
  y1=ResolveOperand();
  flagidx=ResolveOperand();
  ptr=ResolveOperand();
  fpos = ResolveOperand();
  fpos--;
  lpos = ResolveOperand();
  lpos--;
  if (ptr) ptr--;
//  nv=GrabC();
  buf1=code;
  for (p=fpos; p<=lpos; p++) {
//    opt=code;
//    GrabString(charvar[p]);
    if (width<strlen(charvar[p])) width=strlen(charvar[p]);
  }
  buf2=code;

drawloopp:
  drawmap();
  tmenubox(x1+16,y1+16,x1+66+(width*8),y1+28+((lpos-fpos+1)*10));
  for (p=fpos; p<=lpos; p++) {
//    opt=code;
//    GrabString(charvar[p]);
    gotoxy(x1+41,23+y1+((p-fpos)*10));
    printstring(charvar[p]); //opt
  }
  buf2=code;
  code=buf1;
  tcopysprite(23+x1,21+y1+(ptr*10),16,16,menuptr);
  vgadump();
  readcontrols();
  if (first==2) if (b1 || b2 || b4) goto drawloopp;
     else { an=ansave; code=buf2; flags[flagidx]=0; return; }
  if (first && !b1 && !b2 && !b4 && !down && !up) first=0;
  else if (first) goto drawloopp;

  if (down) { ptr++;
              if (ptr==(lpos-fpos+1)) ptr=0;
              playeffect(0);
              first=1;
            }
  if (up)   { if (!ptr) ptr=lpos-fpos;
              else ptr--;
              playeffect(0);
              first=1;
            }

  if (b1) {
    an=ansave;
    code=buf2;
    flags[flagidx]=ptr+fpos+1;
    return;
  }

  while (!b4 && !b2) goto drawloopp;
  while (b4 || b2) { first=2; goto drawloopp; }

  an=ansave;
  code=buf2;
  flags[flagidx]=0;
  return;
}

void BStringMenu()     /*-ric: 03/May/98, Wyrdwad: 30/July/98-*/
{ char *buf1, *buf2;
  char *opt;
  int first=1,p,ptr=0,ansave;
  int x1,y1,flagidx, width=0, fpos, lpos;

  ansave=an;
  an=1;

  x1=ResolveOperand();
  y1=ResolveOperand();
  flagidx=ResolveOperand();
  ptr=ResolveOperand();
  fpos = ResolveOperand();
  fpos--;
  lpos = ResolveOperand();
  lpos--;
  if (ptr) ptr--;
//  nv=GrabC();
  buf1=code;
  for (p=fpos; p<=lpos; p++) {
//    opt=code;
//    GrabString(charvar[p]);
    if (width<strlen(charvar[p])) width=strlen(charvar[p]);
  }
  buf2=code;

drawloopp:
  drawmap();
//  tmenubox(x1+16,y1+16,x1+66+(width*8),y1+28+((lpos-fpos+1)*10));
  for (p=fpos; p<=lpos; p++) {
//    opt=code;
//    GrabString(charvar[p]);
    gotoxy(x1+41,23+y1+((p-fpos)*10));
    printstring(charvar[p]); //opt
  }
  buf2=code;
  code=buf1;
  tcopysprite(23+x1,21+y1+(ptr*10),16,16,menuptr);
  vgadump();
  readcontrols();
  if (first==2) if (b1 || b2 || b4) goto drawloopp;
     else { an=ansave; code=buf2; flags[flagidx]=0; return; }
  if (first && !b1 && !b2 && !b4 && !down && !up) first=0;
  else if (first) goto drawloopp;

  if (down) { ptr++;
              if (ptr==(lpos-fpos+1)) ptr=0;
              playeffect(0);
              first=1;
            }
  if (up)   { if (!ptr) ptr=lpos-fpos;
              else ptr--;
              playeffect(0);
              first=1;
            }

  if (b1) {
    an=ansave;
    code=buf2;
    flags[flagidx]=ptr+fpos+1;
    return;
  }

  while (!b4 && !b2) goto drawloopp;
  while (b4 || b2) { first=2; goto drawloopp; }

  an=ansave;
  code=buf2;
  flags[flagidx]=0;
  return;
}

void StrSText()                              /*-Wyrdwad 07/30/98-*/
{ char *str1, *str2, *str3;
  char st1[31], st2[31], st3[31];
  char portrait, first=1, line=1, chr=0;
  int cvarpos1, cvarpos2, cvarpos3;

  // Setup - Read stuff from the VC
  portrait=ResolveOperand();
  cvarpos1=ResolveOperand();
  cvarpos2=ResolveOperand();
  cvarpos3=ResolveOperand();
  cvarpos1--;
  cvarpos2--;
  cvarpos3--;
  str1 = charvar[cvarpos1];
  str2 = charvar[cvarpos2];
  str3 = charvar[cvarpos3];
  st1[0]=0; st2[0]=0; st3[0]=0;
  an=1;
  setTimerCount(0);

drawloop:
  while (time()!=0)
        {
            switch (line)
            {
               case 1: st1[chr]=str1[chr]; st1[chr+1]=0;
                       if (chr==strlen(str1)) { chr=0; line=2; }
                       else chr++; break;
               case 2: st2[chr]=str2[chr]; st2[chr+1]=0;
                       if (chr==strlen(str2)) { chr=0; line=3; }
                       else chr++; break;
               case 3: st3[chr]=str3[chr]; st3[chr+1]=0;
                       if (chr<strlen(str3)) chr++; break;
            }
            decTimerCount();
        }

  drawmap();
  textwindow(portrait, st1, st2, st3);
  vgadump();

  readcontrols();

  if (first==2) if (b1 || b2 || b4) goto drawloop;
     else { an=0; setTimerCount(0); return; }
  if (first && !b1 && !b2 && !b4) first=0;
  else if (first) goto drawloop;

  while (!b4 && !b2 && !b1) goto drawloop;
  while (b4 || b2 || b1) { first=2; goto drawloop; }
}

void StrText()                               /*-Wyrdwad 07/30/98-*/
{ char *str1, *str2, *str3,portrait,first=1;
  int cvarpos1, cvarpos2, cvarpos3;

  portrait=ResolveOperand();
  cvarpos1=ResolveOperand();
  cvarpos2=ResolveOperand();
  cvarpos3=ResolveOperand();
  cvarpos1--;
  cvarpos2--;
  cvarpos3--;
  str1 = charvar[cvarpos1];
  str2 = charvar[cvarpos2];
  str3 = charvar[cvarpos3];
  an=1;

drawloop:
  drawmap();
  textwindow(portrait,str1,str2,str3);
  vgadump();
  readcontrols();

  if (first==2) if (b1 || b2 || b4) goto drawloop;
     else { an=0; setTimerCount(0); return; }
  if (first && !b1 && !b2 && !b4) first=0;
  else if (first) goto drawloop;

  while (!b4 && !b2 && !b1) goto drawloop;
  while (b4 || b2 || b1) { first=2; goto drawloop; }
}

void StrPrompt()                             /*-Wyrdwad 07/30/98-*/
{ char *str1, *str2, *str3, *opt1, *opt2, portrait, first=1, selptr=0;
  unsigned short int flagidx;
  int cvarpos1, cvarpos2, cvarpos3, cvarpos4, cvarpos5;

  portrait=ResolveOperand();
  cvarpos1=ResolveOperand();
  cvarpos2=ResolveOperand();
  cvarpos3=ResolveOperand();
  flagidx=ResolveOperand();
  cvarpos4=ResolveOperand();
  cvarpos5=ResolveOperand();
  cvarpos1--;
  cvarpos2--;
  cvarpos3--;
  cvarpos4--;
  cvarpos5--;
  str1 = charvar[cvarpos1];
  str2 = charvar[cvarpos2];
  str3 = charvar[cvarpos3];
  opt1 = charvar[cvarpos4];
  opt2 = charvar[cvarpos5];
  an=1;

drawloop:
  drawmap();
  textwindow(portrait,str1,str2,str3);
  tmenubox(190,118,331,148);
  gotoxy(220,124); printstring(opt1);
  gotoxy(220,134); printstring(opt2);
  tcopysprite(200,122+(selptr*10),16,16,menuptr);
  vgadump();
  readcontrols();

  if (first==2) if (b1 || b2 || b4) goto drawloop;
     else { an=0; setTimerCount(0);
            flags[flagidx]=selptr; return; }
  if (first && !b1 && !b2 && !b4 && !down && !up) first=0;
  else if (first) goto drawloop;

  if (up || down) { selptr=selptr^1;
                    playeffect(0);
                    first=1; }

  while (!b4 && !b2 && !b1) goto drawloop;
  while (b4 || b2 || b1) { first=2; goto drawloop; }
}

void VCStringPCX()                           /*-Wyrdwad 07/26/98-*/
{ int x,y,i,cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  x=ResolveOperand();
  y=ResolveOperand();
  LoadPCXHeaderNP(charvar[cvarpos]);

  for (i=0; i<depth; i++)
  {
     ReadPCXLine(((i+y)*320)+x, vcscreen);
  }
  vclose(pcxf);
}

void VCLoadStrPCX()                          /*-Wyrdwad 07/26/98-*/
{ int ofs,i,cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  ofs=ResolveOperand();
  LoadPCXHeaderNP(charvar[cvarpos]);

  for (i=0; i<depth; i++)
  {
     ReadPCXLine((i*width)+ofs, reinterpret_cast<unsigned char*>(vcdatabuf));
  }
  vclose(pcxf);
}

void PlayMusStr()                            /*-Wyrdwad 07/26/98-*/
{ int cvarpos;
  cvarpos = ResolveOperand();
  cvarpos--;
  playsong(charvar[cvarpos]);
  setTimerCount(0);
}

void VCString()                              /*-Wyrdwad 07/26/98-*/
{ int x1,y1,cvarpos;

  x1=ResolveOperand();
  y1=ResolveOperand();
  cvarpos=ResolveOperand();
  cvarpos--;
  VCprintstring(x1,y1,charvar[cvarpos]);
}

void VCCenterString()                        /*-Wyrdwad 07/26/98-*/
{ int x1,y1,cvarpos;

  y1=ResolveOperand();
  cvarpos = ResolveOperand();
  cvarpos--;
  x1=160-(strlen(charvar[cvarpos])*4);
  VCprintstring(x1,y1,charvar[cvarpos]);
}

void AssignString()                          /*-Wyrdwad 07/26/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  GrabString(charvar[cvarpos]);
}

void CloseInfile()                           /*-Wyrdwad 07/26/98-*/
{
  intinflag = 0;
  fclose (vcinfile);
}

void CloseOutfile()                          /*-Wyrdwad 07/26/98-*/
{
  fclose (vcoutfile);
}

void CloseFiles ()                           /*-Wyrdwad 07/26/98-*/
{
  intinflag = 0;
  fclose (vcinfile);
  fclose (vcoutfile);
}

void OpenInfile ()                           /*-Wyrdwad 07/27/98-*/
{
  GrabString (strbuf);
  intinflag = 0;
  vcinfile = fopen (strbuf, "r");
}

void OpenOutfile ()                          /*-Wyrdwad 07/27/98-*/
{
  GrabString (strbuf);
  vcoutfile = fopen (strbuf, "w");
}

void OpenOutfAppend ()                       /*-Wyrdwad 07/27/98-*/
{
  GrabString (strbuf);
  vcoutfile = fopen (strbuf, "a");
}

void OpenInstring ()                         /*-Wyrdwad 07/27/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  intinflag = 0;
  cvarpos--;
  vcinfile = fopen (charvar[cvarpos], "r");
}

void OpenOutstring ()                        /*-Wyrdwad 07/27/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  vcoutfile = fopen (charvar[cvarpos], "w");
}

void OpenOutsAppend ()                       /*-Wyrdwad 07/27/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  vcoutfile = fopen (charvar[cvarpos], "a");
}

void IntIn ()                                /*-Wyrdwad 07/27/98-*/
{ int flagindex;
  int datain;

  flagindex = ResolveOperand();
  fscanf (vcinfile, "%d", &datain);
  flags[flagindex] = datain;
  intinflag = 1;
}

void IntOut ()                               /*-Wyrdwad 07/27/98-*/
{ int outbound;

  outbound = ResolveOperand();
  fprintf (vcoutfile, "%d\n", outbound);
}

void StrIn ()                                /*-Wyrdwad 07/30/98-*/
{ int cvarpos, i;
  char dummy[10];

  cvarpos = ResolveOperand();
  cvarpos--;
  if (intinflag) {
    fgets(dummy, 10, vcinfile);
    intinflag = 0;
  }
  fgets (charvar[cvarpos], 32, vcinfile);
  i = 0;
  while (charvar[cvarpos][i] > 31) {
    i++;
  }
  charvar[cvarpos][i] = 0;
}

void WordIn ()                               /*-Wyrdwad 07/31/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  fscanf (vcinfile, "%s", charvar[cvarpos]);
}

/*
void FilenameIn ()
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  fscanf (vcinfile, "%s", charvar[cvarpos]);
}
*/

void WordOut ()                              /*-Wyrdwad 07/31/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  fprintf (vcoutfile, "%s ", charvar[cvarpos]);
}

void StrOut ()                               /*-Wyrdwad 07/27/98-*/
{ int cvarpos;

  cvarpos = ResolveOperand();
  cvarpos--;
  fprintf (vcoutfile, "%s\n", charvar[cvarpos]);
}

void FileTextOut ()                          /*-Wyrdwad 07/27/98-*/
{
  GrabString (strbuf);
  fprintf (vcoutfile, "%s\n", strbuf);
}

void StringBanner()                          /*-Wyrdwad 07/30/98-*/
{ char first=1;
  int duration, cvarpos;

  //str=code;
  an=1;
  cvarpos = ResolveOperand();
  cvarpos--;
  duration=ResolveOperand()*91;
  setTimerCount(0);
drawlooppp:
  drawmap();
  tmenubox(106,106,246,126);
  gotoxy(176-(strlen(charvar[cvarpos])*4),112);
  printstring(charvar[cvarpos]);
  vgadump();
  readcontrols();

  if (first==2) if (b1 || b2 || b4) goto drawlooppp;
     else { an=0; setTimerCount(0); return; }
  if (first && !b1 && !b2 && !b4 && !down && !up) first=0;
  else if (first) goto drawlooppp;


  while (!b4 && !b2 && !b1 && time()<duration) goto drawlooppp;
  while (b4 || b2 || b1) { first=2; goto drawlooppp; }
  setTimerCount(0); an=0;
}

void CopyString()                            /*-Wyrdwad 07/30/98-*/
{ int cvarpos1, cvarpos2;

  cvarpos1 = ResolveOperand();
  cvarpos2 = ResolveOperand();
  cvarpos1--;
  cvarpos2--;
  strcpy (charvar[cvarpos1], charvar[cvarpos2]);

/*  while (charvar[strpos][cvarpos2] != NULL) {
    charvar[strpos][cvarpos1] = charvar[strpos][cvarpos2];
    strpos++;
  }
  charvar[strpos][cvarpos1] = 0; */
}

void CompareString()                         /*-Wyrdwad 07/30/98-*/
{ int cvarpos1, cvarpos2, flagidx, strres;

  cvarpos1 = ResolveOperand();
  cvarpos2 = ResolveOperand();
  flagidx = ResolveOperand();
  cvarpos1--;
  cvarpos2--;
  strres = strcmp (charvar[cvarpos1], charvar[cvarpos2]);
  if (strres == 0) { flags[flagidx] = 1; } else { flags[flagidx] = 0; }

/*  while ((charvar[strpos][cvarpos1]) && (charvar[strpos][cvarpos2])) {
    if (charvar[strpos][cvarpos1] != charvar[strpos][cvarpos2]) {
      flags[flagidx] = 0;
    }
  }
  if (charvar[strpos][cvarpos1] != charvar[strpos][cvarpos2]) {
    flags[flagidx] = 0;
  }*/
}

void Concatenate()                           /*-Wyrdwad 07/30/98-*/
{ int cvarpos1, cvarpos2;

  cvarpos1 = ResolveOperand();
  cvarpos2 = ResolveOperand();
  cvarpos1--;
  cvarpos2--;
  if ((strlen(charvar[cvarpos1]) + strlen(charvar[cvarpos2])) > 30) {
    return;             //combined string size too big!
  }
  strcat (charvar[cvarpos1], charvar[cvarpos2]);

/*  while (charvar[strpos][cvarpos1]) {
    strpos++;
  }
  strpos2 = 0;
  while ((charvar[strpos2][cvarpos2]) && (strpos < 31)) {
    charvar[strpos][cvarpos1] = charvar[strpos2][cvarpos2];
    strpos++;
    strpos2++;
  }
  charvar[strpos][cvarpos1] = 0;*/
}

void StrChangeCHR()                          /*-Wyrdwad 08/02/98-*/
{ int l, cvarpos;
  char *img;
  FILE *f;

  l=ResolveOperand();
  cvarpos = ResolveOperand();
  strcpy (pstats[l-1].chrfile, charvar[cvarpos-1]);
  img=reinterpret_cast<char*>(chrs) + (CharPos(l) * 15360);
  f=fopen(pstats[l-1].chrfile,"rb");
  fread(img,1,15360,f);
  fclose(f);
}
