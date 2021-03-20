/*
 * Light Up Bow Tie V2
 *
 * main.c
 *
 * Created: 3/20/2021 11:48:43 AM
 * Author : Gregory Evans
 */ 
#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>


int main(void)
{
    DDRB |= (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
	PORTB = 0;

    while (1) 
    {
		PORTB = (1<<PB0);
		_delay_ms(500);
		PORTB = (1<<PB1);
		_delay_ms(500);
		PORTB = (1<<PB2);
		_delay_ms(500);
    }
}

