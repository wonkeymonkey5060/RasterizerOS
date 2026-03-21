[org 0x7E00]
[bits 16]




jmp kernel_entry


%define frontBuffer 0x540000
%define backBuffer  0x590000

mode_info_block: times 256 db 0


kernel_entry:
	
	mov ax, 0x0E41      ; print 'A' immediately
	mov bh, 0
	int 0x10
	;jmp $
	; then try VBE setup
	mov ax, 0x4F02
	mov bx, 0x4115
	int 0x10

	; then print 'B' after VBE
	mov ax, 0x0E42
	mov bh, 0
	int 0x10

	
    mov ax, 0x4F01      ; Get Mode Info
    mov cx, 0x115       ; Must match the mode set
    mov di, mode_info_block
    int 0x10

    cmp ax, 0x004F
	jne vbe_failed
	

	mov eax, [mode_info_block + 0x28] ; address of linear frame buffer
	mov ebx, frontBuffer
	mov [ebx], eax
	


	jmp enter_protected  ;	creates gdt and jumps to start_32  
						 ; defined in start32.asm
	
vbe_failed:
    mov ax, 0x0E41      ; int 0x10 function 0x0E = write char in TTY mode
    mov bh, 0           ; page 0
    int 0x10            ; this will print 'A' to the screen
    
    jmp $               ; infinite loop



;contains "enter_protected" aswell as gdt:
%include "prot.asm"  

;contains some optimized rendering calls for use throughout the program:
%include "render.asm"


times 512-($-$$) db 0