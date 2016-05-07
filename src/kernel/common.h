// 
// Name:	common.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Definition of common functions
// 

#ifndef __COMMON_H__
#define __COMMON_H__

#include "kernel.h"

extern void * operator new(long unsigned int size);
extern void operator delete(void *);
extern void OUTPORT_BYTE(WORD port, BYTE value);
extern BYTE INPORT_BYTE(WORD port);
extern void OUTPORT_WORD(WORD port, WORD value);
extern WORD INPORT_WORD(WORD port);
extern void *memset(void *addr, BYTE value, size_t len);
extern void kprintf(const char *fmt, ... );
extern int strcpy(char *s1, const char *s2);
extern int strcmp(const char *s1, const char *s2);
extern int strlen(const char *s );

#endif // __COMMON_H__

