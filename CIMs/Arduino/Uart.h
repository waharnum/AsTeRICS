

/*
     AsTeRICS Arduino CIM Firmware

	 file: Uart.h
	 Version: 0.1
	 Author: Chris Veigl (FHTW)
	 Date: 11/11/2011


*/

#ifndef _UART_H_
#define _UART_H_

#include <inttypes.h>


#ifndef F_CPU
#define F_CPU 16000000
#endif

#define INBUF_SIZE 300      // 300 bytes for ringbuffers to store HID requests
#define INBUF_WARNING 30

#define OUTBUF_SIZE 150      // 150 bytes for ringbuffer to store CIM replies
#define OUTBUF_WARNING 30

#define DISABLE_SEND_ISR UCSR0B &= ~(1<<UDRIE0);
#define ENABLE_SEND_ISR UCSR0B |= (1<<UDRIE0);

#define DISABLE_RCV_ISR UCSR0B &= ~(1<<RXCIE0);
#define ENABLE_RCV_ISR UCSR0B |= (1<<RXCIE0);


struct ringbuf_i {
	unsigned int buflen;
	unsigned int rpos;
	unsigned int wpos;
	unsigned char data[INBUF_SIZE];
} ;

struct ringbuf_o {
	unsigned int buflen;
	unsigned int rpos;
	unsigned int wpos;
	unsigned char data[OUTBUF_SIZE];
} ;

extern struct ringbuf_o output_buffer;
extern struct ringbuf_i uart_in,input_mouse, input_keyboard, input_joystick;

void init_InBuf(struct ringbuf_i * rb);
int keys_in_InBuf(struct ringbuf_i * rb);
int free_in_InBuf(struct ringbuf_i * rb);
unsigned char read_InBuf(struct ringbuf_i * rb);
void insert_InBuf(struct ringbuf_i * rb, unsigned char c);

void init_OutBuf(struct ringbuf_o * rb);
int keys_in_OutBuf(struct ringbuf_o * rb);
int free_in_OutBuf(struct ringbuf_o * rb);
unsigned char read_OutBuf(struct ringbuf_o * rb);
void insert_OutBuf(struct ringbuf_o * rb, unsigned char c);

void UART_Init(long baud);
void UART_Putchar(unsigned char c);		 // polling 
void UART_Print (char * str);            // polling
void UART_Send_NonBlocking( char *str, int len); // interrupt driven

#endif
