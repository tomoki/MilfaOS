CC = gcc
CFLAGS = -nostdlib -march=i686 -m32 -fno-pic -std=c11
INCLUDE = -I src/ -I src/libc
LDFLAGS = -nostdlib -march=i686 -m32 -fno-pic -std=c11

default: build/milfa.img

build/milfa.img: build/ipl build/milfa.sys
	# 1440   1440K, double-sided, 18 sectors per track, 80 cylinders (for 3 1/2 HD)
	mformat -f 1440 -B build/ipl -C -i build/milfa.img ::
	mcopy build/milfa.sys -i build/milfa.img ::

build/ipl: src/ipl.nas
	nasm src/ipl.nas -o build/ipl -l build/ipl.lst

build/asmhead.o: src/asmhead.nas
	nasm src/asmhead.nas -o build/asmhead.o

build/nasmfunc.o: src/nasmfunc.nasm
	nasm -f elf32 src/nasmfunc.nasm -o build/nasmfunc.o

build/libc.o: src/libc/libc.c
	$(CC) src/libc/libc.c $(CFLAGS) $(INCLUDE) -c -o build/libc.o

build/bootpack.o: src/bootpack.c
	$(CC) src/bootpack.c $(CFLAGS) $(INCLUDE) -c -o build/bootpack.o

build/bootpack.mil: build/bootpack.o build/libc.o build/nasmfunc.o
	$(CC) build/bootpack.o build/libc.o build/nasmfunc.o -T src/oslink.lds -o build/bootpack.mil $(LDFLAGS)

build/milfa.sys: build/asmhead.o build/bootpack.mil
	cat build/asmhead.o build/bootpack.mil > build/milfa.sys

src/hankaku.h: tools/makefont.py data/hankaku.txt
	python tools/makefont.py data/hankaku.txt > src/hankaku.h

clean:
	rm -r build/*

run: build/milfa.img
	qemu-system-x86_64 -m 32 -fda build/milfa.img

.PHONY: run
