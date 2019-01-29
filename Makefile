CC = gcc
CFLAGS = -nostdlib -march=i686 -m32 -fno-pic -std=c11
INCLUDE = -I src/

default: build/milfa.img

build/milfa.img: build/ipl build/milfa.sys
	# 1440   1440K, double-sided, 18 sectors per track, 80 cylinders (for 3 1/2 HD)
	mformat -f 1440 -B build/ipl -C -i build/milfa.img ::
	mcopy build/milfa.sys -i build/milfa.img ::

build/ipl: src/ipl.nas
	nasm src/ipl.nas -o build/ipl -l build/ipl.lst

build/asmhead.o: src/asmhead.nas
	nasm src/asmhead.nas -o build/asmhead.o

build/bootpack.o: src/bootpack.c src/nasmfunc.nasm src/oslink.lds
	nasm -f elf32 src/nasmfunc.nasm -o build/nasmfunc.o
	gcc src/bootpack.c build/nasmfunc.o -T src/oslink.lds $(CFLAGS) $(INCLUDE) -o build/bootpack.o

build/milfa.sys: build/asmhead.o build/bootpack.o
	cat build/asmhead.o build/bootpack.o > build/milfa.sys

clean:
	rm -r build/*

run: build/milfa.img
	qemu-system-x86_64 -m 32 -fda build/milfa.img

.PHONY: run