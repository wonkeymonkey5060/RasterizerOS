#!/bin/bash
nasm -f bin "boot.asm" -o "build/boot.bin"
nasm -f bin kernel/kernel.asm -I./includes/ -o build/kernel.bin



nasm -f elf32 main/start32.asm -I./includes/ -o build/ofiles/start32.o

i686-elf-gcc -ffreestanding -nostdlib -nostartfiles -nodefaultlibs -c \
  main/main.c -o build/ofiles/main.o





# Link to kernel.elf
i686-elf-ld -m elf_i386 -T link.ld \
  build/ofiles/start32.o \
  build/ofiles/main.o \
  -o build/main.elf


#objdump -t build/main.elf | grep cpp_main
#objdump -t build/main.elf | grep start_32


objcopy --pad-to 0x9000 -O binary build/main.elf build/main.bin

cat build/boot.bin build/kernel.bin build/main.bin > RasterizerOS.bin


qemu-system-x86_64 -drive file=RasterizerOS.bin,format=raw,media=disk,index=0