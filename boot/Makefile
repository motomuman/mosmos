ASM = nasm

default: mosmos.img
	make mosmos.img

boot.o: boot.s
	$(ASM) boot.s -o boot.o -l boot.lst

mosmos.img: boot.o
	cp boot.o mosmos.img

clean:
	rm -rf *.img *.lst *.o
