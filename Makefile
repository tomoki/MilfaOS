
default: build/milfa.img


build/milfa.img: build/ipl
	cp build/ipl build/milfa.img

build/ipl: src/ipl.nas
	nasm src/ipl.nas -o build/ipl -l build/ipl.lst

clean:
	rm -r build/*

run: build/milfa.img
	qemu-system-x86_64 -m 32 -fda build/milfa.img

.PHONY: run