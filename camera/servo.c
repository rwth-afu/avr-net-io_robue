/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        27.12.2007
 Description:    Servo Control
----------------------------------------------------------------------------*/

#include "servo.h"


// RoBue:
// Initialisierung PWM fuer Servosteuerung
// Puls muss alle 20ms kommen
// Pulsdauer bestimmt die Position (1ms ... 2ms)
// Timer2 (8Bit) mit Vorteiler 1024
// Timer2 ist PORTD7 zugeordnet -> PORTD7 auf Ausgabe
// Bei 16Mhz:
// 16000000:1024:256=61,03...
// Puls alle 16ms

#if USE_SERVO
void servo_init (void)
{
	DDRD |= (1<<7);
	
#if defined (__AVR_ATmega32__)
	TCCR2 = 1<<WGM00|1<<WGM01|1<<COM01|1<<CS00|1<<CS01|1<<CS02;
#endif

#if defined (__AVR_ATmega644__)
	TCCR2A = 1<<WGM00|1<<WGM01|1<<COM2A1; 
	TCCR2B = 1<<CS00|1<<CS01|1<<CS02;
#endif	

}

// -------------------------------------------------------------------

// RoBue:
// Pulsdauer festlegen = Position des Servos
// ca. 1ms -> Anschlag links
// ca. 2ms -> Anschlag rechts
//   1,5ms -> Mitte
// Wert steht in var_array[8]
// 13 -> links
// 37 -> rechts
// Eintrag und Aufruf erfolgt in httpd.c

void servo_go_pos()
{
	OCR2 = (var_array[8]);
}


#endif 

