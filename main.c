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

#define ENABLE_PWM 	TIMSK |= (1<<OCIE0A)|(1<<OCIE0B)|(1<<TOIE0);
#define DISABLE_PWM TIMSK &= ~((1<<OCIE0A)|(1<<OCIE0B)|(1<<TOIE0));

#define PWM_MIN 8;
#define PWM_MAX 247;

#define LED_EN_0 0x01;
#define LED_EN_1 0x02;
#define LED_EN_2 0x04;
#define LED_EN_3 0x08;
#define LED_EN_4 0x10;

uint8_t led_enable;

//These values should stay between PWM_MIN and PWM_MAX;
struct led_pwm_status{
	uint8_t led0;
	uint8_t led1;
	uint8_t led2;
	uint8_t led3;
	uint8_t led4;
} pwm;

/*		  LED LOCATION
*
*     / ----		---- \
*    |   0   \-----/  2   |
*   |		 |	   |       |
*   |		 |	4  |	   |
*    |	 1   /-----\  3   |
*     \ ----		---- /
*/

const uint8_t infinityFrames[6] = {LED_EN_0, LED_EN_1, LED_EN_4, LED_EN_2, LED_EN_3, LED_EN_4};

volatile uint8_t pwm_state = 0;
volatile uint8_t mode = 0;

int main(void)
{
	uint8_t mode_init = 1;
	uint8_t increasing1 = 0;
	uint8_t frame_cnt = 0;
	
	//uint8_t increasing2 = 1;
	
	pwm.led0 = 8;
	pwm.led1 = 8;
	pwm.led2 = 8;
	pwm.led3 = 8;
	pwm.led4 = 8;
	led_enable = LED_EN_0 + LED_EN_1 + LED_EN_2 + LED_EN_3 + LED_EN_4;
	
	cli();
	
	ACSR |= (1<<ACD); //Disable analog comparator to save power
	
    DDRB |= (1<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
	PORTB = 0;
	
	TCCR0A |= (1<<WGM01) | (1<<WGM00); //Fast PWM Mode, TOP = 0xFF
	TCCR0B |= (1<<CS01); //Timer 0 prescaler: 8
	
	TIMSK |= (1<<OCIE0A)|(1<<OCIE0B)|(1<<TOIE0);
	
	TCCR1 = (1<<CS13)|(1<<CS12)|(1<<CS10); //timer 1 prescaler: 4096 (period = 512 us)

	sei();
	
	mode = 1;
	
    while (1) 
    {
		switch(mode)
		{
			case 0: //ALL LEDS ON			
				if (mode_init)
				{
					pwm.led0 = PWM_MAX;
					pwm.led1 = PWM_MAX;
					pwm.led2 = PWM_MAX;
					pwm.led3 = PWM_MAX;
					pwm.led4 = PWM_MAX;
					
					led_enable = LED_EN_0 + LED_EN_1 + LED_EN_2 + LED_EN_3 + LED_EN_4;
					
					//ENABLE_PWM;
					
					mode_init = 0;
				}
				break;
				
			case 1: //UNIFORM PULSE
				if (mode_init)
				{
					pwm.led0 = PWM_MIN;
					pwm.led1 = PWM_MIN;
					pwm.led2 = PWM_MIN;
					pwm.led3 = PWM_MIN;
					pwm.led4 = PWM_MIN;
					
					led_enable = LED_EN_0 + LED_EN_1 + LED_EN_2 + LED_EN_3 + LED_EN_4;
					
					//ENABLE_PWM;
					
					increasing1 = 1;	
					
					mode_init = 0;
				}
			
				if(TIFR & OCF1A)
				{
					
					if(increasing1)
					{
						pwm.led0++;
						pwm.led1++;
						pwm.led2++;
						pwm.led3++;
						pwm.led4++;
					}
					else
					{
						pwm.led0--;
						pwm.led1--;
						pwm.led2--;
						pwm.led3--;
						pwm.led4--;	
					}
					
					if(pwm.led0 < PWM_MIN || pwm.led0 > PWM_MAX)
					{
						increasing1 ^= 0x01;
					}
							
					OCR1A = 0x14; //20 x 512us = 10.24ms period
					TIFR |= (1<<OCF1A);
					TCNT1 = 0x00; 
				}	
				break;	
				
			case 2:	//INFINITY PATTERN
				if (mode_init)
				{
					pwm.led0 = PWM_MAX;
					pwm.led1 = PWM_MAX;
					pwm.led2 = PWM_MAX;
					pwm.led3 = PWM_MAX;
					pwm.led4 = PWM_MAX;
					
					frame_cnt = 0;
					led_enable = 0;
								
					mode_init = 0;
				}
							
				if(TIFR & OCF1A)
				{
					led_enable = infinityFrames[frame_cnt]; //Set new animation frame
					
					frame_cnt++; //increment frame
					
					if (frame_cnt >= 6) //Loop back on final frame
					{
						frame_cnt = 0;
					}
								
					OCR1A = 0xff; //255 x 512us = 130.56ms period
					TIFR |= (1<<OCF1A);
					TCNT1 = 0x00;
				}

				break;
				
			case 3:
				if (mode_init)
				{
						
					mode_init = 0;
				}
							
				if(TIFR & OCF1A)
				{
								
								
					OCR1A = 0x14; //20 x 512us = 10.24ms period
					TIFR |= (1<<OCF1A);
					TCNT1 = 0x00;
				}

				break;
			
			default:
				break;
			
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
			PORTB &= ~(1<<PB4);
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
			//PORTB |= (1<<PB2)|(1<<PB3);
			PORTB |= led_enable & (LED_EN_2 + LED_EN_3);
			OCR0A = pwm.led4;
			//OCR0B = ; disable this later?
			pwm_state = 1;
			break;
		case 1:
			//PORTB |= (1<<PB4);
			PORTB |= led_enable & (LED_EN_4);
			OCR0A = pwm.led0;
			OCR0B = pwm.led1;
			pwm_state = 2;
			break;
		case 2:
			//PORTB |= (1<<PB0)|(1<<PB1);
			PORTB |= led_enable & (LED_EN_0 + LED_EN_1);
			OCR0A = pwm.led2;
			OCR0B = pwm.led3;
			pwm_state = 0;
			break;
		default:
			break;
	}
}




