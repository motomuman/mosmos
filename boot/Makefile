ASM = nasm

boot: boot.s
	$(ASM) boot.s -o boot -l boot.lst

clean:
	rm -rf *.img *.lst *.o boot
