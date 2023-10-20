#include <avr/io.h>
#include <avr/interrupt.h>

// cout = clock freq / ( 2 * N * desired freq) - 1 = 16M /(2 * 1024 * 440) - 1 = 17
int cout = 17;

void Initialize()
{
	//Disable global interrupts
	cli();
	
	// Set PD6 as OUTPUT.
	DDRD |= (1 << DDD6);
	
	// Timer 0 setup
	// Set Timer 0 clock to be internal div by 1024
	// 16M/1024 Hz timer clock
	TCCR0B |= (1<<CS00);
	TCCR0B &= ~(1<<CS01);
	TCCR0B |= (1<<CS02);
	
	
	// Set Timer 0 to Normal
	TCCR0A &= ~(1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B &= ~(1<<WGM02);
	
	// Enable Output Compare interrupt
	TIMSK0 |= (1<<OCIE0A);
	
	//Initialize OCR0A
	OCR0A = cout;
	
	// Clear Timer/Counter 0 Output Compare A match flag
	TIFR0 |= (1<< OCF0A);
	
	//Enable global interrupts
	sei();
}


ISR(TIMER0_COMPA_vect)
{
	PORTD ^= (1 << PORTD6); //  Timer0 toggle its output when Timer0 overflows
	OCR0A += cout;
}


int main(void)
{
	Initialize();
	while (1);// loop forever
}

