extern "C" {
#include "isr.h"
#include "idt.h"

#include "../vga.h"
#include "../io.h"

#include "../error.h"
}

#include "../task/task.hpp"

isr_callback isr_handlers[256];

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10

void isr_init() {
    idt_setGate(0,  (u32)isr0);
    idt_setGate(1,  (u32)isr1);
    idt_setGate(2,  (u32)isr2);
    idt_setGate(3,  (u32)isr3);
    idt_setGate(4,  (u32)isr4);
    idt_setGate(5,  (u32)isr5);
    idt_setGate(6,  (u32)isr6);
    idt_setGate(7,  (u32)isr7);
    idt_setGate(8,  (u32)isr8);
    idt_setGate(9,  (u32)isr9);
    idt_setGate(10, (u32)isr10);
    idt_setGate(11, (u32)isr11);
    idt_setGate(12, (u32)isr12);
    idt_setGate(13, (u32)isr13);
    idt_setGate(14, (u32)isr14);
    idt_setGate(15, (u32)isr15);
    idt_setGate(16, (u32)isr16);
    idt_setGate(17, (u32)isr17);
    idt_setGate(18, (u32)isr18);
    idt_setGate(19, (u32)isr19);
    idt_setGate(20, (u32)isr20);
    idt_setGate(21, (u32)isr21);
    idt_setGate(22, (u32)isr22);
    idt_setGate(23, (u32)isr23);
    idt_setGate(24, (u32)isr24);
    idt_setGate(25, (u32)isr25);
    idt_setGate(26, (u32)isr26);
    idt_setGate(27, (u32)isr27);
    idt_setGate(28, (u32)isr28);
    idt_setGate(29, (u32)isr29);
    idt_setGate(30, (u32)isr30);
    idt_setGate(31, (u32)isr31);

    //outb(0x0020, 0x11);
    //outb(0x00A0, 0x11);
    //outb(0x0021, 32);
    //outb(0x00A1, 40);
    //outb(0x0021, 0x04);
    //outb(0x00A1, 0x02);
    //outb(0x0021, 0x01);
    //outb(0x00A1, 0x01);
    //outb(0x0021, 0x0);
    //outb(0x00A1, 0x0);

    unsigned char a1, a2;
 
	a1 = inb(PIC1_DATA);
	a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	outb(PIC1_DATA, 0x20);
	outb(PIC2_DATA, 0x28);
	outb(PIC1_DATA, 4);
	outb(PIC2_DATA, 2);
 
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);

    outb(PIC1_DATA, a1);
	outb(PIC2_DATA, a2);

    idt_setGate(32, (u32)irq0);
    idt_setGate(33, (u32)irq1);
    idt_setGate(34, (u32)irq2);
    idt_setGate(35, (u32)irq3);
    idt_setGate(36, (u32)irq4);
    idt_setGate(37, (u32)irq5);
    idt_setGate(38, (u32)irq6);
    idt_setGate(39, (u32)irq7);
    idt_setGate(40, (u32)irq8);
    idt_setGate(41, (u32)irq9);
    idt_setGate(42, (u32)irq10);
    idt_setGate(43, (u32)irq11);
    idt_setGate(44, (u32)irq12);
    idt_setGate(45, (u32)irq13);
    idt_setGate(46, (u32)irq14);
    idt_setGate(47, (u32)irq15);
}

char *error_msgs[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

typedef struct page_fault_err {
    u32 present : 1;
    u32 write   : 1;
    u32 user    : 1;
    u32 overwrt : 1;
    u32 fetch   : 1;
    u32 padding : 27;
} page_fault_err;

extern u32 kernel_stack_base;

isr_regs* previous_isr_regs;

void isr_handler() {
    isr_regs* r = (isr_regs*)task_previous_esp;

    /*task_next_esp = task_previous_esp;
    task_next_ebp = task_previous_ebp;*/

    if (isr_handlers[r->int_no] != 0) {
        isr_handlers[r->int_no](r);
        return;
    }
    
    if (r->int_no > 15)
        apo::Task::switchTask();

    vga_setColor(VGA_RED, VGA_BLACK);
    vga_writeText("\nERROR: ");
    vga_writeText(error_msgs[r->int_no]);

    if (r->int_no == 14) {
        vga_writeText(" - ");
        page_fault_err page_fault = *(page_fault_err*)(&(r->err_code));
        vga_writeText(!page_fault.present ? "PAGE_NOT_PRESENT " : "");
        vga_writeText(page_fault.write    ? "WRITE " : "READ ");
        vga_writeText(page_fault.user     ? "USER_MODE " : "KERNEL_MODE ");
        vga_writeText(page_fault.overwrt  ? "RESERVES_OVERWRITE " : "");
        vga_writeText(page_fault.fetch    ? "INSTRUCTION_FETCH " : "");
    }

    vga_writeChar('\n');

    vga_writeText("EIP = ");
    vga_writeDWord(r->eip);
    vga_writeText("\n\n");
    
    vga_writeText("EAX = ");
    vga_writeDWord(r->eax);
    vga_writeChar('\n');
    vga_writeText("EBX = ");
    vga_writeDWord(r->ebx);
    vga_writeChar('\n');
    vga_writeText("ECX = ");
    vga_writeDWord(r->ecx);
    vga_writeChar('\n');
    vga_writeText("EDX = ");
    vga_writeDWord(r->edx);
    vga_writeText("\n\n");
    
    vga_writeText("ESP = ");
    vga_writeDWord(r->esp);
    vga_writeChar('\n');
    vga_writeText("EBP = ");
    vga_writeDWord(r->ebp);
    vga_writeChar('\n');

    asm("cli");
    while (1) {};
}

void irq_handler() {
    isr_regs* r = (isr_regs*)task_previous_esp;

    if (r->int_no >= 40) outb(0xA0, 0x20);
    outb(0x20, 0x20);

    if (isr_handlers[r->int_no] != 0)
        isr_handlers[r->int_no](r);

    apo::Task::switchTask();
}

void isr_register(u8 n, isr_callback cb) {
    isr_handlers[n] = cb;
}