// 
// Name:	pmm.h
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Physical Memory Manager
// 

#ifndef __PMM_H__
#define __PMM_H__

#include "kernel.h"

//
// page table entry
//
typedef struct page_table_entry
{
   DWORD present    : 1;   // Page present in memory
   DWORD rw         : 1;   // Read-only if clear, readwrite if set
   DWORD user       : 1;   // Supervisor level only if clear
   DWORD accessed   : 1;   // Has the page been accessed since last refresh?
   DWORD dirty      : 1;   // Has the page been written to since last refresh?
   DWORD unused     : 7;   // Amalgamation of unused and reserved bits
   DWORD frame      : 20;  // Frame address (shifted right 12 bits)
} page_table_entry_t;

//
// page table
//
typedef struct page_table
{
   page_table_entry_t pages[1024];
} page_table_t;


//
// page directory
//
typedef struct page_directory
{
   page_table_t *tables[1024];
   /**
      Array of pointers to the pagetables above, but gives their *physical*
      location, for loading into the CR3 register.
   **/
   DWORD tablesPhysical[1024];
   /**
      The physical address of tablesPhysical. This comes into play
      when we get our kernel heap allocated and the directory
      may be in a different location in virtual memory.
   **/
   DWORD physicalAddr;
} page_directory_t;



extern void pmm_Init();
extern void pmm_SwitchDirectory(page_directory_t *dir);
extern page_table_entry_t *pmm_GetPageEntry(DWORD address, int make, page_directory_t *dir);
extern void pmm_PageFaultHandler(registers_t regs);
extern void pmm_AllocFrame(page_table_entry_t *page, int is_kernel, int is_writeable);
extern void pmm_FreeFrame(page_table_entry_t *page);


#endif // __PMM_H__
