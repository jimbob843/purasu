// 
// Name:	idt.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Interrupt Descriptor Table
// 

#ifndef __IDT_H__
#define __IDT_H__

#include "kernel.h"

extern "C" void IDT_FLUSH(DWORD ptr);

// ISR handler func
typedef void (*isr_t)(registers_t);

// IRQ handler indexes
#define IRQ_0  32
#define IRQ_1  33
#define IRQ_2  34
#define IRQ_3  35
#define IRQ_4  36
#define IRQ_5  37
#define IRQ_6  38
#define IRQ_7  39
#define IRQ_8  40
#define IRQ_9  41
#define IRQ_10 42
#define IRQ_11 43
#define IRQ_12 44
#define IRQ_13 45
#define IRQ_14 46
#define IRQ_15 47

//
// IDT Entry
//
struct idt_entry_struct
{
   WORD base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
   WORD sel;                 // Kernel segment selector.
   BYTE always0;             // This must always be zero.
   BYTE flags;               // More flags. See documentation.
   WORD base_hi;             // The upper 16 bits of the address to jump to.
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

//
// IDT Pointer
//
struct idt_ptr_struct
{
   WORD  limit;               // The upper 16 bits of all selector limits.
   DWORD base;                // The address of the first gdt_entry_t struct.
}
 __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t; 


extern "C" 
{
	extern void ISR0();
	extern void ISR1();
	extern void ISR2();
	extern void ISR3();
	extern void ISR4();
	extern void ISR5();
	extern void ISR6();
	extern void ISR7();
	extern void ISR8();
	extern void ISR9();
	extern void ISR10();
	extern void ISR11();
	extern void ISR12();
	extern void ISR13();
	extern void ISR14();
	extern void ISR15();
	extern void ISR16();
	extern void ISR17();
	extern void ISR18();
	extern void ISR19();
	extern void ISR20();
	extern void ISR21();
	extern void ISR22();
	extern void ISR23();
	extern void ISR24();
	extern void ISR25();
	extern void ISR26();
	extern void ISR27();
	extern void ISR28();
	extern void ISR29();
	extern void ISR30();
	extern void ISR31();

	extern void IRQ0();
	extern void IRQ1();
	extern void IRQ2();
	extern void IRQ3();
	extern void IRQ4();
	extern void IRQ5();
	extern void IRQ6();
	extern void IRQ7();
	extern void IRQ8();
	extern void IRQ9();
	extern void IRQ10();
	extern void IRQ11();
	extern void IRQ12();
	extern void IRQ13();
	extern void IRQ14();
	extern void IRQ15();
}


extern void idt_Init();
extern void idt_SetGate(int num, DWORD base, WORD sel, BYTE flags);
extern void idt_RegisterHandler(BYTE index, isr_t handler);
extern void idt_HandleIRQ(registers_t regs);
extern void idt_HandleISR(registers_t regs);


#endif // __IDT_H__
