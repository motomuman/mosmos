OUTPUT_FORMAT("binary");
OUTPUT_ARCH("i386");
ENTRY(kentry)

SECTIONS
{
	. = 0x00101000;
	.text : { *(.text) }
	.data : { *(.data) }
}
