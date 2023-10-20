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
volatile unsigned int edge_rising = 0;
volatile unsigned int edge_falling = 0;
volatile unsigned int distance = 0;
volatile int overflow_num = 0;

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

void distance_Measure()
{
	// Send a trigger pulse
	PORTB |= (1 << PORTB3);
	_delay_ms(1);
	PORTB &= ~(1 << PORTB3);
	

	// Receive echo signal
	while ( !(PINB & (1<<PINB2)) );
	edge_rising = TCNT1;
	overflow_num = 0;
	
	while ( PINB & (1<<PINB2) );
	edge_falling = TCNT1;

	// Distance = ((edge_falling - edge_rising) * 1024/16000000 ) * 340 * 100 /2 (cm)
	distance = (edge_falling + overflow_num * 65535 - edge_rising) * 1.1;
	sprintf(print_String, "distance is %u cm", distance);
	UART_print(print_String);
	
}

void Initialize()
{
	//Disable global interrupts
	cli();
	
	// Set PB2 (pin 10) as INPUT. (Echo pin)
	DDRB &= ~(1 << DDB2);
	
	// Set PB3(pin 11) as OUTPUT. (Trig pin)
	DDRB |= (1 << DDB3);
	
	// Timer 1 setup
	// Set Timer 1 clock to be internal div by 1024
	// 16M/1024 Hz timer clock
	TCCR1B |= (1<<CS10);
	TCCR1B &= ~(1<<CS11);
	TCCR1B |= (1<<CS12);
	
	// Set Timer 1 to Normal
	TCCR1A &= ~(1<<WGM10);
	TCCR1A &= ~(1<<WGM11);
	TCCR1B &= ~(1<<WGM12);
	TCCR1B &= ~(1<<WGM13);
	
	// Overflow Interrupt Enable
	TIMSK1 |= (1<<TOIE1);
	
	// Clear Overflow flag
	TIFR1 |= (1<< TOV1);
	
	//Enable global interrupts
	sei();
}

ISR(TIMER1_OVF_vect)
{
	overflow_num++;
}

int main(void)
{
	Initialize();
	UART_Initialize(MYUBRR);
	
	while (1)// loop forever
	{
		distance_Measure();
		_delay_ms(100);
	}
}