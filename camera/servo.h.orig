/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        27.12.2007
 Description:    Servo Control
----------------------------------------------------------------------------*/
#include "config.h"

#ifndef _SERVO_H
	#define _SERVO_H

	#include <avr/io.h>

	#if defined (__AVR_ATmega644__)
		#define TCCR2	TCCR2A
		#define OCR2	OCR2A
		#define COM00	COM2A0
		#define COM01	COM2A1
	#endif

	extern volatile unsigned char servo_pos;

	extern void servo_init (void);
	extern void servo_go_pos (void);

#endif


