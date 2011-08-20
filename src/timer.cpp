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

unsigned int timer_count = 0, timer = 0, hooktimer = 0;
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

int time() {
    static int t = 0;
    return ++t;
}

namespace {
    int timer_count_offset = 0;
}

void setTimerCount(int offset) {
    timer_count_offset = offset;
}

int getTimerCount() {
    return time() - timer_count_offset;
}
