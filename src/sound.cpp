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
#include "ppapi/cpp/instance.h"
#include "sound.h"
#include "timer.h"
#include "render.h"
#include "fs.h"
#include "nacl.h"

using namespace verge;

char numfx = 0;
char curch = 0;
char jf = 0;
char waitvrt;
char speed = 0;
char moneycheat = 0;
int vspm = 0;
int vcbufm = 0;

unsigned char mp_volume = 100;
signed short mp_sngpos = 0;

void ParseSetup() {
    char strbuf[256];

    auto s = vopen("SETUP.CFG", "r");

    if (!s) {
        printf("Could not open SETUP.CFG - Using defaults.\n");
        delay(2000);
        return;
    }

parseloop:
    if ((keyboard_map[SCAN_ALT]) && (keyboard_map[SCAN_CTRL]) &&
            (keyboard_map[SCAN_DEL])) {
        err("Exiting: CTRL-ALT-DEL pressed.");
    }

    vscanf(s, "%s", strbuf);
    if (!strcmp(strbuf, "waitvrt")) {
        vscanf(s, "%s", strbuf);
        waitvrt = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "vcbuf")) {
        vscanf(s, "%s", strbuf);
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
        vscanf(s, "%s", strbuf);
        jf = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb1")) {
        int q;
        vscanf(s, "%u", &q);
        kb1 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb2")) {
        int q;
        vscanf(s, "%u", &q);
        kb2 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb3")) {
        int q;
        vscanf(s, "%u", &q);
        kb3 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "kb4")) {
        int q;
        vscanf(s, "%u", &q);
        kb4 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb1")) {
        int q;
        vscanf(s, "%u", &q);
        jb1 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb2")) {
        int q;
        vscanf(s, "%u", &q);
        jb2 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb3")) {
        int q;
        vscanf(s, "%u", &q);
        jb3 = q;
        goto parseloop;
    }
    if (!strcmp(strbuf, "jb4")) {
        int q;
        vscanf(s, "%u", &q);
        jb4 = q;
        goto parseloop;
    }

    if (!strcmp(strbuf, "sounddevice")) {
        vscanf(s, "%s", strbuf);
        //md_device = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "mixrate")) {
        vscanf(s, "%s", strbuf);
        //md_mixfreq = atoi(strbuf);
        goto parseloop;
    }
    if (!strcmp(strbuf, "dmabufsize")) {
        vscanf(s, "%s", strbuf);
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
        vscanf(s, "%s", strbuf);
        //mp_volume = atoi(strbuf);
        goto parseloop;
    }

    vclose(s);
}

int lastvol;

void sound_loadsfx(char* fname) {
    VFILE* f = vopen(fname, "r");
    if (!f) {
        err("Could not open sound effect index file.");
    }

    int numfx = 0;
    vscanf(f, "%i", &numfx);

    for (auto i = 0; i < numfx; ++i) {
        char filename[255];
        vscanf(f, "%s", filename);
        verge::plugin->loadSound(filename);
    }

    vclose(f);
}

// Incidently, sound_init() also initializes the control interface, since
// they both use SETUP.CFG.
void sound_init() {
    vspm = 256000;
    vcbufm = 250000;

    waitvrt = 0;
    jf = 0;
    vspspeed = 0;

    ParseSetup();
    allocbuffers();
    initcontrols(jf);

    sound_loadsfx("MAIN.SFX");
}

void sound_freesfx() {
    // blaaaah whatever
}


namespace {
    std::string playingSong;
}

void playsong(const std::string& songName) {
    if (songName != playingSong) {
        playingSong = songName;
        verge::plugin->playSong(songName);
    }
}

void stopsound() {
    verge::plugin->stopSound();
}

void playeffect(int efc) {
    verge::plugin->playEffect(efc);
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
