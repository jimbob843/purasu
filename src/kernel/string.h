// 
// Name:	string.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	31-May-2014
// Purpose:	String Type
// 

#ifndef __STRING_H__
#define __STRING_H__

#include "types.h"
#include "list.h"

class String
{
	char *m_value;
public:
	String(char *value)
	{
		m_value = new char[strlen(value) + 1];
		strcpy(value, m_value);
	}

	char *Get()
	{
		return m_value;
	}

	void Append(String *toappend)
	{
		int currentlength = strlen(m_value);
		int appendlength = strlen(toappend);

		char *newvalue = new char[currentlength + appendlength + 1];
		strcpy(m_value, newvalue);
		strcpy(&m_value[currentlength], toappend);

		delete[] m_value;
		m_value = newvalue;
	}

	List<String*>* Split(char splitchar)
	{
		List<String*> *result = new List<String*>();

		return result;
	}

};

#endif // __STRING_H__
