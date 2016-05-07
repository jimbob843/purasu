// 
// Name:	pit.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Programmable Interrupt Timer (PIT)
// 

#include "pit.h"
#include "idt.h"
#include "console.h"

static DWORD tick = 0;

static void timer_callback(registers_t regs)
{
   tick++;
   con_Write("Tick: ");
   con_WriteDec(tick);
   con_Write("\n");
}


void PITDevice::InitDevice()
{
	DWORD frequency = 50;  // 50 ms

	// Firstly, register our timer callback.
	idt_RegisterHandler(IRQ_0, &timer_callback);

	// The value we send to the PIT is the value to divide it's input clock
	// (1193180 Hz) by, to get our required frequency. Important to note is
	// that the divisor must be small enough to fit into 16-bits.
	DWORD divisor = 1193180 / frequency;

	// Send the command byte.
	OUTPORT_BYTE(0x43, 0x36);

	// Divisor has to be sent byte-wise, so split here into upper/lower bytes.
	BYTE l = (BYTE)(divisor & 0xFF);
	BYTE h = (BYTE)( (divisor>>8) & 0xFF );

	// Send the frequency divisor.
	OUTPORT_BYTE(0x40, l);
	OUTPORT_BYTE(0x40, h);
}
