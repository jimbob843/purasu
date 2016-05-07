// 
// Name:	gdt.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Global Descriptor Table
// 

#include "gdt.h"

static gdt_entry_t gdt_entries[5];
static gdt_ptr_t   gdt_ptr;

//
// gdt_Init
//
void gdt_Init()
{
	gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
	gdt_ptr.base  = (DWORD)&gdt_entries;

	gdt_SetGate(0, 0, 0, 0, 0);                // Null segment
	gdt_SetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	gdt_SetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	gdt_SetGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	gdt_SetGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	GDT_FLUSH((DWORD)&gdt_ptr);
}

//
// gdt_SetGate
//
void gdt_SetGate(int num, DWORD base, DWORD limit, BYTE access, BYTE gran)
{
	gdt_entries[num].base_low    = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high   = (base >> 24) & 0xFF;

	gdt_entries[num].limit_low   = (limit & 0xFFFF);
	gdt_entries[num].granularity = (limit >> 16) & 0x0F;
	
	gdt_entries[num].granularity |= gran & 0xF0;
	gdt_entries[num].access      = access;
}

