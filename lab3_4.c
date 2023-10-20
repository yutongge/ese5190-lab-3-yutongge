#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>


// Define the CPU frequency (16 MHz for the Arduino).
// This is necessary for _delay_ms() to work properly.
#define F_CPU 16000000UL

#define BAUD 9600
#define MYUBRR (((F_CPU / (BAUD * 16UL))) - 1)

char print_String[25];


void UART_Initialize(int ubrr)
{
	
	/*Set baud rate */
	UBRR0H = (unsigned char)(ubrr >>8);
	UBRR0L = (unsigned char)ubrr;
	
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	
	
	/* Set frame format: 2 stop bits, 8 data bits */
	
	// sets the number of data bits (character size) in a frame the receiver and transmitter use
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8 data bits
	
	// This bit selects the number of stop bits to be inserted by the transmitter.
	UCSR0C |= (1<<USBS0); // 2 stop bits
}

void  USART_Transmit(unsigned char data)
{
	// Wait for empty transmit buffer
	while(!(UCSR0A & (1<<UDRE0)));
	// Put data into buffer and send data
	UDR0 = data;
	
}

void UART_print(char* String_print)
{
	while(*String_print != 0x00)
	{
		 USART_Transmit(*String_print);
		String_print++;
	}
}


int main(void)
{
	UART_Initialize(MYUBRR);
	
	while (1)// loop forever
	{
		sprintf(print_String,"Hello world!");
		UART_print(print_String);
	}
}