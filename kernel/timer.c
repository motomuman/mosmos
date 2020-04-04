#include <stdint.h>
#include "nasmfunc.h"
#include "int.h"

void init_pit()
{
	/*     2bit           2bit         3bit    1bit
	 * |access counter| access method | mode | bcd |
	 */
	io_out8(0x0043, 0b00110100);

	//0x2e9c (11932) for 100Hz (interrupt every 10ms)
	io_out8(0x0040, 0x9c); // lower 8 bit
	io_out8(0x0040, 0x2e); // higher 8 bit

	register_interrupt(0, (uint32_t) asm_int_pit);
	return;
}
