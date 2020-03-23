ASM = nasm

kernel.o: kernel.s
	$(ASM) kernel.s -o kernel.o -l kernel.lst

clean:
	rm -rf *.img *.lst *.o
