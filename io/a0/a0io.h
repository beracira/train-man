
typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

typedef struct controller{
	int com;
	int speed;
	char * inbuffer;
	int inbufferfirst;
	int inbufferlast;
	int insize;
	char * outbuffer;
	int outbufferfirst;
	int outbufferlast;
	int outsize;
} controller;


#define COM1	0
#define COM2	1

#define ON	1
#define	OFF	0

#define ON	1
#define	OFF	0

#define SW_STRAIGHT 33
#define SW_CURVED 	34

int setfifo( controller * channel, int state );

int setspeed( controller * channel);

void robchannelsend( controller * channel );

int bufferputc( controller * channel, char c );

int bufferputx( controller * channel, char c );

int bufferputstr( controller * channel, char *str );

int bufferputr( controller * channel, unsigned int reg );

void bufferputw( controller * channel, int n, char fc, char *bf );

int buffergetc( controller * channel );

void bufferprintf( controller * channel, char *format, ... );

void controllerputc( controller * channel);

char c2x( char ch );

int bwa2d( char ch );

char bwa2i( char ch, char **src, int base, int *nump);

void bwui2a( unsigned int num, unsigned int base, char *bf ) ;

void bwi2a( int num, char *bf );

void bwprintf( char *fmt, ... );
