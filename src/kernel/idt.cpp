// 
// Name:	idt.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Interrupt Descriptor Table
// 

#include "idt.h"
#include "console.h"

static idt_entry_t idt_entries[256];
static idt_ptr_t   idt_ptr;
static isr_t	   interrupt_handlers[256];

extern "C"
{
	void isr_handler(registers_t regs)
	{
		idt_HandleISR(regs);
	}

	void irq_handler(registers_t regs)
	{
		idt_HandleIRQ(regs);
	}
}

//
// idt_Init
//
void idt_Init()
{
	idt_ptr.limit = (sizeof(idt_entry_t) * 256) - 1;
	idt_ptr.base  = (DWORD)&idt_entries;

	memset(&idt_entries, 0, sizeof(idt_entry_t)*256);
	memset(&interrupt_handlers, 0, sizeof(isr_t)*256);

	idt_SetGate(  0, (DWORD)ISR0  , 0x08, 0x8E );
	idt_SetGate(  1, (DWORD)ISR1  , 0x08, 0x8E );
	idt_SetGate(  2, (DWORD)ISR2  , 0x08, 0x8E );
	idt_SetGate(  3, (DWORD)ISR3  , 0x08, 0x8E );
	idt_SetGate(  4, (DWORD)ISR4  , 0x08, 0x8E );
	idt_SetGate(  5, (DWORD)ISR5  , 0x08, 0x8E );
	idt_SetGate(  6, (DWORD)ISR6  , 0x08, 0x8E );
	idt_SetGate(  7, (DWORD)ISR7  , 0x08, 0x8E );
	idt_SetGate(  8, (DWORD)ISR8  , 0x08, 0x8E );
	idt_SetGate(  9, (DWORD)ISR9  , 0x08, 0x8E );
	idt_SetGate( 10, (DWORD)ISR10 , 0x08, 0x8E );
	idt_SetGate( 11, (DWORD)ISR11 , 0x08, 0x8E );
	idt_SetGate( 12, (DWORD)ISR12 , 0x08, 0x8E );
	idt_SetGate( 13, (DWORD)ISR13 , 0x08, 0x8E );
	idt_SetGate( 14, (DWORD)ISR14 , 0x08, 0x8E );
	idt_SetGate( 15, (DWORD)ISR15 , 0x08, 0x8E );
	idt_SetGate( 16, (DWORD)ISR16 , 0x08, 0x8E );
	idt_SetGate( 17, (DWORD)ISR17 , 0x08, 0x8E );
	idt_SetGate( 18, (DWORD)ISR18 , 0x08, 0x8E );
	idt_SetGate( 19, (DWORD)ISR19 , 0x08, 0x8E );
	idt_SetGate( 20, (DWORD)ISR20 , 0x08, 0x8E );
	idt_SetGate( 21, (DWORD)ISR21 , 0x08, 0x8E );
	idt_SetGate( 22, (DWORD)ISR22 , 0x08, 0x8E );
	idt_SetGate( 23, (DWORD)ISR23 , 0x08, 0x8E );
	idt_SetGate( 24, (DWORD)ISR24 , 0x08, 0x8E );
	idt_SetGate( 25, (DWORD)ISR25 , 0x08, 0x8E );
	idt_SetGate( 26, (DWORD)ISR26 , 0x08, 0x8E );
	idt_SetGate( 27, (DWORD)ISR27 , 0x08, 0x8E );
	idt_SetGate( 28, (DWORD)ISR28 , 0x08, 0x8E );
	idt_SetGate( 29, (DWORD)ISR29 , 0x08, 0x8E );
	idt_SetGate( 30, (DWORD)ISR30 , 0x08, 0x8E );
	idt_SetGate( 31, (DWORD)ISR31 , 0x08, 0x8E );

	idt_SetGate( 32, (DWORD)IRQ0  , 0x08, 0x8E );
	idt_SetGate( 33, (DWORD)IRQ1  , 0x08, 0x8E );
	idt_SetGate( 34, (DWORD)IRQ2  , 0x08, 0x8E );
	idt_SetGate( 35, (DWORD)IRQ3  , 0x08, 0x8E );
	idt_SetGate( 36, (DWORD)IRQ4  , 0x08, 0x8E );
	idt_SetGate( 37, (DWORD)IRQ5  , 0x08, 0x8E );
	idt_SetGate( 38, (DWORD)IRQ6  , 0x08, 0x8E );
	idt_SetGate( 39, (DWORD)IRQ7  , 0x08, 0x8E );
	idt_SetGate( 40, (DWORD)IRQ8  , 0x08, 0x8E );
	idt_SetGate( 41, (DWORD)IRQ9  , 0x08, 0x8E );
	idt_SetGate( 42, (DWORD)IRQ10 , 0x08, 0x8E );
	idt_SetGate( 43, (DWORD)IRQ11 , 0x08, 0x8E );
	idt_SetGate( 44, (DWORD)IRQ12 , 0x08, 0x8E );
	idt_SetGate( 45, (DWORD)IRQ13 , 0x08, 0x8E );
	idt_SetGate( 46, (DWORD)IRQ14 , 0x08, 0x8E );
	idt_SetGate( 47, (DWORD)IRQ15 , 0x08, 0x8E );

	IDT_FLUSH((DWORD)&idt_ptr);

	// Remap the irq table.
	OUTPORT_BYTE(0x20, 0x11);
	OUTPORT_BYTE(0xA0, 0x11);
	OUTPORT_BYTE(0x21, 0x20);
	OUTPORT_BYTE(0xA1, 0x28);
	OUTPORT_BYTE(0x21, 0x04);
	OUTPORT_BYTE(0xA1, 0x02);
	OUTPORT_BYTE(0x21, 0x01);
	OUTPORT_BYTE(0xA1, 0x01);
	OUTPORT_BYTE(0x21, 0x00);
	OUTPORT_BYTE(0xA1, 0x00);
}

//
// idt_SetGate
//
void idt_SetGate(int num, DWORD base, WORD sel, BYTE flags)
{
   idt_entries[num].base_lo = base & 0xFFFF;
   idt_entries[num].base_hi = (base >> 16) & 0xFFFF;

   idt_entries[num].sel     = sel;
   idt_entries[num].always0 = 0;
   // We must uncomment the OR below when we get to using user-mode.
   // It sets the interrupt gate's privilege level to 3.
   idt_entries[num].flags   = flags /* | 0x60 */;
}


//
// RegisterHandler
//
void idt_RegisterHandler(BYTE index, isr_t handler)
{
	interrupt_handlers[index] = handler;
}


void idt_HandleIRQ(registers_t regs)
{
	con_Write("recieved IRQ: ");
	con_WriteDec(regs.int_no);
	con_Write("\n");

	// Send an EOI (end of interrupt) signal to the PICs.
	// If this interrupt involved the slave.
	if (regs.int_no >= 40)
	{
		// Send reset signal to slave.
		OUTPORT_BYTE(0xA0, 0x20);
	}
	// Send reset signal to master. (As well as slave, if necessary).
	OUTPORT_BYTE(0x20, 0x20);

	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}
}

void idt_HandleISR(registers_t regs)
{
	con_Write("recieved interrupt: ");
	con_WriteDec(regs.int_no);
	con_Write("\n");

	if (interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}

}

