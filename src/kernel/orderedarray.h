// 
// Name:	orderedarray.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	26-May-2014
// Purpose:	Ordered Array Type
// 

#ifndef __ORDEREDARRAY_H__
#define __ORDEREDARRAY_H__

#include "kernel.h"

template <class T>
class Predicate
{
public:
	virtual bool Compare(T a, T b) = 0;
};

template <class T>
class LessThanPredicate : public Predicate<T>
{
public:
	bool Compare(T a, T b)
	{
		return a < b;
	}
};


template <class T>
class OrderedArray
{
	T *m_array;
	DWORD m_size;
	DWORD m_maxsize;
	Predicate<T> *m_predicate;

public:
	OrderedArray(DWORD placement_addr, DWORD maxsize, Predicate<T> *predicate)
	{
		m_array = (T*)placement_addr;
		m_size = 0;
		m_maxsize = maxsize;
		m_predicate = predicate;
	}
	~OrderedArray()
	{
	}
	void Insert(T item)
	{
		DWORD iterator = 0;
		while (iterator < m_size && m_predicate->Compare(m_array[iterator], item))
		{
			iterator++;
		}

		if (iterator == m_size) // just add at the end of the array.
		{
			m_array[m_size++] = item;
		}
		else
		{
			T tmp = m_array[iterator];
			m_array[iterator] = item;
			while (iterator < m_size)
			{
				iterator++;
				T tmp2 = m_array[iterator];
				m_array[iterator] = tmp;
				tmp = tmp2;
			}
			m_size++;
		}
	}

	T Lookup(DWORD index)
	{
		return m_array[index];
	}

	void Remove(DWORD index)
	{
		int i = index;
		while (i < m_size)
		{
			m_array[i] = m_array[i+1];
			i++;
		}
		m_size--;
	}

	DWORD Size()
	{
		return m_size;
	}
};


#endif // __ORDEREDARRAY_H__
