#include <avr/io.h>
#include <avr/interrupt.h>

// cout = clock freq / ( 4 * N * desired freq) = 16M /(4* 1024 * 440) = 9
int cout = 9;

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
	
	// Set Timer/Counter Mode of Operation to Phase Correct PWM mode
	TCCR0A |= (1<<WGM00);
	TCCR0A &= ~(1<<WGM01);
	TCCR0B |= (1<<WGM02);
	
	// Set the Compare Match Output A Mode to Phase Correct PWM Mode
	TCCR0A |= (1<<COM0A0);
	TCCR0A &= ~(1<<COM0A1);
	
	//Initialize OCR0A
	OCR0A = cout;
	
	//Enable global interrupts
	sei();
}

int main(void)
{
	Initialize();
	while (1);// loop forever
}

