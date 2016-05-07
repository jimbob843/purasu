// 
// Name:	heap.cpp
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Kernel Heap
// 

#include "heap.h"
#include "pmm.h"
#include "console.h"

extern DWORD placement_address;
extern page_directory_t *kernel_directory;

extern KernelHeap *g_heap;


static void *kmalloc_internal(DWORD sz, BYTE align, DWORD *phys)
{
	if (g_heap == NULL)
	{
		if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
		{
			// Align it.
			placement_address &= 0xFFFFF000;
			placement_address += 0x1000;
		}
		if (phys != NULL)
		{
			*phys = placement_address;
		}
		DWORD tmp = placement_address;
		placement_address += sz;

		return (void *)tmp;
	}
	else
	{
		// Use the proper heap
		return g_heap->Alloc( sz, align );
	}
}

void *kmalloc(size_t size)
{
	return kmalloc_internal(size, 0, NULL);
}

void *kmalloc_aligned(size_t size)
{
	return kmalloc_internal(size, 1, NULL);
}

void *kmalloc_physical(size_t size, DWORD *physicalAddr)
{
	return kmalloc_internal(size, 0, physicalAddr);
}

void *kmalloc_alignedphysical(size_t size, DWORD *physicalAddr)
{
	return kmalloc_internal(size, 1, physicalAddr);
}


void kfree(void *p)
{
	if (g_heap == NULL)
	{
		// Sorry, no freeing before the proper heap is setup
	}
	else
	{
		g_heap->Free(p);
	}
}

KernelHeap::KernelHeap()
{
}

void KernelHeap::StartHeap(OrderedArray<header_t*> *heapindex, DWORD start, DWORD end, DWORD max, BYTE supervisor, BYTE readonly)
{
   // All our assumptions are made on startAddress and endAddress being page-aligned.
   //ASSERT(start%0x1000 == 0);
   //ASSERT(end_addr%0x1000 == 0);

	// Initialise the index.
	m_index = heapindex;

//	m_index = new OrderedArray<header_t*>(start, HEAP_INDEX_SIZE, new HeaderComparer());
   //m_index = place_ordered_array( (void*)start, HEAP_INDEX_SIZE, &header_t_less_than);

   // Shift the start address forward to resemble where we can start putting data.
   start += sizeof(void*) * HEAP_INDEX_SIZE;

   // Make sure the start address is page-aligned.
   if (start & 0xFFFFF000 != 0)
   {
       start &= 0xFFFFF000;
       start += 0x1000;
   }
   // Write the start, end and max addresses into the heap structure.
   m_start_address = start;
   m_end_address = end;
   m_max_address = max;
   m_supervisor = supervisor;
   m_readonly = readonly;

   // We start off with one large hole in the index.
   header_t *hole = (header_t *)start;
   hole->size = end - start;
   hole->magic = HEAP_MAGIC_HEAD;
   hole->is_hole = 1;

   m_index->Insert(hole);
}


int KernelHeap::FindSmallestHole(DWORD size, BYTE page_align)
{
   // Find the smallest hole that will fit.
   DWORD iterator = 0;

   while (iterator < m_index->Size())
   {
       header_t *header = m_index->Lookup(iterator);
       // If the user has requested the memory be page-aligned
       if (page_align > 0)
       {
           // Page-align the starting point of this header.
           DWORD location = (DWORD)header;
           int offset = 0;
           if ((location + sizeof(header_t)) & 0xFFFFF000 != 0)
               offset = 0x1000 /* page size */  - (location + sizeof(header_t)) % 0x1000;
           int hole_size = (int)header->size - offset;
           // Can we fit now?
           if (hole_size >= (int)size)
               break;
       }
       else if (header->size >= size)
           break;
       iterator++;
   }
   // Why did the loop exit?
   if (iterator == m_index->Size())
       return -1; // We got to the end and didn't find anything.
   else
       return iterator;
}


void *KernelHeap::Alloc(DWORD size, BYTE page_align)
{
   // Make sure we take the size of header/footer into account.
   DWORD new_size = size + sizeof(header_t) + sizeof(footer_t);

   // Find the smallest hole that will fit.
   int iterator = FindSmallestHole(new_size, page_align);
   

	if (iterator == -1) // If we didn't find a suitable hole
   {
       // Save some previous data.
       DWORD old_length = m_end_address - m_start_address;
       DWORD old_end_address = m_end_address;

       // We need to allocate some more space.
       Expand(old_length + new_size);
       DWORD new_length = m_end_address - m_start_address;

       // Find the endmost header. (Not endmost in size, but in location).
       iterator = 0;
       // Vars to hold the index of, and value of, the endmost header found so far.
       DWORD idx = -1;
	   DWORD value = 0x0;
       while (iterator < m_index->Size())
       {
           DWORD tmp = (DWORD)m_index->Lookup(iterator);
           if (tmp > value)
           {
               value = tmp;
               idx = iterator;
           }
           iterator++;
       }

       // If we didn't find ANY headers, we need to add one.
       if (idx == -1)
       {
           header_t *header = (header_t *)old_end_address;
           header->magic = HEAP_MAGIC_HEAD;
           header->size = new_length - old_length;
           header->is_hole = 1;
           footer_t *footer = (footer_t *) (old_end_address + header->size - sizeof(footer_t));
           footer->magic = HEAP_MAGIC_FOOT;
           footer->header = header;
		   m_index->Insert(header);
       }
       else
       {
           // The last header needs adjusting.
           header_t *header = m_index->Lookup(idx);
           header->size += new_length - old_length;
           // Rewrite the footer.
           footer_t *footer = (footer_t *) ( (DWORD)header + header->size - sizeof(footer_t) );
           footer->header = header;
           footer->magic = HEAP_MAGIC_FOOT;
       }
       // We now have enough space. Recurse, and call the function again.
       return Alloc(size, page_align);
   } 

   header_t *orig_hole_header = m_index->Lookup(iterator);
   DWORD orig_hole_pos = (DWORD)orig_hole_header;
   DWORD orig_hole_size = orig_hole_header->size;
   // Here we work out if we should split the hole we found into two parts.
   // Is the original hole size - requested hole size less than the overhead for adding a new hole?
   if (orig_hole_size-new_size < (sizeof(header_t) + sizeof(footer_t)))
   {
       // Then just increase the requested size to the size of the hole we found.
       size += orig_hole_size-new_size;
       new_size = orig_hole_size;
   } 

   // If we need to page-align the data, do it now and make a new hole in front of our block.
   if (page_align && (orig_hole_pos & 0xFFFFF000))
   {
       DWORD new_location   = orig_hole_pos + 0x1000 /* page size */ - (orig_hole_pos & 0xFFF) - sizeof(header_t);
       header_t *hole_header = (header_t *)orig_hole_pos;
       hole_header->size     = 0x1000 /* page size */ - (orig_hole_pos & 0xFFF) - sizeof(header_t);
       hole_header->magic    = HEAP_MAGIC_HEAD;
       hole_header->is_hole  = 1;
       footer_t *hole_footer = (footer_t *) ( (DWORD)new_location - sizeof(footer_t) );
       hole_footer->magic    = HEAP_MAGIC_FOOT;
       hole_footer->header   = hole_header;
       orig_hole_pos         = new_location;
       orig_hole_size        = orig_hole_size - hole_header->size;
   }
   else
   {
       // Else we don't need this hole any more, delete it from the index.
	   m_index->Remove(iterator);
   } 

   // Overwrite the original header...
   header_t *block_header  = (header_t *)orig_hole_pos;
   block_header->magic     = HEAP_MAGIC_HEAD;
   block_header->is_hole   = 0;
   block_header->size      = new_size;
   // ...And the footer
   footer_t *block_footer  = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
   block_footer->magic     = HEAP_MAGIC_FOOT;
   block_footer->header    = block_header; 

   // We may need to write a new hole after the allocated block.
   // We do this only if the new hole would have positive size...
   if (orig_hole_size - new_size > 0)
   {
       header_t *hole_header = (header_t *) (orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
       hole_header->magic    = HEAP_MAGIC_HEAD;
       hole_header->is_hole  = 1;
       hole_header->size     = orig_hole_size - new_size;
       footer_t *hole_footer = (footer_t *) ( (DWORD)hole_header + orig_hole_size - new_size - sizeof(footer_t) );
       if ((DWORD)hole_footer < m_end_address)
       {
           hole_footer->magic = HEAP_MAGIC_FOOT;
           hole_footer->header = hole_header;
       }
       // Put the new hole in the index;
	   m_index->Insert(hole_header);
   } 

   // ...And we're done!
   return (void *) ( (DWORD)block_header + sizeof(header_t) );
}


void KernelHeap::Free(void *p)
{
	// Exit gracefully for null pointers.
	if (p == 0)
		return;

	// Get the header and footer associated with this pointer.
	header_t *header = (header_t*) ( (DWORD)p - sizeof(header_t) );
	footer_t *footer = (footer_t*) ( (DWORD)header + header->size - sizeof(footer_t) );

	// Sanity checks.
	//ASSERT(header->magic == HEAP_MAGIC);
	//ASSERT(footer->magic == HEAP_MAGIC); 

	// Make us a hole.
	header->is_hole = 1;

	// Do we want to add this header into the 'free holes' index?
	char do_add = 1; 

	// Unify left
	// If the thing immediately to the left of us is a footer...
	footer_t *test_footer = (footer_t*) ( (DWORD)header - sizeof(footer_t) );
	if (test_footer->magic == HEAP_MAGIC_FOOT &&
		test_footer->header->is_hole == 1)
	{
		DWORD cache_size = header->size; // Cache our current size.
		header = test_footer->header;     // Rewrite our header with the new one.
		footer->header = header;          // Rewrite our footer to point to the new header.
		header->size += cache_size;       // Change the size.
		do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
	} 

	// Unify right
	// If the thing immediately to the right of us is a header...
	header_t *test_header = (header_t*) ( (DWORD)footer + sizeof(footer_t) );
	if (test_header->magic == HEAP_MAGIC_HEAD &&
		test_header->is_hole)
	{
		header->size += test_header->size; // Increase our size.
		test_footer = (footer_t*) ( (DWORD)test_header + // Rewrite it's footer to point to our header.
										test_header->size - sizeof(footer_t) );
		footer = test_footer;
		// Find and remove this header from the index.
		DWORD iterator = 0;
		while ( (iterator < m_index->Size()) &&
			(m_index->Lookup(iterator) != test_header) )
				iterator++;

		// Make sure we actually found the item.
		//ASSERT(iterator < heap->index.size);
		// Remove it.
		m_index->Remove(iterator);
	} 

	// If the footer location is the end address, we can contract.
	if ( (DWORD)footer+sizeof(footer_t) == m_end_address)
	{
		DWORD old_length = m_end_address - m_start_address;
		DWORD new_length = Contract( (DWORD)header - m_start_address);
		// Check how big we will be after resizing.
		if (header->size - (old_length - new_length) > 0)
		{
			// We will still exist, so resize us.
			header->size -= old_length-new_length;
			footer = (footer_t*) ( (DWORD)header + header->size - sizeof(footer_t) );
			footer->magic = HEAP_MAGIC_FOOT;
			footer->header = header;
		}
		else
		{
			// We will no longer exist :(. Remove us from the index.
			DWORD iterator = 0;
			while ( (iterator < m_index->Size()) &&
				(m_index->Lookup(iterator) != test_header) )
					iterator++;
			// If we didn't find ourselves, we have nothing to remove.
			if (iterator < m_index->Size())
				m_index->Remove(iterator);
		}
	}

	if (do_add == 1)
		m_index->Insert(header);
}


void KernelHeap::Expand(DWORD new_size)
{
   // Sanity check.
   //ASSERT(new_size > heap->end_address - heap->start_address);

   // Get the nearest following page boundary.
   if ((new_size & 0xFFFFF000) != 0)
   {
       new_size &= 0xFFFFF000;
       new_size += 0x1000;
   }
   // Make sure we are not overreaching ourselves.
   //ASSERT(heap->start_address+new_size <= heap->max_address);

   // This should always be on a page boundary.
   DWORD old_size = m_end_address - m_start_address;
   DWORD i = old_size;
   while (i < new_size)
   {
       pmm_AllocFrame( pmm_GetPageEntry(m_start_address + i, 1, kernel_directory),
                    (m_supervisor)?1:0, (m_readonly)?0:1);
       i += 0x1000 /* page size */;
   }
   m_end_address = m_start_address + new_size;
}

DWORD KernelHeap::Contract(DWORD new_size)
{
   // Sanity check.
   //ASSERT(new_size < heap->end_address-heap->start_address);

   // Get the nearest following page boundary.
   if (new_size & 0x1000)
   {
       new_size &= 0x1000;
       new_size += 0x1000;
   }
   // Don't contract too far!
   if (new_size < HEAP_MIN_SIZE)
       new_size = HEAP_MIN_SIZE;
   DWORD old_size = m_end_address - m_start_address;
   DWORD i = old_size - 0x1000;
   while (new_size < i)
   {
       pmm_FreeFrame(pmm_GetPageEntry(m_start_address + i, 0, kernel_directory));
       i -= 0x1000;
   }
   m_end_address = m_start_address + new_size;

   return new_size;
}
