#include <spede/stdio.h>
#include <spede/flames.h>

void DisplayMsg(int);

int main(void) {
	long i;
	i = 111;
	
	for(i; i <= 115; i++) {
		DisplayMsg(i);
	}

	return 0;
}

void DisplayMsg(int i) {
	printf("i=%d\n", i);
	cons_printf("i=%d\n", i);
}
