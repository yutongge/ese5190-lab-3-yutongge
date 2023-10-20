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
volatile unsigned int distance_test = 0;
volatile int overflow_num = 0;
int mode = 0;
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

unsigned int distance_Measure()
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

	distance = (edge_falling + overflow_num * 65535 - edge_rising) * 0.272;
	sprintf(print_String, "distance is %u cm\n", distance);
	UART_print(print_String);
	
	return 0.1163 * distance + 29.7674;
}

void Initialize()
{
	//Disable global interrupts
	cli();
	
	
	DDRB &= ~(1 << DDB0); // Set PB0 (ICP1 pin) as INPUT.
	DDRB &= ~(1 << DDB2); // Set PB2 (pin 10) as INPUT. (Echo pin)
	
	DDRB |= (1 << DDB3); // Set PB3(pin 11) as OUTPUT. (Trig pin)
	DDRD |= (1 << DDD5); // Set PD5 as OUTPUT. (buzzer)
	
	// Timer 0 setup
	// Set Timer 0 clock to be internal div by 64
	// 16M/64 Hz timer clock
	TCCR0B |= (1<<CS00);
	TCCR0B |= (1<<CS01);
	TCCR0B &= ~(1<<CS02);
	
	// Set Timer/Counter Mode of Operation to Phase Correct PWM mode
	TCCR0A |= (1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B |= (1<<WGM02);
	
	// Set the Compare Match Output A Mode to Phase Correct PWM Mode
	TCCR0A |= (1<<COM0A0);
	TCCR0A &= ~(1<<COM0A1);
	
	
	// Timer 1 setup
	// Set Timer 1 clock to be internal div by 1024
	// 16M/1024 Hz timer clock
	TCCR1B &= ~(1<<CS10);
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
	
	// Looking for falling edge
	TCCR1B &= ~(1<<ICES1);
	
	// Clear interrupt flag
	TIFR1 |= (1<<ICF1);
	
	//Enable input capture interrupt
	TIMSK1 |= (1<<ICIE1);
	
	
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
	
	//Enable global interrupts
	sei();
}

ISR(TIMER1_OVF_vect)
{
	overflow_num++;
}

ISR(TIMER1_CAPT_vect)
{
	mode = 1- mode;			// Change the mode
}

ISR(ADC_vect) 
{
	adc_value = ADC;
}

int main(void)
{
	Initialize();
	UART_Initialize(MYUBRR);
	
	while (1)// loop forever
	{
		if (mode == 0) // Continuous Mode
		{
			OCR0A = distance_Measure();
		}
		else    // Discrete Mode
		{
			distance_test = distance_Measure();
			if (distance_test <= 10)
			{
				OCR0A = 10;
			}
			else if(distance_test <= 20)
			{
				OCR0A = 22;
			}
			else if(distance_test <= 30)
			{
				OCR0A = 36;
			}
			else if(distance_test <= 50)
			{
				OCR0A = 40;
			}
			else if(distance_test <= 90)
			{
				OCR0A = 54;
			}
			else if(distance_test <= 150)
			{
				OCR0A = 67;
			}
			else if(distance_test <= 200)
			{
				OCR0A = 73;
			}
			else
			{
				OCR0A = 80;
			}
		}
		sprintf(print_String, "distance is %u cm", distance);
	UART_print(print_String);
		OCR0B = (adc_value < 100) ? OCR0A*0.05 : (adc_value < 200) ? OCR0A*0.1 : (adc_value < 300) ? OCR0A*0.15 : (adc_value < 400) ? OCR0A*0.2 : (adc_value < 500) ? OCR0A*0.25 : (adc_value < 600) ? OCR0A*0.3: (adc_value < 700) ? OCR0A*0.35: (adc_value < 800) ? OCR0A*0.4: (adc_value < 900) ? OCR0A*0.45: OCR0A*0.5;
		Duty_Cycle =  OCR0B * 100/OCR0A;
		sprintf(print_String, "adc: %d, Duty Cycle: %d\n", adc_value, Duty_Cycle);
		UART_print(print_String);
		_delay_ms(100);
	}
}