#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define  F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)

char print_String[25];
volatile int adc_value = 0;
int Duty_Cycle = 0;

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


void Initialize() {
	// put your setup code here, to run once:
	cli();
	
	// Set PD5 output, low output (Buzzer)
	DDRD |= (1 << DDD5);
	PORTD &= ~(1 << PORTD5);

	// prescaler / 256
	TCCR0B |= (1 << CS00);
	TCCR0B |= (1 << CS01);
	TCCR0B &= ~(1 << CS02);

	// set timer 0 to PWM, phase correct mode
	TCCR0A |= (1 << WGM00);
	TCCR0A &= ~(1 << WGM01);
	TCCR0B |= (1 << WGM02);

	OCR0A = 200;
	OCR0B = 50;

	// Clear OC0B on compare match when up-counting. Set OC0B on compare match when down-counting
	TCCR0A |= (1 << COM0B1);
	
	// ADC
	PRR &= ~(1<<PRADC);

	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);

	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);

	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);

	ADCSRA |= (1<<ADATE);
	ADCSRB &= ~(1<<ADTS0);
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);

	DIDR0 |= (1<<ADC0D);

	ADCSRA |= (1<<ADEN);

	ADCSRA |= (1<<ADIE);

	ADCSRA |= (1<<ADSC);

	sei();
}

ISR(ADC_vect) {
	adc_value = ADC;
}

int main() {
	
	Initialize();
	UART_Initialize(BAUD_PRESCALER);
	
	while(1) {

		OCR0B = (adc_value < 100) ? OCR0A*0.05 : (adc_value < 200) ? OCR0A*0.1 : (adc_value < 300) ? OCR0A*0.15 : (adc_value < 400) ? OCR0A*0.2 : (adc_value < 500) ? OCR0A*0.25 : (adc_value < 600) ? OCR0A*0.3: (adc_value < 700) ? OCR0A*0.35: (adc_value < 800) ? OCR0A*0.4: (adc_value < 900) ? OCR0A*0.45: OCR0A*0.5;
		Duty_Cycle =  OCR0B * 100/OCR0A;
		sprintf(print_String, "adc: %d, Duty Cycle: %d\n", adc_value, Duty_Cycle);
		UART_print(print_String);
		_delay_ms(1000);
	}
}

