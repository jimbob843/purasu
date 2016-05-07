// 
// Name:	pit.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Programmable Interrupt Timer (PIT)
// 

#ifndef __PIT_H__
#define __PIT_H__

#include "kernel.h"
#include "dev.h"

class PITDevice : public Device
{
	int x;
public:
	void InitDevice();
};

#endif // __PIT_H__
