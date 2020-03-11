ASM = nasm

default: mosmos.img
	make mosmos.img

ipl.o: ipl.asm
	$(ASM) ipl.asm -o ipl.o -l ipl.lst

mosmos.img: ipl.o
	cp ipl.o mosmos.img

clean:
	rm -rf *.img *.lst *.o
