// 
// Name:	dev.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Device Manager
// 

#ifndef __DEV_H__
#define __DEV_H__

#include "kernel.h"

class Device
{
public:
	virtual void InitDevice() = 0;
};

#endif // __DEV_H__
