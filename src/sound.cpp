// sound.c
// RPG <-> MikMod interfacing code
// Copyright (C)1997 BJ Eirich

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "keyboard.h"
#include "engine.h"
#include "control.h"
//#include "mikmod.h"
#include "sound.h"
#include "timer.h"
#include "render.h"

char numfx;
char curch;
char jf;
char waitvrt, speed = 0, moneycheat = 0;
int vspm;
//int mapvcm; // -- aen; 31/May/98 -- no longer used; see LoadVC().
int vcbufm;
//int mapm; // -- aen; 30/May/98 -- no longer used; see load_map().

char playing = 0;
char playingsong[13];

extern char kb1, kb2, kb3, kb4;
extern char jb1, jb2, jb3, jb4;

unsigned char mp_volume;
signed short mp_sngpos;

void tickhandler() {
}

int ParseSetup() {
    FILE* s;

    if (!(s = fopen("SETUP.CFG", "r"))) {
        printf("Could not open SETUP.CFG - Using defaults.\n");
        delay(2000);
        return 0;
    }

parseloop:
    if ((keyboard_map[SCAN_ALT]) && (keyboard_map[SCAN_CTRL]) &&
            (keyboard_map[SCAN_DEL])) {
        err("Exiting: CTRL-ALT-DEL pressed.");
    }

    fscanf(s, "%s", strbuf);
    if (!strcmp(strbuf, "waitvrt")) {
        fscanf(s, "%s", strbuf);
        waitvrt = atoi(strbuf);
        goto parseloop;
    }
    //  if (!strcmp(strbuf,"vsp"))    // -- aen; 31/May/98 -- no longer used;
    //     { fscanf(s,"%s",strbuf);   // -- see load_map().
    //       vspm=atoi(strbuf);
    //       goto parseloop; }
    //  if (!strcmp(strbuf,"map"))    // -- aen; 30/May/98 -- no longer used;
    //     { fscanf(s,"%s",strbuf);   // -- see load_map().
    //       mapm=atoi(strbuf);
    //       goto parseloop; }
    //  if (!strcmp(strbuf,"mapvc"))  // -- aen; 31/May/98 -- no longer used;
    //     { fscanf(s,"%s",strbuf);   // -- see LoadVC().
    //       mapvcm=atoi(strbuf);
    //       goto parseloop; }
    if (!strcmp(strbuf, "vcbuf")) {
        fscanf(s, "%s", strbuf);
        vcbufm = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "speeddemon")) {
        speed = 1;
        goto parseloop;
    }
    if (!strcmp(strbuf, "meseta")) {
        moneycheat = 1;
        goto parseloop;
    }
    if (!strcmp(strbuf, "vspspeed")) {
        vspspeed = 1;
        goto parseloop;
    }

    if (!strcmp(strbuf, "joystick")) {
        fscanf(s, "%s", strbuf);
        jf = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb1")) {
        fscanf(s, "%u", &kb1);
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb2")) {
        fscanf(s, "%u", &kb2);
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb3")) {
        fscanf(s, "%u", &kb3);
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb4")) {
        fscanf(s, "%u", &kb4);
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb1")) {
        fscanf(s, "%u", &jb1);
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb2")) {
        fscanf(s, "%u", &jb2);
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb3")) {
        fscanf(s, "%u", &jb3);
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb4")) {
        fscanf(s, "%u", &jb4);
        goto parseloop;
    }

    if (!strcmp(strbuf, "sounddevice")) {
        fscanf(s, "%s", strbuf);
        //md_device = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "mixrate")) {
        fscanf(s, "%s", strbuf);
        //md_mixfreq = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "dmabufsize")) {
        fscanf(s, "%s", strbuf);
        //md_dmabufsize = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "forcemono")) {
        //md_mode &= ~DMODE_STEREO;
        goto parseloop;
    }
    if (!strcmp(strbuf, "force8bit")) {
        //md_mode &= ~DMODE_16BITS;
        goto parseloop;
    }
    if (!strcmp(strbuf, "interpolate")) {
        //md_mode |= DMODE_INTERP;
        goto parseloop;
    }
    if (!strcmp(strbuf, "volume")) {
        fscanf(s, "%s", strbuf);
        //mp_volume = atoi(strbuf);
        goto parseloop;
    }

    fclose(s);
}

int lastvol;

void sound_init()
// Incidently, sound_init() also initializes the control interface, since
// they both use SETUP.CFG.
{
    char t;

    vspm = 256000;
    //mapvcm=50000; // -- aen; 31/May/09 -- no longer used; see LoadVC().
    vcbufm = 250000;
    //mapm=150000; // -- aen; 30/May/09 -- no longer used; see load_map().

    kb1 = 28;
    kb2 = 56;
    kb3 = 1;
    kb4 = 57;
    jb1 = 1;
    jb2 = 2;
    jb3 = 3;
    jb4 = 4;
    waitvrt = 0;
    jf = 0;
    vspspeed = 0;

    ParseSetup();
    allocbuffers();
    initcontrols(jf);
    playingsong[0] = 0;
}

void sound_loadsfx(char* fname) {
}

void sound_freesfx() {
}

void playsong(char* sngnme) {
}

void stopsound() {
}

void playeffect(char efc) {
}

/*playsound(char *fname,int rate)
{ char chanl;

  chanl=md_numchn-curch;
  if (curch==1) curch=2; else curch=1;

  sfx1=MW_LoadWavFN(fname);
  MD_VoiceSetVolume(chanl,64);
  MD_VoiceSetPanning(chanl,128);
  MD_VoiceSetFrequency(chanl,rate);
  MD_VoicePlay(chanl,sfx1->handle,0,sfx1->length,0,0,sfx1->flags);
}*/
