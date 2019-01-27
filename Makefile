default: build/milfa.img

build/milfa.img: build/ipl
	# 1440   1440K, double-sided, 18 sectors per track, 80 cylinders (for 3 1/2 HD)
	mformat -f 1440 -B build/ipl -C -i build/milfa.img ::

build/ipl: src/ipl.nas
	nasm src/ipl.nas -o build/ipl -l build/ipl.lst

clean:
	rm -r build/*

run: build/milfa.img
	qemu-system-x86_64 -m 32 -fda build/milfa.img

.PHONY: run