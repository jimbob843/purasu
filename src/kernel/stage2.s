;
; JamesM Tutorial COFF Loader
;
; File:				stage2.s
; Description:		Boot stage 2
;					Uses unreal mode to load a COFF kernel
; Author:			James Smith
; Created:			14-Oct-2012
;

[BITS 16]		; Real mode
[ORG 0x9000]	; Standard boot block load address

;
; MEMORY MAP
;
; 0000:0000 Interrupt Vector Table (IVT)
; 0000:0400 BIOS data area
; 0000:0500 FAT storage area
; 0000:4500 FREE
; 0000:7C00	Bootblock
; 0000:7E00 FREE
; 0000:8000 Boot stack (4KB)
; 0000:9000 STAGE TWO  (604KB available)
; A000:0000 VGA Video RAM
; C000:0000 BIOS ROM
; F000:FFFF Free memory starting at 0x100000
;


;
; Stage2 Code Start
;
stage2start:
	
	mov [bootdrive], dl	; Store boot drive

	mov ax, 0xB800
	mov es, ax
	mov byte [ES:0x0000], 'R'
	mov byte [ES:0x0001], 0x07

;
; Setup UNREAL mode
;
   cli                    ; no interrupts
   push ds                ; save real mode
 
   lgdt [gdt_pointer]         ; load gdt register
 
   mov  eax, cr0          ; switch to pmode by
   or   al,1              ; set pmode bit
   mov  cr0, eax
 
   mov  bx, 0x10          ; select descriptor 1
   mov  ds, bx            ; 8h = 1000b
   mov  es, bx
 
   and  al,0xFE           ; back to realmode
   mov  cr0, eax          ; by toggling bit again
 
   pop ds                 ; get back old segment
   sti
   
   xor ax, ax
   mov ds, ax
   mov es, ax
 
   mov bx, 0x0f01         ; attrib/char of smiley
   mov eax, 0x0b8000      ; note 32 bit offset
   mov word [ds:eax], bx

	jmp LoadStageTwo
	
 
gdtinfo:
   dw gdt_end - gdt - 1   ;last byte in table
gdtaddr:
   dd gdt                 ;start of table
 
gdt         dd 0,0        ; entry 0 is always unused
flatdesc    db 0xff, 0xff, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:



	mov [bootdrive], dl	; Store boot drive
	
;
; Stage Two Boot Load
;   - Load the FAT and FAT12 directory from disk to 0x00500 (sectors 1-33)
;   - Locate the KERNEL.BIN in the root dir
;   - Load the file at 0x09000. (Gives 604k of space from 0x09000->0xA0000)
;
; NB: To save time and space, this code assumes a normal 1.44MB floppy
;     with 512 bytes per sector.
;
LoadStageTwo:
	; FAT and directory already loaded at 0x0500
	;  Continue with search for kernel

	xor ebx, ebx
	mov esi, ebx
	mov edi, ebx
	
	; All sectors loaded now, scan the directory table for the file.
	mov bh, 0x29		; Hard-coded start of FAT12 directory table. (BX=0x2900)
	mov si, KernelBin	; String to search for
	mov di, bx			; Start address of search
	mov cl, 224			; File entries per dir.
	

;
; StringCompare
;
; SI - string 1
; DI - string 2
; returns DX : 0 = no match, 1 = match
;
StringCompare:
	push cx
	mov cx, 11		; Assume length of the filename we are looking for
	xor dx, dx		; Init DX to not found
	cld				; Set direction to up
	push di
	push si
.compareloop2:
	cmpsb			; Compare [SI] with [DI]
	jnz .exitcompare	; ZF set if character matches
	loop .compareloop2	; do the next one
 
	inc dx			; Match found, set return = 1
.exitcompare:
	pop si
	pop di
	pop cx

	cmp dx, 1
	jz .foundentry
	
	; No entry found, move to next entry
	add di, 32			; 32 bytes per entry
	add bx, 32
	loop StringCompare
	
	; String not found in any of the entries, print message and stop
	mov si, NoSecondStageMsg
	call WriteStr
	
	cli
	hlt		; STOP
	
.foundentry:
	; BX currently points to the directory entry we want
	mov ax, [bx+26]		; Get the first cluster number
						; Assume SectorsPerCluster = 1
						
	; Now load the file into memory at 0x100000
	mov ebx, 0x100000
loadnextsector:
	push ebx
	xor ebx, ebx
	mov bx, 0x4500		; Fixed load addr
	push ax
	add ax, 31			; First cluster is (32-1) sectors from the start of disk
	call LoadLBA
	
	pop ax
	
; Now the loaded data to it's new home
	pop ebx	
	
_MEMCPY:
	push esi
	push edi
	push ecx
	push eax
	
	; Copy the main block using DWORDs
	mov esi, 0x00004500		; Load source address
	mov edi, ebx			; Load destination address
	mov ecx, 0x200			; Load number of bytes to copy
	shr ecx, 2				; Convert to whole DWORDS
	
memcpyloop:
	mov eax, [esi]
	mov [edi], eax
	add esi, 4
	add edi, 4
	loop memcpyloop
		
	pop eax
	pop ecx
	pop edi
	pop esi
	
	add ebx, 0x200			; Next block
	
;
; GetSector - Finds the next cluster from the FAT
;  AX - current cluster no., returns next cluster number in AX
;
GetSector:
	push ebx
	push cx
	push dx

	mov cx, ax		; save AX for later

	mov dx, 3		; 3 nibbles per FAT entry
	mul dx			; Calculate offset in nibbles
	shr ax, 1		; Convert to bytes  (/2)
	add ax, 0x0500	; Start of FAT
	mov bx, ax
	mov ax, [bx]	; Get both bytes  BC DA => DABC (even), CD AB => ABCD (odd)

	bt cx, 0		; Check to see if CX (input cluster num) is even
	jc .oddFAT
.evenFAT:
	and ax, 0x0FFF	; Mask off the bits we don't want => 0ABC
	jmp .sectorexit

.oddFAT:
	shr ax, 4		; Move the value (ABCD) down to the right place => 0ABC

.sectorexit:
	pop dx
	pop cx
	pop ebx
; end GetSector

;
; Increment dest pointer and loop for next cluster
;

	cmp ax, 0x0FFF		; Check for end of FAT chain
	jnz loadnextsector



;
; Stop the floppy motors
;
StopFloppyMotors:
	mov dx, 0x3F2			; Controller A
	mov al, 0x0C			; Motors=OFF, DMA/IRQ=ON, Controller=ON
	out dx, al
	mov dx, 0x372			; Controller B
	mov al, 0x0C			; Motors=OFF, DMA/IRQ=ON, Controller=ON
	out dx, al
	
	
;
; Enter Protected Mode
;
EnterPMode:
	cli						; Stop interrupts
	lgdt [gdt_pointer]		; Load GDT

	mov eax, cr0
	or al, 1				; Set bit0 of cr0
	mov cr0, eax

	jmp 0x08:Trampoline


;
; WriteStr
;
; [input] si = addr of string
;
WriteStr:
	mov ah,0x0E		; Function (teletype)
	mov bx,0x0007	; Page 0, White text

.nextchar:
	lodsb			; Loads [SI] into AL and increases SI by one
	or al, al		; check al = 0
	jz .return
	int 0x10		; BIOS print char
	jmp .nextchar

.return:
	ret
	

;
; LoadLBA
;   AX=LBA sector number
;   ES:BX=Dest addr
;
LoadLBA:
	push dx
	push cx
	push ax			; Save trashed registers
	push bx			; Save dest addr for later

	xor dx, dx
	mov bx, 18		; Sectors per track
	div bx			; ax/bx -> ax,dx
	inc dx
	mov cx, dx		; Put sector count into CL

	xor dx, dx
	mov bx, 2		; Number of Heads
	div bx			; AL=Cylinder, DL=Head, CL=Sector

 ; Now do the load
	mov ch, al		; Put the cylinder (track) into CH
	shl dx, 8		; Shift head into DH
	mov byte dl, [bootdrive]	; Get booting drive number
	mov ax, 0x0201  ; BIOS read, one sector
	pop bx			; Get the address to put the data that we saved earlier
	int 0x13		; Do it.

	; Carry flag set if there is an error. Code in AH.
	jc FloppyError
	
	pop ax
	pop cx
	pop dx
	ret
	
FloppyError:
	; Something went wrong. Report the error code from AH.
	push ax
	mov si, FloppyErrMsg
	call WriteStr
	pop ax
	
	; Print the error code
	rol ax, 4
	call PrintDigit
	rol ax, 4
	call PrintDigit
	
	cli
	hlt				; STOP

;
; Prints a single hexadecimal digit to the screen
;
PrintDigit:
	push ax
	and al, 0x0F		; Mask the nibble we want
	cmp al, 9
	jle .nothexdigit
	add al, 0x07		; Shift up to the letter A-F
.nothexdigit:
	add al, 0x30		; Convert to ASCII
	mov ah, 0x0E		; Function (teletype)
	mov bx, 0x0007		; Page 0, White text
	int 0x10			; Call BIOS
	pop ax
	ret
 
;
; KeyboardWait - wait for keyboard controller to be ready
;
KeyboardWait:
	in al, dx
	and al, 2			; Check bit1 of controller status
	jnz KeyboardWait	; Loop until 0
	ret


;
; KeyboardWaitData - wait for keyboard controller to be ready
;
KeyboardWaitData:
	in al, dx
	and al, 1				; Check bit0 of controller status
	jz KeyboardWaitData		; Loop until 1
	ret


;
; GDT (This is just a temporary one to switch to PMode)
;
gdt_start:
 dw 0x0000	; NULL selector
 dw 0x0000
 dw 0x0000
 dw 0x0000

 dw 0xFFFF	; CODE selector, Base=0000, Len=4GB
 dw 0x0000	; Base
 dw 0x9A00	; Present, DPL=0, DT=App, CODE, Execute/Read, Non-conforming
 dw 0x00CF	; G=4K, D=32bit, Seg high nibble = 0xF

 dw 0xFFFF	; DATA selector, Base=0000, Len=4GB
 dw 0x0000	; Base
 dw 0x9200	; Present, DPL=0, DT=App, DATA, Read/Write, Expand-up
 dw 0x00CF	; G=4K, D=32bit, Seg high nibble = 0xF

gdt_pointer:
 dw gdt_pointer - gdt_start			; Calculate length of GDT (WHY -1????)
 dd gdt_start


; Boot drive
bootdrive:
 db 0x00


KernelBin			db 'KERNEL     '	; 8.3 filename of the second stage
NoSecondStageMsg	db 'No kernel!',13,10,0
FloppyErrMsg		db 'FDC Err 0x',0


[BITS 32]
Trampoline:
						; Now in protected mode
	mov eax, 0x10		; Set to DATA segment (0x10)
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax

	xor eax, eax
	xor ebx, ebx
	xor ecx, ecx
	xor edx, edx
	xor esi, esi
	xor edi, edi

	call 0x101000
	
mainloop:
	hlt
	jmp mainloop
	
	cli
	hlt	


 