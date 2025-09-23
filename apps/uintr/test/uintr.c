#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "uintr.h"


const char *test_info = "[MAIN] N-extension u-timer interrupt test\n";

void exit_function(void) {
	printf("[MAIN] u-timer interrupt test finished\n");
}

int main(void) {
	atexit(exit_function);

	printf(test_info);

    prepare_u_intr();
	csr_write(0x045,0);

	for (int i=0;i<10;i++){
		printf("[U_INTR_HANDLER] set timer for %d time(s).\n",i);
		csr_write(0x045, 1000000);
		uint64_t count;
		while ((count = csr_read(0x045)) != 0); //printf("%d\n",count);
	}

	clear_u_intr();
	return 0;
}