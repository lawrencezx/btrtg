CC = gcc
GPP= g++
LD=ld

# Physical address of service / kernel
# - Needed by the linker to set appropriate offsets (since our segment is 0x0)
SERVICE_LOCATION    = 0x8000

COPTS	= -O2 -Wall -c -g -m32 -fno-stack-protector -fno-defer-pop -fno-builtin -march=i386 -I.
LDOPTS = -nostartfiles -nostdlib -melf_i386 -N

all: bootloader service vmbuilder

bootloader: bootloader.asm
	nasm -f bin -g -o $@ $<

vmbuilder: vmbuilder.c
	$(CC) -o $@ $<

service: service.c
	$(CC) $(COPTS) $<
	$(LD) $(LDOPTS)	-Ttext $(SERVICE_LOCATION) service.o -o service

clean: 
	rm *.o
	rm service
	rm bootloader
	rm vmbuilder

