#
# Name:		Makefile
# Author:	James Smith
# Date:		24-May-2014
# Purpose:	Makefile for Purasu
# 

SOURCES=boot.o main.o common.o interrupt.o gdt.o idt.o console.o pmm.o heap.o dev.o pit.o vfs.o

CPPFLAGS=-nostdlib -nostdinc -fno-builtin -fno-stack-protector -ffreestanding -fno-rtti -fno-exceptions -std=c++11
LDFLAGS=-Tlink.ld -Map kernel.map
ASFLAGS=-fcoff

all: purasu_fd.img

clean:
	-del *.o
	-del kernel
	-del purasu*.img

#
# FAT12 Floppy Image
#
purasu_fd.img : fdboot.bin stage2.bin $(SOURCES)
	ld $(LDFLAGS) -o kernel $(SOURCES)
	copy kernel Bootdisk\kernel
	copy stage2.bin Bootdisk\stage2.bin
	bfi -o=kernel -b=fdboot.bin -f=purasu_fd.img Bootdisk
	
fdboot.bin: fdboot.s
	nasm fdboot.s -o fdboot.bin
	
stage2.bin: stage2.s
	nasm stage2.s -o stage2.bin

.s.o:
	nasm $(ASFLAGS) $<
