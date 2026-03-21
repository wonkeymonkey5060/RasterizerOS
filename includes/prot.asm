
align 8
gdt_start:
    dq 0x0              ; Null descriptor (8 bytes of 0)
gdt_code:               ; Code segment descriptor
    dw 0xffff           ; Limit (bits 0-15)
    dw 0x0              ; Base (bits 0-15)
    db 0x0              ; Base (bits 16-23)
    db 10011010b        ; Access byte (Present, Ring 0, Code, Readable)
    db 11001111b        ; Flags & Limit (bits 16-19)
    db 0x0              ; Base (bits 24-31)
gdt_data:               ; Data segment descriptor
    dw 0xffff           ; Limit (bits 0-15)        - 2 bytes
    dw 0x0              ; Base (bits 0-15)         - 2 bytes
    db 0x0              ; Base (bits 16-23)        - 1 byte
    db 10010010b        ; Access byte              - 1 byte
    db 11001111b        ; Flags & Limit (16-19)    - 1 byte
    db 0x0             ; Base (bits 24-31)        - 1 byte
                        ; Total: 8 bytes
gdt_end:

align 8
gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size (Limit)
    dd gdt_start               ; Address (Base)

enter_protected:

    cli                 ; Disable standard interrupts
    in al, 0x70         ; Read CMOS index port
    or al, 0x80         ; Set the top bit to disable NMI
    out 0x70, al        ; Write back
    
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1          ; Set PE (Protected Mode Enable) bit
    mov cr0, eax
    
    cli
    
    jmp 0x08:0x8000  ; 0x08 is the offset to gdt_code


