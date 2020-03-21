ASM = nasm

default: mosmos.img
	make mosmos.img

ipl.o: ipl.s
	$(ASM) ipl.s -o ipl.o -l ipl.lst

bootmon.o: bootmon.s
	$(ASM) bootmon.s -o bootmon.o -l bootmon.lst

mosmos.img: ipl.o bootmon.o
	cp ipl.o mosmos.img
	dd if=bootmon.o		of=mosmos.img bs=1 seek=512 conv=notrunc

clean:
	rm -rf *.img *.lst *.o
