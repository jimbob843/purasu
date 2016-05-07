// 
// Name:	list.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	26-May-2014
// Purpose:	Generic List Type
// 

#ifndef __LIST_H__
#define __LIST_H__

#include "kernel.h"


//
// IEnumerator
//
template <class T>
class IEnumerator
{
public:
	virtual bool MoveNext() = 0;
	virtual void Reset() = 0;
	virtual T GetCurrent() = 0;
};

//
// IEnumerable
//
template <class T>
class IEnumerable
{
public:
	virtual IEnumerator<T>* GetEnumerator() = 0;
};


template <class T>
class ListEnumerator;

//
// LIST
//
template <class T>
class List : public IEnumerable<T>
{
private:
	//
	// Internal ListItem
	//
	template <class P>
	class ListItem
	{
	public:
		// Doubly-linked list
		ListItem<P> *m_prev;
		ListItem<P> *m_next;

		// Value
		P m_value;

		// Constructor
		ListItem(P value, ListItem<P>* prev, ListItem<P>* next)
		{
			m_prev = prev;
			m_next = next;
			m_value = value;
		}
	};

	//
	// List properties
	//
	ListItem<T> *m_head;
	ListItem<T> *m_tail;
	DWORD m_size;

public:
	List()
	{
		m_head = NULL;
		m_tail = NULL;
		m_size = 0;
	}
	~List()
	{
		// TODO
	}

	//
	// Add
	//
	void Add(T item)
	{
		ListItem<T> *newitem = new ListItem<T>(item, m_tail, NULL);
		if (m_tail != NULL)
		{
			m_tail->m_next = newitem;
		}
		m_tail = newitem;
		if (m_head == NULL)
		{
			m_head = newitem;
		}
		m_size++;
	}

	//
	// Prepend
	//
	void Prepend(T item)
	{
		ListItem<T> *newitem = new ListItem<T>(item, NULL, m_head);
		if (m_head != NULL)
		{
			m_head->m_prev = newitem;
		}
		m_head = newitem;
		if (m_tail == NULL)
		{
			m_tail = newitem;
		}
		m_size++;
	}

	//
	// Item
	//
	T Item(DWORD index)
	{
		if (index >= m_size)
		{
			// Not enough items in the list
			return 0;
		}

		int i = 0;
		ListItem<T> *current = m_head;
		while ((current != NULL) && (i < index))
		{
			current = current->m_next;
			i++;
		}

		if (current != NULL)
		{
			return current->m_value;
		}
		else
		{
			// Shouldn't happen as we've already
			// checked that (index >= m_size)
			return 0;
		}
	}

	//
	// Remove
	//
	void Remove(DWORD index)
	{
		if (index >= m_size)
		{
			// Not enough items in the list
			return;
		}

		// Find the item
		int i = 0;
		ListItem<T> *current = m_head;
		while ((current != NULL) && (i < index))
		{
			current = current->m_next;
			i++;
		}

		// Did we find it?
		if (current != NULL)
		{
			// Unlink this item
			if (current->m_prev != NULL)
			{
				current->m_prev = current->m_next;
			}
			if (current->next != NULL)
			{
				current->m_next = current->m_prev;
			}

			// Free the memory
			delete current;
		}
	}

	//
	// Count
	//
	DWORD Count()
	{
		return m_size;
	}

	//
	// Sort
	//
	void Sort()
	{
	}

	//
	// IEnumerable.GetEnumerator
	//
	IEnumerator<T>* GetEnumerator()
	{
		return new ListEnumerator<T>(this);
	}
};


//
// Enumerator
//
template <class T>
class ListEnumerator : public IEnumerator<T>
{
	int m_position = -1;
	List<T> *m_list;

public:
	ListEnumerator(List<T> *list)
	{
		m_list = list;
		m_position = -1;
	}

	bool MoveNext()
	{
		m_position++;
		return (m_position < m_list->Count());
	}

	void Reset()
	{
		m_position = -1;
	}

	T GetCurrent()
	{
		return m_list->Item(m_position);
	}
};


#endif // __LIST_H__
