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

#define ENABLE_PWM 	TIMSK |= (1<<OCIE0A)|(1<<OCIE0B)|(1<<TOIE0)
#define DISABLE_PWM TIMSK &= ~((1<<OCIE0A)|(1<<OCIE0B)|(1<<TOIE0))

#define PWM_MIN 8
#define PWM_MAX 247

#define LED_EN_0 0x01
#define LED_EN_1 0x02
#define LED_EN_2 0x04
#define LED_EN_3 0x08
#define LED_EN_4 0x10

uint32_t xorshift32(uint32_t *state);

uint32_t xorshift32_state;

uint8_t led_enable;

uint8_t en_01;
uint8_t en_23;
uint8_t en_4;

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
const uint8_t shimmer[128] = {8, 14, 20, 25, 31, 37, 43, 49, 54, 60, 66, 71, 77, 83, 88, 94, 99, 104,
						  110, 115, 120, 125, 130, 135, 140, 145, 150, 154, 159, 163, 168, 172, 176,
						  180, 184, 188, 192, 196, 199, 203, 206, 209, 212, 215, 218, 221, 223, 226,
						  228, 230, 232, 234, 236, 238, 239, 241, 242, 243, 244, 245, 246, 246, 247,
						  247, 247, 247, 247, 247, 246, 245, 245, 244, 243, 242, 240, 239, 238, 236,
					      234, 232, 230, 228, 225, 223, 220, 217, 215, 212, 208, 205, 202, 198, 195,
						  191, 187, 183, 179, 175, 171, 167, 162, 158, 153, 149, 144, 139, 134, 129,
						  124, 119, 114, 109, 103, 98, 92, 87, 81, 76, 70, 65, 59, 53, 47, 42, 36,
						  28, 21, 15};

volatile uint8_t pwm_state = 0;
volatile uint8_t mode = 0;

int main(void)
{
	uint8_t mode_init = 1;
	uint8_t increasing1 = 0;
	uint8_t frame_cnt = 0;
	uint8_t temp = 0;
	
	//uint8_t increasing2 = 1;
	
	xorshift32_state = 0x1337; //seed for random
	
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
	
	mode = 4;
	
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
			
				if(TIFR & (1<<OCF1A))
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
							
				if(TIFR & (1<<OCF1A))
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
				
			case 3: //SHIMMER
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
							
				if(TIFR & (1<<OCF1A))
				{
					if (frame_cnt < 32)
					{
						led_enable = LED_EN_1;
						pwm.led1 = shimmer[frame_cnt];
						frame_cnt++;
					}
					else if(frame_cnt < 64)
					{
						led_enable = LED_EN_0 + LED_EN_1 + LED_EN_3 + LED_EN_4;
						pwm.led0 = shimmer[frame_cnt - 32];
						pwm.led1 = shimmer[frame_cnt];
						pwm.led3 = shimmer[frame_cnt - 32];
						pwm.led4 = shimmer[frame_cnt - 32];
						frame_cnt++;
					}
					else if(frame_cnt < 128)
					{
						led_enable = LED_EN_0 + +LED_EN_1 + LED_EN_2 + LED_EN_3 + LED_EN_4;
						pwm.led0 = shimmer[frame_cnt - 32];
						pwm.led1 = shimmer[frame_cnt];
						pwm.led2 = shimmer[frame_cnt - 64];
						pwm.led3 = shimmer[frame_cnt - 32];
						pwm.led4 = shimmer[frame_cnt - 32];	
						frame_cnt++;
					}
					else if(frame_cnt < 160)
					{
						led_enable = LED_EN_0 + LED_EN_2 + LED_EN_3 + LED_EN_4;
						pwm.led0 = shimmer[frame_cnt - 32];
						pwm.led2 = shimmer[frame_cnt - 64];
						pwm.led3 = shimmer[frame_cnt - 32];
						pwm.led4 = shimmer[frame_cnt - 32];
						frame_cnt++;
					}
					else if( frame_cnt < 192)
					{
						led_enable = LED_EN_2;
						pwm.led2 = shimmer[frame_cnt - 64];
						frame_cnt++;
					}
					else if( frame_cnt < 255)
					{
						led_enable = 0;
						frame_cnt++;
					}
					else
					{
						frame_cnt = 0;
					}
									
					OCR1A = 0x07; //7 x 512us = 3.584ms period
					TIFR |= (1<<OCF1A);
					TCNT1 = 0x00;					
				}

				break;
				
			case 4:
				if (mode_init)
				{
					pwm.led0 = PWM_MIN;
					pwm.led1 = PWM_MIN;
					pwm.led2 = PWM_MIN;
					pwm.led3 = PWM_MIN;
					pwm.led4 = PWM_MIN;
					
					led_enable = LED_EN_0;
					
					//ENABLE_PWM;
					temp = 0;
					
					increasing1 = 1;
					
					mode_init = 0;
				}
				
				if(TIFR & (1<<OCF1A))
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
					
					if(pwm.led0 < PWM_MIN)
					{
						increasing1 = 1;
						temp = xorshift32(&xorshift32_state)%5;
						led_enable = (1<<temp);
					}
					else if(pwm.led0 > PWM_MAX)
					{
						increasing1 = 0;
					}
					
					OCR1A = 0x01; //1 x 512us = 512us period
					TIFR |= (1<<OCF1A);
					TCNT1 = 0x00;
				}
				break;
			
			case 5:
				if (mode_init)
				{
					pwm.led0 = PWM_MIN;
					pwm.led1 = PWM_MIN;
					pwm.led2 = PWM_MIN;
					pwm.led3 = PWM_MIN;
					pwm.led4 = PWM_MIN;
					
					led_enable = LED_EN_0 + LED_EN_1 + LED_EN_2 + LED_EN_3 + LED_EN_4;
					
					//ENABLE_PWM;
					
					
					mode_init = 0;
				}
				
				if(TIFR & (1<<OCF1A))
				{
					

					OCR1A = 0x14; //20 x 512us = 10.24ms period
					TIFR |= (1<<OCF1A);
					TCNT1 = 0x00;
				}
			
				break;
			
			default:
				break;
			
		}
		
		en_01 = led_enable & (LED_EN_0 + LED_EN_1);
		en_23 = led_enable & (LED_EN_2 + LED_EN_3);
		en_4  = led_enable & (LED_EN_4);
		
    }
}

//XORSHIFT32 algorithm for pseudo random numbers
uint32_t xorshift32(uint32_t *state){
	
	uint32_t x = *state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*state = x;
	return x;
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
			//PORTB |= led_enable & (LED_EN_2 + LED_EN_3);
			PORTB = en_23;
			OCR0A = pwm.led4;
			//OCR0B = ; disable this later?
			pwm_state = 1;
			break;
		case 1:
			//PORTB |= (1<<PB4);
			//PORTB |= led_enable & (LED_EN_4);
			PORTB = en_4;
			OCR0A = pwm.led0;
			OCR0B = pwm.led1;
			pwm_state = 2;
			break;
		case 2:
			//PORTB |= (1<<PB0)|(1<<PB1);
			//PORTB |= led_enable & (LED_EN_0 + LED_EN_1);
			PORTB = en_01;
			OCR0A = pwm.led2;
			OCR0B = pwm.led3;
			pwm_state = 0;
			break;
		default:
			break;
	}
}




