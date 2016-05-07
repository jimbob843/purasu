// 
// Name:	console.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Simple console implementation
// 

#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "kernel.h"

extern void con_Init();
extern void con_Clear();
extern void con_UpdateCursor();
extern void con_Write(const char *msg);
extern void con_WriteChar(const char c);
extern void con_WriteHexDigit(BYTE c);
extern void con_WriteHex(DWORD n);
extern void con_WriteDec(DWORD n);


#endif // __CONSOLE_H__

