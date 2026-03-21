ENTRY(start_32)
SECTIONS
{
    . = 0x8000;
    .text : { *(.text*) }
    .data : { *(.data*) }
    .bss  : { *(.bss*) }
    . = ALIGN(51200);
}
