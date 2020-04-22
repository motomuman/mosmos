#include "user_syscall.h"

void userland_main() {
	int count = 0;
	int i;
	while(1) {
		for(i = 0; i < 200000000; i++){
		}
		int ret = sys_print("user: syscall");

		//remove this. directly call kernel func
		printstr_app("user: syscall ret = ");
		printnum_app(ret);
		printstr_app("\n");
		count++;
		if(count == 10){
			io_cli(); //trigger General Protection Fault
		}
	}
}

