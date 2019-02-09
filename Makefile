CC = gcc
CFLAGS = -nostdlib -march=i686 -m32 -fno-pic -std=c11 -fno-stack-protector
INCLUDE = -I src/ -I src/libc
LDFLAGS = -nostdlib -march=i686 -m32 -fno-pic -std=c11

BOOTPACK_OBJS =  build/bootpack.o build/libc.o build/nasmfunc.o build/int.o build/hankaku.o build/descriptors.o build/graphics.o build/mouse.o

default: build/milfa.img

# 1440   1440K, double-sided, 18 sectors per track, 80 cylinders (for 3 1/2 HD)
build/milfa.img: build/ipl build/milfa.sys
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

build/%.o: src/%.c
	$(CC) src/$*.c $(CFLAGS) $(INCLUDE) -c -o build/$*.o

build/bootpack.mil: $(BOOTPACK_OBJS)
	$(CC) $(BOOTPACK_OBJS) -T src/oslink.lds -o build/bootpack.mil $(LDFLAGS)

build/milfa.sys: build/asmhead.o build/bootpack.mil
	cat build/asmhead.o build/bootpack.mil > build/milfa.sys

src/hankaku.h: tools/makefont.py data/hankaku.txt
	python tools/makefont.py data/hankaku.txt > src/hankaku.h

clean:
	rm -r build/*

# Instead of qemu-system-x86_64 -m 32 -fda build/milfa.img, use this format to suppress warning
run: build/milfa.img
	qemu-system-x86_64 -m 32 -drive format=raw,file=build/milfa.img,index=0,if=floppy


run-windows: build/milfa.img
	/mnt/c/Program\ Files/qemu/qemu-system-x86_64.exe -m 32 -drive format=raw,file=build/milfa.img,index=0,if=floppy

.PHONY: run
