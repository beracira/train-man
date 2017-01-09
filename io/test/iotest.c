 /*
 * iotest.c
 */

#include <bwio.h>
#include <ts7200.h>

int main( int argc, char* argv[] ) {

	int * timer_ldr = (int *)(TIMER3_BASE + LDR_OFFSET);
	int * timer_val = (int *)(TIMER3_BASE + VAL_OFFSET);
	int * timer_ctrl = (int *)(TIMER3_BASE + CRTL_OFFSET);

	bwprintf( COM2, "timer ldr: %d, timer val: %d, timer ctrl: %d\n\r", *timer_ldr, *timer_val, *timer_ctrl );

	char str[] = "Hello\n\r";
	bwsetfifo( COM2, OFF );
	bwputstr( COM2, str );
	bwputw( COM2, 10, '*', str );
	bwprintf( COM2, "Hello world.\n\r" );
	bwprintf( COM2, "%s world%u.\n\r", "Well, hello", 23 );
	bwprintf( COM2, "%d worlds for %u person.\n\r", -23, 1 );
	bwprintf( COM2, "%x worlds for %d people.\n\r", -23, 723 );
	str[0] = bwgetc( COM2 );
	bwprintf( COM2, "%s", str );

	
	bwprintf( COM2, "timer ldr: %d, timer val: %d, timer ctrl: %d\n\r", *timer_ldr, *timer_val, *timer_ctrl );


	return 0;
}

