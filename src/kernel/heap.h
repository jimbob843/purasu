// 
// Name:	heap.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Kernel Heap
// 

#ifndef __HEAP_H__
#define __HEAP_H__

#include "kernel.h"
#include "orderedarray.h"

extern void *kmalloc(size_t size);
extern void *kmalloc_aligned(size_t size);
extern void *kmalloc_physical(size_t size, DWORD *physicalAddr);
extern void *kmalloc_alignedphysical(size_t size, DWORD *physicalAddr);
extern void kfree(void *p);

typedef struct
{
  DWORD magic;  // Magic number, used for error checking and identification.
  DWORD is_hole; // 1 if this is a hole, 0 if this is a block.
  DWORD size;   // Size of the block, including this and the footer.
} header_t;

typedef struct
{
  DWORD magic;     // Magic number, same as in header_t.
  header_t *header; // Pointer to the block header.
} footer_t; 

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000
#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC_HEAD   0xDEADBEEF
#define HEAP_MAGIC_FOOT   0xFEEBDAED
#define HEAP_MIN_SIZE     0x70000

class KernelHeap
{
   OrderedArray<header_t*> *m_index;
   DWORD m_start_address; // The start of our allocated space.
   DWORD m_end_address;   // The end of our allocated space. May be expanded up to max_address.
   DWORD m_max_address;   // The maximum address the heap can be expanded to.
   BYTE m_supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
   BYTE m_readonly;       // Should extra pages requested by us be mapped as read-only?

public:
	KernelHeap();
	void StartHeap(OrderedArray<header_t*> *heapindex, DWORD start, DWORD end, DWORD max, BYTE supervisor, BYTE readonly);
	int FindSmallestHole(DWORD size, BYTE page_align);
	void *Alloc(DWORD size, BYTE page_align);
	void Free(void *p); 
	void Expand(DWORD new_size);
	DWORD Contract(DWORD new_size);
};


class HeaderComparer : public Predicate<header_t*>
{
public:
	bool Compare(header_t* a, header_t* b)
	{
		// Compare sizes
		return (a->size) < (b->size);
	}
};

#endif // __HEAP_H__
