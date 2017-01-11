
#include <ts7200.h>
#include "a0io.h"

int setfifo( controller * channel, int state ) {
	int *line, buf;
	switch( channel->com ) {
	case COM1:
		line = (int *)( UART1_BASE + UART_LCRH_OFFSET );
		buf = *line;
		buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
		buf = (((buf | STP2_MASK) | WLEN_MASK) & (~PEN_MASK)) & (~BRK_MASK);
	    break;
	case COM2:
	    line = (int *)( UART2_BASE + UART_LCRH_OFFSET );
		buf = *line;
		buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
	    break;
	default:
		return -1;
	    break;
	}
	*line = buf;
	return 0;
}

int setspeed( controller * channel) {
	int *high, *low;
	switch( channel->com ) {
	case COM1:
		high = (int *)( UART1_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART1_BASE + UART_LCRL_OFFSET );
	        break;
	case COM2:
		high = (int *)( UART2_BASE + UART_LCRM_OFFSET );
		low = (int *)( UART2_BASE + UART_LCRL_OFFSET );
	        break;
	default:
	    return -1;
	    break;
	}
	switch( channel->speed ) {
	case 115200:
		*high = 0x0;
		*low = 0x3;
		break;
	case 2400:
		*high = 0x0;
		*low = 0xBF;
		break;
	default:
	    return -1;
	    break;
	}
	setfifo( channel, OFF);
	return 0;
}



int bufferputc( controller * channel, char c ) {
	int i = channel->outbufferlast;
	channel->outbuffer[i] = c;
	channel->outbufferlast = (i + 1) % channel->outsize;
	return 0;
}

char c2x( char ch ) {
	if ( (ch <= 9) ) return '0' + ch;
	return 'a' + ch - 10;
}

int bufferputx( controller * channel, char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	bufferputc( channel, chh );
	return bufferputc( channel, chl );
}

int bufferputr( controller * channel, unsigned int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) bufferputx( channel, ch[byte] );
	return bufferputc( channel, ' ' );
}

int bufferputstr( controller * channel, char *str ) {
	while( *str ) {
		if( bufferputc( channel, *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void bufferputw( controller * channel, int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) bufferputc( channel, fc );
	while( ( ch = *bf++ ) ) bufferputc( channel, ch );
}

int buffergetc( controller * channel ) {
	int *flags, *data;

	switch( channel->com ) {
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

	if( (*flags & RXFF_MASK )){
		int i = channel->inbufferlast;
		channel->inbuffer[i] = *data;
		channel->inbufferlast = (i + 1) % channel->insize;
	}
	return 0;
}

int bwa2d( char ch ) {
	if( ch >= '0' && ch <= '9' ) return ch - '0';
	if( ch >= 'a' && ch <= 'f' ) return ch - 'a' + 10;
	if( ch >= 'A' && ch <= 'F' ) return ch - 'A' + 10;
	return -1;
}

char bwa2i( char ch, char **src, int base, int *nump ) {
	int num, digit;
	char *p;

	p = *src; num = 0;
	while( ( digit = bwa2d( ch ) ) >= 0 ) {
		if ( digit > base ) break;
		num = num*base + digit;
		ch = *p++;
	}
	*src = p; *nump = num;
	return ch;
}

void bwui2a( unsigned int num, unsigned int base, char *bf ) {
	int n = 0;
	int dgt;
	unsigned int d = 1;
	
	while( (num / d) >= base ) d *= base;
	while( d != 0 ) {
		dgt = num / d;
		num %= d;
		d /= base;
		if( n || dgt > 0 || d == 0 ) {
			*bf++ = dgt + ( dgt < 10 ? '0' : 'a' - 10 );
			++n;
		}
	}
	*bf = 0;
}

void bwi2a( int num, char *bf ) {
	if( num < 0 ) {
		num = -num;
		*bf++ = '-';
	}
	bwui2a( num, 10, bf );
}

void format ( controller * channel, char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			bufferputc( channel, ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = bwa2i( ch, &fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				bufferputc( channel, va_arg( va, char ) );
				break;
			case 's':
				bufferputw( channel, w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				bwui2a( va_arg( va, unsigned int ), 10, bf );
				bufferputw( channel, w, lz, bf );
				break;
			case 'd':
				bwi2a( va_arg( va, int ), bf );
				bufferputw( channel, w, lz, bf );
				break;
			case 'x':
				bwui2a( va_arg( va, unsigned int ), 16, bf );
				bufferputw( channel, w, lz, bf );
				break;
			case '%':
				bufferputc( channel, ch );
				break;
			}
		}
	}
}

void bufferprintf( controller * channel, char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        format( channel, fmt, va );
        va_end(va);
}

void controllerputc( controller * channel) {
	if(channel->outbufferfirst == channel->outbufferlast){
		return;
	}
	int *flags, *data;
	switch( channel->com ) {
	case COM1:
		flags = (int *)( UART1_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART1_BASE + UART_DATA_OFFSET );
		break;
	case COM2:
		flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );
		data = (int *)( UART2_BASE + UART_DATA_OFFSET );
		break;
	default:
		break;
	}


	int i;
	for (i = 0; i < 300; i++) {
	//while (channel->outbufferfirst < channel->outbufferlast) {
		if ((channel->com == COM1 && (*flags & CTS_MASK) && !(*flags & TXBUSY_MASK) && (*flags & TXFE_MASK)) ||
			(channel->com == COM2 && !(*flags & TXBUSY_MASK) && (*flags & TXFE_MASK))) {
			if (channel->outbufferfirst < channel->outbufferlast) {
				int j = channel->outbufferfirst;
				*data = channel->outbuffer[j];
				channel->outbufferfirst = (j + 1) % channel->insize;
			}
			else {
				return;
			}
		}
		
	}
}



int bwputc( char c ) {
	int *flags, *data;

	data = (int *)( UART2_BASE + UART_DATA_OFFSET );
	flags = (int *)( UART2_BASE + UART_FLAG_OFFSET );

	while ( (*flags & TXFF_MASK ));
	*data = c;
	return 0;
}

int bwputx( char c ) {
	char chh, chl;

	chh = c2x( c / 16 );
	chl = c2x( c % 16 );
	bwputc( chh );
	return bwputc( chl );
}


int bwputr( int reg ) {
	int byte;
	char *ch = (char *) &reg;

	for( byte = 3; byte >= 0; byte-- ) bwputx( ch[byte] );
	return bwputc( ' ' );
}


int bwputstr( char *str ) {
	while( *str ) {
		if( bwputc( *str ) < 0 ) return -1;
		str++;
	}
	return 0;
}

void bwputw( int n, char fc, char *bf ) {
	char ch;
	char *p = bf;

	while( *p++ && n > 0 ) n--;
	while( n-- > 0 ) bwputc( fc );
	while( ( ch = *bf++ ) ) bwputc( ch );
}

void bwformat(char *fmt, va_list va ) {
	char bf[12];
	char ch, lz;
	int w;

	
	while ( ( ch = *(fmt++) ) ) {
		if ( ch != '%' )
			bwputc( ch );
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch ( ch ) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = bwa2i( ch, (char **)&fmt, 10, &w );
				break;
			}
			switch( ch ) {
			case 0: return;
			case 'c':
				bwputc( va_arg( va, char ) );
				break;
			case 's':
				bwputw( w, 0, va_arg( va, char* ) );
				break;
			case 'u':
				bwui2a( va_arg( va, unsigned int ), 10, bf );
				bwputw( w, lz, bf );
				break;
			case 'd':
				bwi2a( va_arg( va, int ), bf );
				bwputw( w, lz, bf );
				break;
			case 'x':
				bwui2a( va_arg( va, unsigned int ), 16, bf );
				bwputw( w, lz, bf );
				break;
			case '%':
				bwputc( ch );
				break;
			}
		}
	}
}


void bwprintf( char *fmt, ... ) {
        va_list va;

        va_start(va,fmt);
        bwformat( fmt, va );
        va_end(va);
}
