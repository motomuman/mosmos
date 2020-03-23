all:
	make -C boot boot.o
	make -C kernel kernel.o

	cp boot/boot.o mosmos.img
	dd if=kernel/kernel.o		of=mosmos.img bs=1 seek=8192  conv=notrunc

clean:
	make -C boot clean
	make -C kernel clean
