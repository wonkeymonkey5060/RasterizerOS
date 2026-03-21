[bits 32]


extern c_main


global start_32


global setPixel
global fillScreen
global drawBuffer
global tan
global clearDepth


%define frontBuffer   0x540000
%define backBuffer  0x00570000
%define depthBuffer 0x08000000
section .text



start_32:
    


    mov ax, 0x10        ; Correct data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    mov esp, 0x10000    ; Set up stack


    mov eax, cr0
    and eax, 0x9FFFFFFF    ; clear CD (bit 30)
    and eax, 0xDFFFFFFF    ; clear NW (bit 29)
    mov cr0, eax
    wbinvd                 ; flush caches

    
    ; Enable FPU/SSE
    mov eax, cr0
    and ax, 0xFFFB      ; Clear EM (emulation) bit
    or ax, 0x2          ; Set MP (monitor coprocessor) bit
    mov cr0, eax

    mov eax, cr4
    or ax, 3 << 9       ; Set OSFXSR and OSXMMEXCPT bits for SSE
    mov cr4, eax
    fninit              ; Initialize FPU
    
    ; Test: Write directly to framebuffer
    mov ebx, [frontBuffer]           ; Get framebuffer address
    mov dword [ebx], 0xFF0000           ; Red pixel
    mov dword [ebx+4], 0x00FF00    ; Green pixel
    mov dword [ebx+80], 0x0000FF    ; Blue pixel
    
    cld
    and esp, 0xFFFFFFF0   ; aligns esp to 16 bytes
    
    mov ecx, depthBuffer   ;  first parameter
    
    call c_main

setPixel:  ; linear offset in ecx, hex color code in edx
    push ebx
    mov ebx, backBuffer ;[frontBuffer]
    imul ecx, 3
    add ebx, ecx
    mov dword [ebx], edx
    pop ebx
    ret

fillScreen:  ; color in ecx
    cld
    mov edi, backBuffer ; edi = destination
    mov ecx, 480000  ; ecx = number of loops for LOOP instruction, not for REP.


    mov eax, 0x00000000 ; store "blue" in [EDI], and add 1 to EDI

    rep stosd  ; jump to .fill and subtract from ecx, unless ecx = 0
    
    ret
       ; returns if ecx = 0   
clearDepth:
    push edi
    cld
    mov edi, ecx
    mov ecx, 100000  ; ecx = number of loops

    mov eax, 0x0FFFFFFF ; store highest possible positive 32 bit signed integer

    rep stosd  ; fill
    pop edi
    ret
drawBuffer:
    push esi
    push edi
    cld  ;clear direction flag
    mov ecx, 480000  ;  ecx starting point. when it reaches 0, loop stops
    mov esi, backBuffer ;esi = source

    mov edi, [frontBuffer]  ;edi = destination

    rep movsd

    pop edi
    pop esi
    ret

tan:
    push ebp
    mov ebp, esp
    sub esp, 8
    fld dword [ebp+8]
    fptan
    fstp st0
    mov esp, ebp
    pop ebp
    ret


testcall:
    ret


