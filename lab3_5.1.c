#include <avr/io.h>
#include <avr/interrupt.h>



void Initialize()
{
	//Disable global interrupts
	cli();
	
	// Set PD6 as OUTPUT.
	DDRD |= (1 << DDD6);
		
	// Timer 0 setup
	// Set Timer 0 clock to be internal div by 8
	TCCR0B &= ~(1<<CS00);
	TCCR0B |= (1<<CS01);
	TCCR0B &= ~(1<<CS02);
	
	
	// Set Timer 0 to Normal
	TCCR0A &= ~(1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B &= ~(1<<WGM02);
	
	// Overflow Interrupt Enable
	TIMSK0 |= (1<<TOIE0);
	
	// Clear Overflow flag
	TIFR0 |= (1<< TOV0);
	
	//Enable global interrupts
	sei();
}


ISR(TIMER0_OVF_vect)
{
	PORTD ^= (1 << PORTD6); //  Timer0 toggle its output when Timer0 overflows
}


int main(void)
{
	Initialize();
	
	while (1)// loop forever
	{
	}
}

