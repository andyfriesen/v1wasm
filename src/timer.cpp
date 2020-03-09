// TIMER.C
// Copyright (C)1997 BJ Eirich
// Timer irq-hooking and PIT speed setting routines.

#include <stdio.h>

#include <emscripten.h>

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

EM_JS(void, wasm_initTimer, (unsigned int* count), {
    function incr() {
        HEAP32[count >> 2]++;
    }
    window.vergeTimer = setInterval(incr, 10);
});

EM_JS(void, wasm_closeTimer, (), {
    cancelTimer(window.vergeTimer);
    window.vergeTimer = null;
});

namespace {
    unsigned int timer_count = 0;
    int vc_timer;
}

int timer_init() {
    wasm_initTimer(&timer_count);
    return 0;
}

int timer_close() {
    wasm_closeTimer();
    return 0;
}

void delay(int ms) {
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
    return timer_count;
}

int getVcTimer() {
    return vc_timer;
}

void setVcTimer(int value) {
    vc_timer = value;
}
