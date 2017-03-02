#include "a0io.h"
#include <ts7200.h>
#define FOREVER for( ; ; )

void bufferprinttime(controller * terminal, int numTicks){
	int ticksPerMin = 600;
	int ticksPerSec = 10;

	int minutes = numTicks / ticksPerMin;
	int seconds = (numTicks - (minutes * ticksPerMin)) / (ticksPerSec);
	int tenth = numTicks % ticksPerSec;

	bufferprintf(terminal, "System elapsed time: ");

	if (minutes >= 10) {
		bufferprintf(terminal, "%d:", minutes);
	} 
	else {
		bufferprintf(terminal, "0%d:", minutes);
	}

	if (seconds >= 10) {
		bufferprintf(terminal, "%d.", seconds);
	} 
	else {
		bufferprintf(terminal, "0%d.", seconds);
	}

	bufferprintf(terminal, "%d\n\r", tenth);
}

void bufferprintlooptime(controller * terminal, int ticksBefore, int ticksNow) {
	int ticks = ticksBefore + 50800 - ticksNow;
	bufferprintf(terminal, "Ticks in loop: %d\n\r", ticks);
}

char buffergetcommand( controller * channel ) {
	if(channel->inbufferfirst == channel->inbufferlast){
		return 0;
	}
	int i = channel->inbufferfirst;
	char c = channel->inbuffer[i];
	channel->inbufferfirst = (i + 1) % channel->insize;
	return c;
}

int comChar2Int(char * command, int start, int end){
	int base = end - start;
	int ret = 0;
	int i;
	int j;
	for (i = start; i <= end; i++) {
		int c = command[i] - '0';
		for (j = 0; j < base; j++) {
			c = c * 10;
		}
		ret = ret + c;
		base--;
	}
	return ret;
}

int docommand(controller * train, char * combuffer, int * combufferindex, char * switches){

	if(combuffer[0] == 't' && combuffer[1] == 'r' && combuffer[2] == ' '){
		int i = 3;
		while (combuffer[i] >= '0' && combuffer[i] <= '9') {
			i++;
		}
		int trainNum = comChar2Int(combuffer, 3, i-1);

		i++;
		int j = i;

		while (combuffer[j] >= '0' && combuffer[j] <= '9') {
			j++;
		}
		int trainSpeed = comChar2Int(combuffer, i, j-1);

		bufferputc(train, trainSpeed);
		bufferputc(train, trainNum); 
	}

	if(combuffer[0] == 'r' && combuffer[1] == 'v' && combuffer[2] == ' '){
		int i = 3;
		while (combuffer[i] >= '0' && combuffer[i] <= '9') {
			i++;
		}
		int trainNum = comChar2Int(combuffer, 3, i-1);

		bufferputc(train, 0);
		bufferputc(train, trainNum); 
		bufferputc(train, 15);
		bufferputc(train, trainNum); 
		bufferputc(train, 14);
		bufferputc(train, trainNum);
	}

	if(combuffer[0] == 's' && combuffer[1] == 'w' && combuffer[2] == ' '){
		int i = 3;
		while (combuffer[i] >= '0' && combuffer[i] <= '9') {
			i++;
		}
		int switchNum = comChar2Int(combuffer, 3, i-1);

		i++;
		char d = combuffer[i];
		int dir = 0;
		if (d == 'S' || d == 's') {
			dir = SW_STRAIGHT;
		} 
		if (d == 'C' || d == 'c') {
			dir = SW_CURVED;
		}

		switches[switchNum - 1] = d;

		bufferputc(train, dir); 
		bufferputc(train, switchNum);
		bufferputc(train, 32);
	}

	if(combuffer[0] == 'q'){
		return 1;
	}

	return 0;
}

void initSwitches(char * switches, controller * train) {
	int i;
	int s;
	int t;

	switches[0] = 'C';
	switches[1] = 'C';
	switches[2] = 'C';
	switches[3] = 'C';
	switches[4] = 'C';
	switches[5] = 'C';
	switches[6] = 'C';
	switches[7] = 'C';
	switches[8] = 'C';
	switches[9] = 'C';
	switches[10] = 'S';
	switches[11] = 'C';
	switches[12] = 'C';
	switches[13] = 'S';
	switches[14] = 'C';
	switches[15] = 'C';
	switches[16] = 'S';
	switches[17] = 'S';
	switches[18] = 'C';
	switches[19] = 'C';
	switches[20] = 'C';
	switches[21] = 'C';

	for (i = 0; i < 22; i++) {
		if (switches[i] == 'C') {
			t = SW_CURVED;
		}
		else{
			t = SW_STRAIGHT;
		}

		if (i == 18) {
			s = 0x99;
		}
		else if (i == 19) {
			s = 0x99A;
		}
		else if (i == 20) {
			s = 0x99B;
		}
		else if (i == 21) {
			s = 0x99C;
		}
		else {
			s = i + 1;
		}
		bufferputc(train, t);
		bufferputc(train, s);
		bufferputc(train, 32);
	}
}


int main( int argc, char* argv[] ) {

	controller terminal;
	terminal.com = COM2;
	terminal.speed = 115200;
	terminal.outbufferfirst = 0;
	terminal.outbufferlast = 0;
	terminal.inbufferfirst = 0;
	terminal.inbufferlast = 0;
	terminal.outsize = 1000;
	terminal.insize = 100;

	controller train;
	train.com = COM1;
	train.speed = 2400;
	train.outbufferfirst = 0;
	train.outbufferlast = 0;
	train.outsize = 1000;
	train.inbufferfirst = 0;
	train.inbufferlast = 0;
	train.insize = 100;

	char terminaloutbuffer[1000];
	char terminalinbuffer[100];
	char trainoutbuffer[train.outsize];
	char traininbuffer[train.insize];

	terminal.outbuffer = terminaloutbuffer;
	terminal.inbuffer = terminalinbuffer;

	train.outbuffer = trainoutbuffer;
	train.inbuffer = traininbuffer;

	setspeed(&terminal);
	setspeed(&train);

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

	// Commands
	char combuffer[100];
	int combufferindex = 0;
	int i;
	for (i = 0; i < 100; i++) {
		combuffer[i] = 0;
	}
	
	// Switches
	char switches[21];
	initSwitches(switches, &train);

	int diff = 0;

	FOREVER{
		ticksNow = *timerValue;
	//	controllerputc(&terminal);

		controllerputc(&train);
		//  Get any data that is available
		buffergetc(&terminal);
		buffergetc(&train);

		char com = buffergetcommand(&terminal);
		if(com){
			combuffer[combufferindex] = com;
			combufferindex++;
			combuffer[combufferindex] = 0;

			if(com == '\n' || com == '\r'){
				if(docommand(&train, combuffer, &combufferindex, switches)){
					bufferprintf(&terminal, "diff %d\n\r", diff);
					bwprintf(terminaloutbuffer);
					return 0;
				}

				for (i = 0; i < combufferindex; i++) {
					combuffer[i] = 0;
				}
				combufferindex = 0;
			}
		}

		if (ticksNow > ticksBefore) {
			numTicks++;
			bufferprintf( &terminal, "\033[2J\033[1;1H");
		//	bufferprintf( &terminal, "\033[1;1H");
	//		bufferprintf( &terminal, "\033[2J");
	//		bufferprintf( &terminal, "\033[1;1H"); //\033[2J\033[1;1H
			bufferprinttime(&terminal, numTicks); 
			bufferprintf(&terminal, "\n\r"); 
		//	bufferprintlooptime(&terminal, ticksNow);
		//	bufferprintswitches(&terminal, switches);

			bufferprintf(&terminal, "  Switch   Direction\n\r"); 
			bufferprintf(&terminal, "  ------------------\n\r");
			bufferprintf(&terminal, "  1        %c       \n\r", switches[0]);
			bufferprintf(&terminal, "  2        %c       \n\r", switches[1]);
			bufferprintf(&terminal, "  3        %c       \n\r", switches[2]);
			bufferprintf(&terminal, "  4        %c       \n\r", switches[3]);
			bufferprintf(&terminal, "  5        %c       \n\r", switches[4]);
			bufferprintf(&terminal, "  6        %c       \n\r", switches[5]);
			bufferprintf(&terminal, "  7        %c       \n\r", switches[6]);
			bufferprintf(&terminal, "  8        %c       \n\r", switches[7]);
			bufferprintf(&terminal, "  9        %c       \n\r", switches[8]);
			bufferprintf(&terminal, "  10       %c       \n\r", switches[9]);
			bufferprintf(&terminal, "  11       %c       \n\r", switches[10]);
			bufferprintf(&terminal, "  12       %c       \n\r", switches[11]);
			bufferprintf(&terminal, "  13       %c       \n\r", switches[12]);
			bufferprintf(&terminal, "  14       %c       \n\r", switches[13]);
			bufferprintf(&terminal, "  15       %c       \n\r", switches[14]);
			bufferprintf(&terminal, "  16       %c       \n\r", switches[15]);
			bufferprintf(&terminal, "  17       %c       \n\r", switches[16]);
			bufferprintf(&terminal, "  0x99     %c       \n\r", switches[17]);
			bufferprintf(&terminal, "  0x9A     %c       \n\r", switches[18]);
			bufferprintf(&terminal, "  0x9B     %c       \n\r", switches[19]);
			bufferprintf(&terminal, "  0x9C     %c       \n\r", switches[20]);
			bufferprintf(&terminal, "\n\r"); 
			bufferprintf( &terminal, combuffer);

			bwprintf(terminal.outbuffer);
			
		}
		
		if 	(ticksBefore - ticksNow > ticksBefore){
			diff = ticksBefore - ticksNow;
		}
	
		ticksBefore = ticksNow;

	}
	return 0;
}
