#include <spede/flames.h>
#include <spede/machine/asmacros.h>
#include <spede/machine/io.h>
#include <spede/machine/pic.h>
#include <spede/machine/proc_reg.h>
#include <spede/machine/seg.h>
#include <spede/stdio.h>

#define LOOP 1666666

#define TIMER_INTR 32
#define PIC_MASK 0x21
#define MASK ~0x01
#define PIC_CONTROL 0x20
#define TIMER_DONE 0x60

void TimerEntry(void);

void TimerCode(void) {
	static unsigned count = 0;

	if(count % 50 == 0) cons_putchar('.');
	if (count % 300 == 0) cons_printf("Daniel Akhterov\n");

	outportb(PIC_CONTROL, TIMER_DONE);
	count++;
}

int main(void) {
	// 1
	char ch;
	struct i386_gate * intr_table;

	// 2
	intr_table = get_idt_base();

	// 3
	fill_gate(&intr_table[TIMER_INTR], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0);

	// 4
	outportb(PIC_MASK, MASK);

	// 5
	asm("sti");

	// 6
	while(1) {
		if(cons_kbhit()) {
			ch = cons_getchar();
			if(ch == 'b') {
				breakpoint();
			}
			cons_printf("Key %c is pressed...\n", ch);
		}

		// TimerCode();
	}

	// 7
	return 0;
}
