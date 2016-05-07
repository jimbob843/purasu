// 
// Name:	console.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Simple console implementation
// 

#include "console.h"
#include "common.h"

#define SCREENRES_X 80
#define SCREENRES_Y 25

#define COLOUR_BLACK 0
#define COLOUR_WHITE 15

#define CRTC_ADDRESS	0x3D4
#define CRTC_VALUE		0x3D5
#define CRTC_ADDR_CURSORHIGH	0x0E
#define CRTC_ADDR_CURSORLOW		0x0F


static BYTE cursor_x = 0;
static BYTE cursor_y = 0;
static volatile WORD *video_memory = (WORD*)0xB8000;

// Scrolls the text on the screen up by one line.
static void scroll()
{
   // Get a space character with the default colour attributes.
   BYTE attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
   WORD blank = 0x20 /* space */ | (attributeByte << 8);

   // Row 25 is the end, this means we need to scroll up
   if(cursor_y >= 25)
   {
       // Move the current text chunk that makes up the screen
       // back in the buffer by a line
       int i;
       for (i = 0*80; i < 24*80; i++)
       {
           video_memory[i] = video_memory[i+80];
       }

       // The last line should now be blank. Do this by writing
       // 80 spaces to it.
       for (i = 24*80; i < 25*80; i++)
       {
           video_memory[i] = blank;
       }
       // The cursor should now be on the last line.
       cursor_y = 24;
   }
}

void con_Init()
{
	cursor_x = 0;
	cursor_y = 0;
	video_memory = (WORD*)0xB8000;
}

void con_Clear()
{
	BYTE attribute = (COLOUR_BLACK << 4) | (COLOUR_WHITE & 0x0F);
	WORD space = 0x20 | (attribute << 8);

	int i = 0;
	for (i = 0; i < (SCREENRES_X * SCREENRES_Y); i++)
	{
		video_memory[i] = space;
	}

	cursor_x = 0;
	cursor_y = 0;
	con_UpdateCursor();
}

void con_UpdateCursor()
{
	WORD position = (cursor_y * SCREENRES_X) + cursor_x;

	OUTPORT_BYTE( CRTC_ADDRESS, CRTC_ADDR_CURSORHIGH );
	OUTPORT_BYTE( CRTC_VALUE,   position >> 8 );
	OUTPORT_BYTE( CRTC_ADDRESS, CRTC_ADDR_CURSORLOW );
	OUTPORT_BYTE( CRTC_VALUE,   position );
}

void con_Write(const char *msg)
{
	int i = 0;

	while (msg[i] != 0)
	{
		con_WriteChar(msg[i]);
		i++;
	}
}

void con_WriteChar(const char c)
{
	BYTE attribute = (COLOUR_BLACK << 4) | (COLOUR_WHITE & 0x0F);
	
	if (c == '\r')
	{
		cursor_x = 0;
	}
	else if (c == '\n')
	{
		cursor_x = 0;
		cursor_y++;
	}
	else if (c >= ' ')
	{
		DWORD location = (cursor_y * SCREENRES_X) + cursor_x;
		video_memory[location] = c | (attribute << 8); 
		cursor_x++;
	}

	if (cursor_x >= SCREENRES_X)
	{
		cursor_x = 0;
		cursor_y++;
	}

	// Scroll if required
	scroll();

	// Refresh the cursor
	con_UpdateCursor();
}


void con_WriteHexDigit(BYTE c)
{
	/* Input: 0-F, prints char */
	char x = '0';

	if ((c < 0) || (c > 0xF))
	{
		/* Bad char */
		x = '0';
	} else
	{
		if ((c >= 0) && (c <= 9))
		{
			x = '0' + c;
		} else {
			x = '0' + c + 7;
		}
	}

	con_WriteChar( x );
}


void con_WriteHex(DWORD n)
{
	DWORD i = 0;
	DWORD shift = 32;		// 32 bits in total

	// Print each nibble of the DWORD, starting at the top
	for (i=0; i<8; i++)
	{
		shift -= 4;		// Move to next nibble
		con_WriteHexDigit( (0xF & (n >> shift)) );
	}
}

void con_WriteDec(DWORD n)
{
	BYTE revdigits[12];
	DWORD current = n;
	DWORD i = 0;
	DWORD j = 0;

	if (n == 0)
	{
		con_WriteHexDigit( 0 );
	}

	while (current>0)
	{
		revdigits[i] = current % 10;
		current /= 10;
		i++;
	}

	for (j=i; j>0; j--)
	{
		con_WriteHexDigit( revdigits[j-1] );
	}
}

