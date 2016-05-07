// 
// Name:	main.c
// Project:	Purasu Operating System
// Author:	James Smith
// Date:	24-May-2014
// Purpose:	Kernel entry point and startup
// 

#include "kernel.h"
#include "console.h"
#include "gdt.h"
#include "idt.h"
#include "pmm.h"
#include "dev.h"
#include "pit.h"
#include "heap.h"
#include "orderedarray.h"
#include "list.h"
#include "vfs.h"

extern DWORD bss_start;  // First byte of BSS
extern DWORD bss_end;    // First byte after BSS

extern DWORD placement_address;

//
// main
//  - mboot_ptr is a hang over from the GRUB multiboot header
//
int main(void *mboot_ptr)
{
	enum class Color
	{
		RED,
		BLUE
	};

	Color col = Color::RED;

	// Clear the BSS
	DWORD bssstart = (DWORD)&bss_start;
	DWORD bssend = (DWORD)&bss_end;
	memset( (void*)bssstart, 0, bssend - bssstart );

	// Configure where the end of the kernel is
	// We'll start allocating heap memory from here
	placement_address = bssend;

	con_Init();
	con_Clear();
	con_Write("Welcome to Purasu!\n");

	//con_Write("bssstart = 0x");
	//con_WriteHex(bssstart);
	//con_Write("\n");
	//con_Write("bssend = 0x");
	//con_WriteHex(bssend);
	//con_Write("\n");
	//con_Write("bsslen = ");
	//con_WriteDec(bssend - bssstart);
	//con_Write("\n");

	gdt_Init();
	idt_Init();

//	PITDevice *pit = new PITDevice();
	Device *pit = new PITDevice();
	pit->InitDevice();
	con_Write("pit = 0x");
	con_WriteHex((DWORD)pit);
	con_Write("\n");

//	OrderedArray<void*> *arr = new OrderedArray<void*>(20, new LessThanPredicate<void*>());


	DWORD a = (DWORD)kmalloc(8);
	pmm_Init();

	DWORD b = (DWORD)kmalloc(8);
	DWORD c = (DWORD)kmalloc(8);
	con_Write("a: ");
	con_WriteHex(a);
	con_Write(", b: ");
	con_WriteHex(b);
	con_Write("\nc: ");
	con_WriteHex(c);

	kfree((void*)c);
	kfree((void*)b);
	DWORD d = (DWORD)kmalloc(12);
	con_Write(", d: ");
	con_WriteHex(d); 
	con_Write("\n");

	kprintf("This is a %x test\n", d);

	List<int> *list = new List<int>();
	int x = 6;
	list->Add(x);
	list->Add(12);
	list->Add(24);

	IEnumerator<int> *iterator = list->GetEnumerator();
	while (iterator->MoveNext())
	{
		int x = iterator->GetCurrent();
		con_Write("LIST:");
		con_WriteDec(x);
		con_Write("\n");
	}
	delete iterator;
	delete list;

	int j = 0;
	for (j = 0; j < list->Count(); j++)
	{
		kprintf("LIST %d = %d\n", j, list->Item(j));
	}

	//
	// Setup file system
	//
	vfs_Init();
	kprintf("\n");

	VFSNode *root = vfs_GetRootNode();
	kprintf("ROOT = %s\n", root->Name());

	int k = 0;
	VFSNode *dir = NULL;
	while (root->ReadDir(k, &dir) == KRESULT_SUCCESS)
	{
		kprintf("DIR /%s/\n", dir->Name());
		k++;
	}

//	root->FindDir("dev", &dev);
//	kprintf("DEV = %s\n", dev->Name());
//	kprintf("\n");

//	asm volatile ("int $0x3");
//	asm volatile ("int $0x4"); 

//	asm volatile ("sti");
	asm volatile ("cli");
	asm volatile ("hlt");
	
	return 0;
}

