/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        some changes by Michael Kleiber
 known Problems: none
 Version:        06.12.2008
 Description:    Servo Control
----------------------------------------------------------------------------*/

#include "servo.h"

#if USE_SERVO

void servo_init (void)
{
#if defined (__AVR_ATmega644__)
	TCCR2A = 1<<WGM00|1<<WGM01|1<<COM2A1; 
	TCCR2B = 1<<CS00|1<<CS01|1<<CS02;
#endif	

#if defined (__AVR_ATmega32__)
	TCCR2 = (1<<WGM00)|(1<<WGM01)|(1<<COM01)|(1<<CS00)|(1<<CS01)|(1<<CS02); // invert output: |(1<<COM00)
#endif
	DDRD |= 0x80;
}

void servo_go_pos() // pos in var_array[8] 0..255 
{
	OCR2 = (var_array[8]/11) + 13; // ->min = 13 max = 37 
}

#endif 

