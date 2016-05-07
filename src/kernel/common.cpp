// 
// Name:	common.c
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Common global routines
// 

#include "common.h"
#include "heap.h"
#include "console.h"

//
// C++ NEW
//
void * operator new(size_t size)
{
	return (void *)kmalloc(size);
}

//
// C++ ARRAY NEW
//
void *operator new[](size_t size)
{
    return kmalloc(size);
} 

//
// C++ DELETE
//
void operator delete(void *p)
{
	kfree(p);
}

//
// C++ ARRAY DELETE
//
void operator delete[](void *p)
{
	kfree(p);
}

//
// C++ Placement NEW + DELETE
//
//inline void *operator new(size_t, void *p)      { return p; }
//inline void *operator new[](size_t, void *p)    { return p; }
//inline void  operator delete  (void *, void *)  { };
//inline void  operator delete[](void *, void *)  { };


//
// Allow pure virtual methods
//
extern "C" void __cxa_pure_virtual()
{
    // Do nothing or print an error message.
	con_Write("PURE VIRTUAL CALLED!!!");
	asm volatile ("cli");
	asm volatile ("hlt");

}


//
// OUTPORT_BYTE
//
void OUTPORT_BYTE(WORD port, BYTE value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}


//
// INPORT_BYTE
//
BYTE INPORT_BYTE(WORD port)
{
   BYTE ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}


//
// OUTPORT_WORD
//
void OUTBYTE(WORD port, WORD value)
{
    asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}


//
// INPORT_WORD
//
WORD INPORT_WORD(WORD port)
{
   WORD ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}


//
// memset
//
void *memset(void *addr, BYTE value, size_t len)
{
	BYTE *p;

	for (p = (BYTE*)addr; len > 0; len--)
		*p++ = value;

	return addr;
}


//
// strcpy
//
int strcpy( char *s1, const char *s2 )
{
	while (*s1++ = *s2++) ;
}

//
// strcmp
//
int strcmp( const char *s1, const char *s2 )
{
 
	while (*s1 == *s2++) {
		if (*s1++ == 0) {
			return 0;
		}
	}
	
	return (*s1 - *(s2-1));
}

//
// strlen
//
int strlen( const char *s )
{
	int count = 0;
	while (*s != 0)
	{
		count++;
		s++;
	}
	return count;
}


//
// kprintf
//
void kprintf( const char *fmt, ... )
{
	va_list ap;
	va_start(ap,fmt);

	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;
			switch (*fmt)
			{
				case 'd':
					con_WriteDec( va_arg(ap, DWORD) );
					break;
				case 'x':
					con_WriteHex( va_arg(ap, DWORD) );
					break;
				//case 'w':
				//	con_WriteWORD( (WORD)(va_arg(ap, DWORD)&0xFFFF) );
				//	break;
				case 's':
					con_Write( va_arg(ap, char*) );
					break;
				case 'c':
					con_WriteChar( (char)(va_arg(ap, DWORD) & 0xFF) );
					break;
				//case 'b':
				//	con_WriteBYTE( (char)(va_arg(ap, DWORD) & 0xFF) );
				//	break;
				default:
					con_WriteChar( *fmt );
					break;
			}
		}
		else
		{
			con_WriteChar( *fmt );
		}
		fmt++;
	}
}
