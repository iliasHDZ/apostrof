#include "timer.h"

#include "../vga.h"
#include "../int/isr.h"
#include "../io.h"
#include "../task/task.h"

u32 timer_counter = 0;

static void timer_callback(isr_regs* r) {
    timer_counter++;
}

void timer_init() {
    isr_register(IRQ0, timer_callback);

    u32 divisor = 1193180 / 1000;
    u8  low     = (u8)(divisor & 0xFF);
    u8  high    = (u8)( (divisor >> 8) & 0xFF);

    outb(0x43, 0x36);
    outb(0x40, low);
    outb(0x40, high);
}

void timer_sleep(u32 ms) {
    timer_counter = 0;

    asm("sti");
    while (timer_counter < ms) asm("hlt");
}