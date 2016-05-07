// 
// Name:	pmm.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Physical Memory Manager
// 

#include "pmm.h"
#include "heap.h"
#include "idt.h"
#include "console.h"
#include "heap.h"

KernelHeap *g_heap = NULL;

DWORD placement_address;

// A bitset of frames - used or free.
static DWORD *frames;
static DWORD nframes;

page_directory_t *kernel_directory;
page_directory_t *current_directory;


// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))


// Static function to set a bit in the frames bitset
static void set_frame(DWORD frame_addr)
{
   DWORD frame = frame_addr/0x1000;
   DWORD idx = INDEX_FROM_BIT(frame);
   DWORD off = OFFSET_FROM_BIT(frame);
   frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(DWORD frame_addr)
{
   DWORD frame = frame_addr/0x1000;
   DWORD idx = INDEX_FROM_BIT(frame);
   DWORD off = OFFSET_FROM_BIT(frame);
   frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
static DWORD test_frame(DWORD frame_addr)
{
   DWORD frame = frame_addr/0x1000;
   DWORD idx = INDEX_FROM_BIT(frame);
   DWORD off = OFFSET_FROM_BIT(frame);
   return (frames[idx] & (0x1 << off));
}

// Static function to find the first free frame.
static DWORD first_frame()
{
   DWORD i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
       {
           // at least one bit is free here.
           for (j = 0; j < 32; j++)
           {
               DWORD toTest = 0x1 << j;
               if ( !(frames[i]&toTest) )
               {
                   return i*4*8+j;
               }
           }
       }
   }
} 

// Function to allocate a frame.
void pmm_AllocFrame(page_table_entry_t *page, int is_kernel, int is_writeable)
{
   if (page->frame != 0)
   {
       return; // Frame was already allocated, return straight away.
   }
   else
   {
       DWORD idx = first_frame(); // idx is now the index of the first free frame.
       if (idx == (DWORD)-1)
       {
           // PANIC is just a macro that prints a message to the screen then hits an infinite loop.
           PANIC("No free frames!");
       }
       set_frame(idx*0x1000); // this frame is now ours!
       page->present = 1; // Mark it as present.
       page->rw = (is_writeable)?1:0; // Should the page be writeable?
       page->user = (is_kernel)?0:1; // Should the page be user-mode?
       page->frame = idx;
   }
}

// Function to deallocate a frame.
void pmm_FreeFrame(page_table_entry_t *page)
{
   DWORD frame;
   if (!(frame=page->frame))
   {
       return; // The given page didn't actually have an allocated frame!
   }
   else
   {
       clear_frame(frame); // Frame is now free again.
       page->frame = 0x0; // Page now doesn't have a frame.
   }
} 

//
// pmm_Init
//
void pmm_Init()
{
	// The size of physical memory. For the moment we
	// assume it is 16MB big.
	DWORD mem_end_page = 0x1000000;

	nframes = mem_end_page / 0x1000;
	frames = (DWORD*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	// Let's make a page directory.
	kernel_directory = (page_directory_t*)kmalloc_aligned(sizeof(page_directory_t));
	memset(kernel_directory, 0, sizeof(page_directory_t));
	current_directory = kernel_directory;

	// First create the ordered array that we will use in the heap
	OrderedArray<header_t*> *heapindex = new OrderedArray<header_t*>(KHEAP_START, HEAP_INDEX_SIZE, new HeaderComparer());
	KernelHeap *heap = new KernelHeap();

   // Map some pages in the kernel heap area.
   // Here we call get_page but not alloc_frame. This causes page_table_t's
   // to be created where necessary. We can't allocate frames yet because they
   // they need to be identity mapped first below, and yet we can't increase
   // placement_address between identity mapping and enabling the heap!
   int i = 0;
   for (i = KHEAP_START; i < (KHEAP_START + KHEAP_INITIAL_SIZE); i += 0x1000)
       pmm_GetPageEntry(i, 1, kernel_directory); 
   
	// We need to identity map (phys addr = virt addr) from
	// 0x0 to the end of used memory, so we can access this
	// transparently, as if paging wasn't enabled.
	// NOTE that we use a while loop here deliberately.
	// inside the loop body we actually change placement_address
	// by calling kmalloc(). A while loop causes this to be
	// computed on-the-fly rather than once at the start.
	i = 0;
	while (i < placement_address)
	{
		// Kernel code is readable but not writeable from userspace.
		pmm_AllocFrame( pmm_GetPageEntry(i, 1, kernel_directory), 0, 0);
		i += 0x1000;
	}

	// Now allocate those pages we mapped earlier.
    for (i = KHEAP_START; i < (KHEAP_START + KHEAP_INITIAL_SIZE); i += 0x1000)
       pmm_AllocFrame( pmm_GetPageEntry(i, 1, kernel_directory), 0, 0);

	// Before we enable paging, we must register our page fault handler.
	idt_RegisterHandler(14, pmm_PageFaultHandler);

	// Now, enable paging!
	pmm_SwitchDirectory(kernel_directory);

	// Initialise the kernel heap.
	g_heap = heap;
	g_heap->StartHeap(heapindex, KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0); 
}

//
// pmm_SwitchDirectory
//
void pmm_SwitchDirectory(page_directory_t *dir)
{
	current_directory = dir;
	asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));

	DWORD cr0;
	asm volatile("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // Enable paging!
	asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_table_entry_t *pmm_GetPageEntry(DWORD address, int make, page_directory_t *dir)
{
	// Turn the address into an index.
	address /= 0x1000;
	// Find the page table containing this address.
	DWORD table_idx = address / 1024;

	if (dir->tables[table_idx]) // If this table is already assigned
	{
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else if(make)
	{
		DWORD tmp;
		dir->tables[table_idx] = (page_table_t*)kmalloc_alignedphysical(sizeof(page_table_t), &tmp);
		memset(dir->tables[table_idx], 0, 0x1000);
		dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else
	{
		return 0;
	}
}

void pmm_PageFaultHandler(registers_t regs)
{
	// A page fault has occurred.
	// The faulting address is stored in the CR2 register.
	DWORD faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

	// The error code gives us details of what happened.
	int present   = !(regs.err_code & 0x1); // Page not present
	int rw = regs.err_code & 0x2;           // Write operation?
	int us = regs.err_code & 0x4;           // Processor was in user-mode?
	int reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
	int id = regs.err_code & 0x10;          // Caused by an instruction fetch?

	// Output an error message.
	con_Write("Page fault! ( ");
	if (present) {con_Write("present ");}
	if (rw) {con_Write("read-only ");}
	if (us) {con_Write("user-mode ");}
	if (reserved) {con_Write("reserved ");}
	con_Write(") at 0x");
	con_WriteHex(faulting_address);
	con_Write("\n");
	PANIC("Page fault");
}
