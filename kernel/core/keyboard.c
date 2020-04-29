#include <stdint.h>
#include "print.h"
#include "list.h"
#include "memory.h"
#include "int.h"
#include "lib.h"
#include "asm.h"
#include "workqueue.h"
#include "task.h"

#define NULL 0

#define ASCII_NEW_LINE 0x0a
#define KEY_TABLE_SIZE 58

static char keytable[KEY_TABLE_SIZE] = {
	//00
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', 0, 0, 0,
	//10
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 0, 0, ASCII_NEW_LINE, 0, 'a', 's',
	//20
	'd', 'f', 'g', 'h', 'j', 'k', 'l', 0, ':', 0, 0, 0, 'z', 'x', 'c', 'v',
	//30
	'b', 'n', 'm', 0, '.', '/', 0, 0, 0, ' '};

#define KEY_BUF_LEN 1024

uint8_t key_buf[KEY_BUF_LEN];
int key_buf_read;
int key_buf_write;
int key_buf_size;
int key_cond;

int key_buf_empty()
{
	return key_buf_size == 0;
}

int key_buf_full()
{
	return key_buf_size == KEY_BUF_LEN - 1;
}

void key_buf_push(uint8_t key)
{
	if(!key_buf_full()) {
		key_buf[key_buf_write] = key;
		key_buf_write++;
		key_buf_write %= KEY_BUF_LEN;
		key_buf_size++;
	}
}

uint8_t key_buf_pop()
{
	uint8_t ret = 0;
	if(!key_buf_empty()) {
		ret = key_buf[key_buf_read];
		key_buf_read++;
		key_buf_size--;
		key_buf_read %= KEY_BUF_LEN;
	}
	return ret;
}

void int_keyboard() {
	uint8_t ch;

	ch = io_in8(0x0060);
	if(ch < KEY_TABLE_SIZE) {
		uint8_t key = keytable[ch];
		if(key != 0) {
			key_buf_push(key);
			task_wakeup(&key_cond);
		}
	}

	pic_sendeoi(KEYBOARD_IRQ);

	return;
}

uint8_t key_getc(int is_blocking)
{
	if(key_buf_empty()) {
		if(is_blocking) {
			task_sleep(&key_cond);
		} else {
			return 0;
		}
	}

	return key_buf_pop();
}

void init_keyboard()
{
	register_interrupt(KEYBOARD_IRQ, asm_int_keyboard);
	key_buf_read = 0;
	key_buf_write = 0;
	key_buf_size = 0;
	return;
}
