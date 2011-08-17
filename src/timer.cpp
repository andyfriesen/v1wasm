// TIMER.C
// Copyright (C)1997 BJ Eirich
// Timer irq-hooking and PIT speed setting routines.

#include <stdio.h>
#include <dos.h>
#include <sys/nearptr.h>
#include <go32.h>
#include <dpmi.h>
#include <crt0.h>
#include <pc.h>

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

int _crt0_startup_flags = _CRT0_FLAG_NEARPTR;
typedef __dpmi_paddr *PVI;
static PVI oldhandler;
unsigned int timer_count=0,timer=0,hooktimer=0;
unsigned char an=0,tickctr=0,sec=0,min=0,hr=0;
extern char playing;

PVI DJSetHandlerFunc(unsigned char irqno, void (*handler)(), int len) {
    PVI oldvect = (PVI) valloc(sizeof(__dpmi_paddr),"DJSetHandlerFunc:oldvect");
    int vecno=(irqno>7) ? irqno+0x68 : irqno+0x8;
    _go32_dpmi_seginfo wrapper;
    __dpmi_paddr new;

    wrapper.pm_offset = (long int) handler;
    wrapper.pm_selector = _my_cs();
    _go32_dpmi_allocate_iret_wrapper(&wrapper);
    new.offset32 = wrapper.pm_offset;
    new.selector = wrapper.pm_selector;
    __dpmi_get_and_disable_virtual_interrupt_state();
    if (len) _go32_dpmi_lock_code(handler,len);
    _go32_dpmi_lock_data(&wrapper,sizeof(_go32_dpmi_seginfo));
    __dpmi_get_protected_mode_interrupt_vector(vecno,oldvect);
    __dpmi_set_protected_mode_interrupt_vector(vecno,&new);
    __dpmi_get_and_enable_virtual_interrupt_state();
    return oldvect;
}

void DJSetHandlerAddr(unsigned char irqno, PVI handler) {
    int vecno=(irqno>7) ? irqno+0x68 : irqno+0x8;
    _go32_dpmi_seginfo wrapper;
    __dpmi_paddr oldhandler;

    __dpmi_get_and_disable_virtual_interrupt_state();
    __dpmi_get_protected_mode_interrupt_vector(vecno, &oldhandler);
    wrapper.pm_offset = oldhandler.offset32;
    wrapper.pm_selector = oldhandler.selector;
    _go32_dpmi_free_iret_wrapper(&wrapper);
    __dpmi_set_protected_mode_interrupt_vector(vecno,handler);
    __dpmi_get_and_enable_virtual_interrupt_state();
    free(handler);
}

static SendEOI (unsigned char irqno) {
    unsigned char ocr=(irqno>7) ? OCR2 : OCR1;
    unsigned char eoi=0x60|(irqno&7);

    outportb(ocr,eoi);
    if (irqno>7) outportb(OCR1,0x20);
}

static newhandler(void) {
    timer_count++;
    timer++;
    if (playing) MD_Update();
    if (an) check_tileanimation();
    {
        tickctr++;
        if (tickctr == 100) {
            tickctr=0;
            sec++;
        }
        if (sec == 60) {
            min++;
            sec=0;
        }
        if (min == 60) {
            hr++;
            min=0;
        }
    }
    if (hooktimer) ExecuteHookedScript(hooktimer);
    SendEOI(0);
}

static void EndNewHandler() { }

sethz(unsigned int hz) {
    unsigned int pit0_set, pit0_value;

    disable();

    outportb(PITMODE, 0x34);
    pit0_value=PITCONST / hz;
    pit0_set=(pit0_value & 0x00ff);
    outportb(PIT0, pit0_set);
    pit0_set=(pit0_value >> 8);
    outportb(PIT0, pit0_set);

    enable();
}

restorehz() {
    disable();
    outportb(PITMODE, 0x34);
    outportb(PIT0, 0x00);
    outportb(PIT0, 0x00);
    enable();
}

timer_init() {
    oldhandler = DJSetHandlerFunc(0, newhandler,
                                  ((int) EndNewHandler) - ((int) newhandler));
    sethz(100);
}

timer_close() {
    DJSetHandlerAddr(0, oldhandler);
    restorehz();
}
