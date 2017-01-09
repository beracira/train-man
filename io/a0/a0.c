#include <bwio.h>
#include <ts7200.h>
#define FOREVER for( ; ; )

void printTime(int numTicks) {
	int ticksPerMin = 600;
	int ticksPerSec = 10;

	int minutes = numTicks / ticksPerMin;
	int seconds = (numTicks - (minutes * ticksPerMin)) / (ticksPerSec);
	int tenth = numTicks % ticksPerSec;

	bwprintf(COM2, "System elapsed time: ");

	if (minutes >= 10) {
		bwprintf(COM2, "%d:", minutes);
	} 
	else {
		bwprintf(COM2, "0%d:", minutes);
	}

	if (seconds >= 10) {
		bwprintf(COM2, "%d.", seconds);
	} 
	else {
		bwprintf(COM2, "0%d.", seconds);
	}

	bwprintf(COM2, "%d\n\r", tenth);
}

int comChar2Int(char * buffer, int start, int end){
	int base = end - start;
	int ret = 0;
	int i;
	int j;
	for (i = start; i <= end; i++) {
		int c = buffer[i] - '0';
		for (j = 0; j < base; j++) {
			c = c * 10;
		}
		ret = ret + c;
		base--;
	}
	return ret;
}

int doCommand(char * buffer, int index) {

	// tr <train--number> <train--speed>
	if (buffer[0] == 't' && buffer[1] == 'r') {
		int i = 3;
		while (buffer[i] >= '0' && buffer[i] <= '9') {
			i++;
		}
		int trainNum = comChar2Int(buffer, 3, i-1);

		i++;
		int j = i;

		while (buffer[j] >= '0' && buffer[j] <= '9') {
			j++;
		}
		int trainSpeed = comChar2Int(buffer, i, j-1);

		bwprintf(COM2, "Setting train %d's speed to %d.\n\r", trainNum, trainSpeed);

		bwputc(COM1, trainSpeed);
		bwputc(COM1, trainNum);
		return 0;
	}

	// rv <train--number>
	if (buffer[0] == 'r' && buffer[1] == 'v') {
		int i = 3;
		while (buffer[i] >= '0' && buffer[i] <= '9') {
			i++;
		}
		int trainNum = comChar2Int(buffer, 3, i);

		bwprintf(COM2, "Reversing train %d\n\r", trainNum);

		bwputc(COM1, 15);
		bwputc(COM1, trainNum);

		return 0;
	}

	// sw <switch--number> <switch--direction>
	if (buffer[0] == 's' && buffer[1] == 'w') {

		return 0;
	}

	// q
	if (buffer[0] == 'q') {

		return 1;
	}
	return 0;
}

int getc ( int channel, char * buffer, int size) {
	int *flags, *data;
	unsigned char c;

	switch( channel ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		return -1;
		break;
	}
	while (!( *flags & RXFF_MASK ));

	c = *data;

	while ( c != '\r' || c != '\n' ) {
		buffer[size] = c;
		size++;	
		c = *data;

	}
	buffer[size] = 0;
//	c = *data;
	return size;
}

int main( int argc, char* argv[] ) {

	// Initialization
	// Train controller
	bwsetfifo( COM1, OFF );
	bwsetspeed(COM1, 2400);
	// Monitor
	bwsetfifo( COM2, OFF );
	bwsetspeed(COM2, 115200);

	// Time
	// 32-bit timer
	int * timerLoad = (int *)(TIMER3_BASE + LDR_OFFSET);
	int * timerValue = (int *)(TIMER3_BASE + VAL_OFFSET);
	int * timerControl = (int *)(TIMER3_BASE + CRTL_OFFSET);

	// 508000 cycles/s = 50800 cycles/0.1s
	int cyclesPerTick = 50800;
	int numTicks = 0;
	int ticksBefore = numTicks;
	int ticksNow;

	// Load the timer
	// Disable first
	*timerControl = (*timerControl) ^ ENABLE_MASK;
	// Load initial value
	*timerLoad = cyclesPerTick;
	// Enable timer, periodic mode, 508 kHz clock
	*timerControl = ENABLE_MASK | MODE_MASK| CLKSEL_MASK ;

	// Command buffer
	char comBuffer[100];
	int comBufferIndex = 0;

	FOREVER {

		// User Input
		// comBufferIndex = bwgetc(COM2, comBuffer, 0);
		char c = bwgetc(COM2);
		if (c) {
			comBuffer[comBufferIndex] = c;
			comBufferIndex++;
			if (c == '\n' || c == '\r') {
				if (doCommand(comBuffer, comBufferIndex)) {
					return 0;
				}
				comBufferIndex = 0;
			}

		}

		// Time
		ticksNow = *timerValue;

		if (ticksNow > ticksBefore) {
			numTicks++;
			bwprintf(COM2, "\033[2J");
			printTime(numTicks);
			bwprintf(COM2, comBuffer);
		}



		ticksBefore = ticksNow;
	}
	return 0;
}

