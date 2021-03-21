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
#include <avr/interrupt.h>
#include <util/delay.h>

//These values should stay between approximately 8 and 247;
struct led_pwm_status{
	uint8_t led0;
	uint8_t led1;
	uint8_t led2;
	uint8_t led3;
	uint8_t led4;
} pwm;

volatile uint8_t pwm_state = 0;

int main(void)
{
	uint8_t increasing1 = 0;
	uint8_t increasing2 = 1;
	
	pwm.led0 = 32;
	pwm.led1 = 64;
	pwm.led2 = 128;
	pwm.led3 = 170;
	pwm.led4 = 200;
	
	cli();
	
    DDRB |= (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
	PORTB = 0;
	
	TCCR0A |= (1<<WGM01) | (1<<WGM00); //Fast PWM Mode, TOP = 0xFF
	TCCR0B |= (1<<CS01); // Clock prescaler: 8
	
	TIMSK |= (1<<OCIE0A)|(1<<OCIE0B)|(1<<TOIE0);

	sei();
    while (1) 
    {
		_delay_ms(10);
		if(increasing1)
		{
			pwm.led0++;
		}
		else
		{
			pwm.led0--;
		}
		
		if(increasing2)
		{
			pwm.led2 += 2;
		}
		else
		{
			pwm.led2 -= 2;
		}
		
		if(pwm.led0 <= 8 || pwm.led0 >= 247)
		{
			increasing1 ^= 0x01;
		}
		
		if(pwm.led2 <= 8 || pwm.led2 >= 247)
		{
			increasing2 ^= 0x01;	
		}
		
    }
}

ISR(TIMER0_COMPA_vect)
{
	switch(pwm_state)
	{
		case 0:
			PORTB &= ~(1<<PB0);
			break;
		case 1:
			PORTB &= ~(1<<PB2);
			break;
		case 2:
			//PORTB &= ~(1<<PB4);
			break;
		default:
			break;
	}
}

ISR(TIMER0_COMPB_vect)
{
	switch(pwm_state)
	{
		case 0:
			PORTB &= ~(1<<PB1);
			break;
		case 1:
			PORTB &= ~(1<<PB3);
			break;
		case 2:
			//PORTB &= ~(1<<PB4);
			break;
		default:
			break;
	}
}

ISR(TIMER0_OVF_vect)
{	
	switch(pwm_state)
	{
		case 0:
			PORTB |= (1<<PB2)|(1<<PB3);
			OCR0A = pwm.led4;
			//OCR0B = ; disable this later?
			pwm_state = 1;
			break;
		case 1:
			/*PORTB |= (1<<PB4);*/

			OCR0A = pwm.led0;
			OCR0B = pwm.led1;
			pwm_state = 2;
			break;
		case 2:
			PORTB |= (1<<PB0)|(1<<PB1);
			OCR0A = pwm.led2;
			OCR0B = pwm.led3;
			pwm_state = 0;
			break;
		default:
			break;
	}
}

