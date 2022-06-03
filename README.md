# 8086 Assembler

Convert assembly code files directly to bin(com) file

## Here are a few examples:

###### Simple bootloader

```
	%BOOTLOADER_SIZE = 512

	org 7C00h
	
	mov BOOT_DISK, dl

	mov bp, 7C00h
	mov sp, bp

	jmp_short -2

	BOOT_DISK: db 0
	
	db ((BOOTLOADER_SIZE - 2) - #) @ 0 
	
	db 55h, 0AAh
```

###### DOS Hello World

```
	mov ah, 9h
	mov dx, &message
	int 21h
	
	message: db 'Hello World!$'
```

###### Self-modifying code

Changes operand in "mov dl" instruction

```
	mov ah, [bx - 2]
	mov ah, 2h
	mov [$ + 7], 'B'
	mov dl, 'A'
	int 21h
```