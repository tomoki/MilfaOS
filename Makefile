
default: build/milfa.img


build/milfa.img: build/ipl.bin
	cp build/ipl.bin build/milfa.img

build/ipl.bin: src/ipl.nas
	nasm src/ipl.nas -o build/ipl.bin -l build/ipl.lst

clean:
	rm -r build/

img: build/ipl.bin

run: build/milfa.img
	qemu-system-x86_64 -m 32 -fda build/milfa.img

.PHONY: run