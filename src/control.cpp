// CONTROL.C
// Handles keyboard/joystick interfaces into a unified system.
// Copyright (C)1997 BJ Eirich

#include "keyboard.h"

/* -- ric: 03/May/98 -- */
struct keyb_map {
  char      pressed;                  // keyboard flags
  short int boundscript; };           // bound script

struct keyb_map key_map[128];         // for recording bound keys

char j;                               // use joystick or not

char b1,b2,b3,b4;                     // four button flags for GamePad
char up,down,left,right;              // stick position flags
int jx,jy;                            // joystick x / y values

char foundx,foundy;                   // found-flags for joystick read.
int cenx, ceny;                       // stick-center values
int upb,downb,leftb,rightb;           // barriers for axis determination

// --- Control masks
char kb1, kb2, kb3, kb4;              // keyboard definable controls.
char jb1, jb2, jb3, jb4;              // joystick definable controls.

initcontrols(char joystk)
{ int i;
  j=joystk;
  if (j)
     if (!calibrate()) j=0;
  for (i=0;i<128;i++) key_map[i].boundscript=0;    // no keys are bound yet
}

readb()
{
  if (j) readbuttons();
  else { b1=0; b2=0; b3=0; b4=0; }
  if (keyboard_map[kb1]) b1=1;
  if (keyboard_map[kb2]) b2=1;
  if (keyboard_map[kb3]) b3=1;
  if (keyboard_map[kb4]) b4=1;

  if ((keyboard_map[SCAN_ALT]) &&
     (keyboard_map[SCAN_X]))
        err("Exiting: ALT-X pressed.");

  if (keyboard_map[SCAN_F10])
     { keyboard_map[SCAN_F10]=0;
       ScreenShot(); }
}

readcontrols()
{ int i;
  if (j) readjoystick();
  else { b1=0; b2=0; b3=0; b4=0;
         up=0; down=0; left=0; right=0; }

  if (keyboard_map[SCAN_UP]) up=1;
  if (keyboard_map[SCAN_DOWN]) down=1;
  if (keyboard_map[SCAN_LEFT]) left=1;
  if (keyboard_map[SCAN_RIGHT]) right=1;
  if (keyboard_map[kb1]) b1=1;
  if (keyboard_map[kb2]) b2=1;
  if (keyboard_map[kb3]) b3=1;
  if (keyboard_map[kb4]) b4=1;

  for (i=0;i<128;i++) {                          /* -- ric: 03/May/98 -- */
    key_map[i].pressed=0;
    if (keyboard_map[i]) key_map[i].pressed=1;   // no keys are bound yet
  }

  if ((keyboard_map[SCAN_ALT]) &&
     (keyboard_map[SCAN_X]))
        err("Exiting: ALT-X pressed.");

  if (keyboard_map[SCAN_F10])
     { keyboard_map[SCAN_F10]=0;
       ScreenShot(); }
}

readbuttons()
{ unsigned char b;
  char btbl[4];

  b=inportb(0x201);                   // poll joystick port
  b=b >> 4;                           // lose high nibble
  b=b ^ 15;                           // flip mask bits

  btbl[0]=b & 1;                           // mask button status
  btbl[1]=b & 2;
  btbl[2]=b & 4;
  btbl[3]=b & 8;

  if (btbl[jb1-1]) b1=1; else b1=0;
  if (btbl[jb2-1]) b2=1; else b2=0;
  if (btbl[jb3-1]) b3=1; else b3=0;
  if (btbl[jb4-1]) b4=1; else b4=0;

}

getcoordinates()
// Gets raw, machine dependant coordinates from the joystick.
{
  foundx=0;
  foundy=0;

  asm("cli                            \n\t"  // disable interrupts
      "movw $513, %%dx                \n\t"  // start joystick timer
      "outb %%al, %%dx                \n\t"
      "xorl %%ecx, %%ecx              \n\t"  // clear out counter
      "movl $2, %%ebx                 \n\t"  // number of axii left to report
"joyloop:                             \n\t"
      "incl %%ecx                     \n\t"  // increment counter
      "cmpl $65500, %%ecx             \n\t"  // time out?
      "je j_end                       \n\t"
      "inb %%dx, %%al                 \n\t"  // poll joystick status
      "cmpb $1, _foundx               \n\t"
      "je search_y                    \n\t"

      "test $1, %%al                  \n\t"  // is this axis in?
      "jnz j0                         \n\t"
      "movl %%ecx, _jx                \n\t"  // if so, store coordinate
      "movl $1, _foundx               \n\t"  // say we already got it
      "decl %%ebx                     \n\t"  // one less axis to go.
      "jz j_end                       \n\t"

"j0:                                  \n\t"
      "cmp $1, _foundy                \n\t"
      "je joyloop                     \n\t"
"search_y:                            \n\t"
      "test $2, %%al                  \n\t"  // is the Y axis in?
      "jnz joyloop                    \n\t"
      "movl %%ecx, _jy                \n\t"  // if so, handle it.
      "movl $1, _foundy               \n\t"
      "decl %%ebx                     \n\t"
      "jz j_end                       \n\t"  // are we done?
      "jmp joyloop                    \n\t"

"j_end:                               \n\t"
      "sti                            \n\t"  // turn interrupts back on
      :
      :
      : "eax","ebx","ecx","edx","cc" );
}

int calibrate()
{ // assumes the stick is centered when called.

  getcoordinates();                  // read stick position
  if ((!foundx) || (!foundy))
     { printf("Could not detect joystick. Disabling.\n");
       return 0; }

  cenx=jx;
  ceny=jy;
  upb=(ceny*75)/100;                 // 25% dead zone
  leftb=(cenx*75)/100;
  rightb=(cenx*125)/100;             // 25% dead zone
  downb=(ceny*125)/100;
  return 1;
}

readjoystick()
{
  readbuttons();
  getcoordinates();
  up=0; down=0;
  left=0; right=0;

  if (jx<leftb) left=1;
  if (jx>rightb) right=1;
  if (jy<upb) up=1;
  if (jy>downb) down=1;
}

