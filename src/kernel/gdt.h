// 
// Name:	gdt.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Global Descriptor Table
// 

#ifndef __GDT_H__
#define __GDT_H__

#include "kernel.h"

extern "C" void GDT_FLUSH(DWORD ptr);

extern void gdt_Init();
extern void gdt_SetGate(int num, DWORD base, DWORD limit, BYTE access, BYTE gran);

//
// GDT Entry
//
struct gdt_entry_struct
{
   WORD limit_low;           // The lower 16 bits of the limit.
   WORD base_low;            // The lower 16 bits of the base.
   BYTE base_middle;         // The next 8 bits of the base.
   BYTE access;              // Access flags, determine what ring this segment can be used in.
   BYTE granularity;
   BYTE base_high;           // The last 8 bits of the base.
} __attribute__((packed));
typedef struct gdt_entry_struct gdt_entry_t; 

//
// GDT Pointer
//
struct gdt_ptr_struct
{
   WORD  limit;               // The upper 16 bits of all selector limits.
   DWORD base;                // The address of the first gdt_entry_t struct.
}
 __attribute__((packed));
typedef struct gdt_ptr_struct gdt_ptr_t; 


#endif // __GDT_H__
