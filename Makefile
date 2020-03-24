all:
	make -C boot boot
	make -C kernel kernel

	cp boot/boot mosmos.img
	dd if=kernel/kernel		of=mosmos.img bs=1 seek=8192  conv=notrunc

clean:
	make -C boot clean
	make -C kernel clean
