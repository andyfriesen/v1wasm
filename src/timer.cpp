// TIMER.C
// Copyright (C)1997 BJ Eirich
// Timer irq-hooking and PIT speed setting routines.

#include <stdio.h>

#include "engine.h" // for valloc()

#define PIT0 0x40
#define PIT1 0x41
#define PIT2 0x42
#define PITMODE 0x43
#define PITCONST 1193180L

#define OCR1    0x20
#define IMR1    0x21

#define OCR2    0xA0
#define IMR2    0xA1

unsigned hooktimer = 0;
unsigned char an = 0, tickctr = 0, sec = 0, min = 0, hr = 0;

void sethz(unsigned int hz) {
}

void restorehz() {
}

void timer_init() {
}

void timer_close() {
}

void delay(int ms) {
}

namespace {
    volatile unsigned int timer_count = 0;
    volatile int vc_timer;
}

int time() {
    return timer_count;
}

void setTimerCount(int offset) {
    timer_count = offset;
}

// Don't do anything fancy here: this function runs on a different thread.
// -- andy 21 August 2011
void incTimerCount() {
    timer_count++;
    vc_timer++;

    tickctr++;
    if (tickctr == 100) {
        tickctr = 0;
        sec++;

        if (sec == 60) {
            sec = 0;
            min++;
            if (min == 60) {
                min = 0;
                hr++;
            }
        }
    }

    // FIXME: HookTimer.  Can't run it here.
}

void decTimerCount() {
    timer_count--;
}

int getTimerCount() {
    return time();
}

int getVcTimer() {
    return vc_timer;
}

void setVcTimer(int value) {
    vc_timer = value;
}
