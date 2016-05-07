// 
// Name:	kernel.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Main header file
// 

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "types.h"
#include "common.h"
//#include "string.h"

//
// Useful Assembler Commands
//
#define STOP_PROCESSOR() asm volatile("cli"); asm volatile("hlt")
#define ENABLED_INTERRUPTS() asm volatile("sti")
#define DISABLE_INTERRUPTS() asm volatile("cli")

//
// PANIC!
//
#define PANIC(msg) con_Write(msg); asm volatile("cli"); asm volatile("hlt")


#endif // __KERNEL_H__

