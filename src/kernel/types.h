// 
// Name:	types.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Definition of common types
// 

#ifndef __TYPES_H__
#define __TYPES_H__


#define NULL 0


//
// Standard Types
//
typedef char  int8;
typedef short int16;
typedef int   int32;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

typedef unsigned char      BYTE;
typedef unsigned short     WORD;
typedef unsigned int       DWORD;
typedef unsigned long long QWORD;

typedef long unsigned int  size_t;
typedef DWORD KRESULT;
#define KRESULT_SUCCESS  0
#define KRESULT_FAILURE  1


//
// Variable Argument Defines
//
#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)
#define va_copy(d,s)  __builtin_va_copy(d,s)
typedef __builtin_va_list va_list;


//
// Common Macros
//
#define ABS(x)   (((x)<0)?(-(x)):(x))
#define SGN(x)   (((x)<0)?(-1):(1))
#define MAX(x,y) (((x)>(y))?((x)):(y))
#define MIN(x,y) (((x)<(y))?((x)):(y))

// Registers passed to an interrupt handler
typedef struct registers
{
   DWORD ds;                  // Data segment selector
   DWORD edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
   DWORD int_no, err_code;    // Interrupt number and error code (if applicable)
   DWORD eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t; 


#endif // __TYPES_H__

