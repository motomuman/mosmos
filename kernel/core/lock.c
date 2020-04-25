
void mutex_init(uint64_t *mtx) {
	*mtx = 0;
}

void mutex_lock(uint64_t *mtx) {
	uint64_t rflags = get_rflags();
	if(rflags & 0x0020) {
		printstr_app("Tried to get lock in interruption\n");
		panic();
	}
	io_cli();
	while(test_and_set(mtx, 1)){
		task_sleep(mtx);
	}
	sti();
}

void mutex_unlock(uint64_t *mtx) {
	test_and_set(mtx, 0);
	task_wakeup(mtx);
}
