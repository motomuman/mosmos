ASM = nasm

boot.o: boot.s
	$(ASM) boot.s -o boot.o -l boot.lst

clean:
	rm -rf *.img *.lst *.o
