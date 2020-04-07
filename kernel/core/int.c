#include <stdint.h>
#include "nasmfunc.h"
#include "dsctbl.h"
#include "int.h"

uint8_t master_imr;
uint8_t slave_imr;

void init_pic(void)
{
	//master config
	master_imr = 0xff;
	io_out8(0x20, 0x11);
	io_out8(0x21, 0x20);
	io_out8(0x21, 0x04);
	io_out8(0x21, 0x05);
	io_out8(0x21, master_imr);

	//slave config
	slave_imr = 0xff;
	io_out8(0xa0, 0x11);
	io_out8(0xa1, 0x28);
	io_out8(0xa1, 0x02);
	io_out8(0xa1, 0x01);
	io_out8(0xa1, slave_imr);
}

void pic_clearmask(int irq)
{
	if(irq < 8) {
		master_imr &= ~(1<<irq);
		io_out8(0x21, master_imr);
	} else {
		master_imr &= ~(1<<2);
		io_out8(0x21, master_imr);

		slave_imr &= ~(1<<(irq-8));
		io_out8(0xa1, slave_imr);
	}
}

void register_interrupt(int irq, uint32_t handler) {
	pic_clearmask(irq);
	set_idt(irq + 0x20, handler);
}

void init_interrupt()
{
	init_pic();
	register_interrupt(KEYBOARD_IRQ, (uint32_t) asm_int_keyboard);
}

void pic_sendeoi(int irq)
{
	if(irq >= 8){
		io_out8(0xa0, 0x20);
	}
	io_out8(0x20, 0x20);
}
