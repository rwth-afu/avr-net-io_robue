/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        27.12.2007
 Description:    Servo Control
----------------------------------------------------------------------------*/

#include "servo.h"

#if USE_SERVO
volatile unsigned char servo_pos;// min = 28/1ms max = 56/2ms

void servo_init (void)
{
	DDRD |= (1<<7);
	
#if defined (__AVR_ATmega644__)
	TCCR2A = 1<<WGM00|1<<WGM01|1<<COM2A1; 
	TCCR2B = 1<<CS00|1<<CS01|1<<CS02;
#endif	

#if defined (__AVR_ATmega32__)
	TCCR2 = 1<<WGM00|1<<WGM01|1<<COM01|1<<CS00|1<<CS01|1<<CS02;
#endif
	servo_pos = 0;
	servo_go_pos();
}

void servo_go_pos(void)
{
	OCR2 = ((F_CPU/1024/1000)-1) + (unsigned char)((unsigned int)servo_pos*(F_CPU/1024/1000)/255);
}

#endif 

