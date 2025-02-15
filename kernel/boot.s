;; Copyright © 2018-2019 N. Van Bossuyt.                                      ;;
;; This code is licensed under the MIT License.                               ;;
;; See: LICENSE.md                                                            ;;

;; boots.s: the HJERT kernel entry point from the bootloader.                 ;;

;; --- multiboot header. ---------------------------------------------------- ;;

MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
FLAGS    equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + FLAGS)   ; checksum of above, to prove we are multiboot
 
; Declare a multiboot header that marks the program as a kernel.
section .multiboot
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
 
;; --- kernel stack --------------------------------------------------------- ;;

section .bss
align 16

global __stack_bottom
global __stack_top
__stack_bottom:
resb 16384 ; 16 KiB
__stack_top:

;; --- kernel entry point --------------------------------------------------- ;;

section .text
global _kstart:function (_kstart.end - _kstart)
_kstart:
	cld
	; To set up a stack, we set the esp register to point to the top of our
	; stack (as it grows downwards on x86 systems).
	mov esp, __stack_top
 
	push eax ; Push the multiboot magic
	push ebx ; Push the multiboot header adress.
	
	extern kmain
	call kmain

	cli
.hang:	hlt
	jmp .hang
.end: