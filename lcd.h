/*------------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        22.11.2007
 Description:    Programm zur Ansteuerung eines Standart LCD
				 (HD44870),(SED1278F) und kompatible

Addons :  USE_LCD_4Bit wird zur Ansteuerung für Paraell Ansteuerung benutzt (Ralf Figge)

------------------------------------------------------------------------------*/

#include "config.h"

#if USE_SER_LCD
#if USE_LCD_PORTC
//nix
#else
#ifndef _LCD_H_
	#define _LCD_H_

	#include <stdlib.h>
	#include <stdarg.h>
	#include <ctype.h>
	#include <string.h>
	#include <avr/io.h>
	#include <avr/pgmspace.h>
	
	//Prototypes
	extern void lcd_write (char,char);
#if USE_LCD_4Bit
	extern char Read_LCD (char);
#else
	void shift_data_out (char);
#endif //USE_LCD_4Bit
	extern void lcd_init (void);
	extern void lcd_clear (void);
	extern void lcd_print_P (unsigned char,unsigned char,const char *Buffer,...);
	extern void lcd_print_str (char *Buffer);
	
	#define lcd_print(a,b,format, args...)  lcd_print_P(a,b,PSTR(format) , ## args)
	
	volatile unsigned char back_light;
				
#if USE_LCD_4Bit

	//LCD_D0 - LCD_D3 connect to GND
	//Im 4Bit Mode LCD_D4-->PORTx.4 ........ LCD_D7-->PORTx.7
	//LCD_RS --> PORTx.4 | LCD_RW --> PORTx.5 | LCD_E --> PORTx.6 | PORTx.7-->NotConnect
	#define LCD_Port_DDR		DDRD	//Port an dem das Display angeschlossen wurde
	#define LCD_Port_Write		PORTD
	#define LCD_Port_Read		PIND

	#define LCD_RS			2 		//Pin für RS		-> PORTD2
	#define LCD_RW			3		//Pin für Read/Write	-> PORTD3
	#define LCD_E			0 		//Pin für Enable 	-> PORTB0 (!)
																
	//Enable hängt an PORTB0
	//Hintergrundbeleuchtung an PORTB3 

	#define LCD_DataOutput			0xF0
	#define LCD_DataInput			0x00

	#define BUSYBIT				7

#else	

	// RoBue:
	// PORTD2-4 fuer serielle LCD nutzen
	#define PORT_LCD_DATA_ENABLE	PORTD
	#define DDR_LCD_DATA_ENABLE		DDRD
	#define LCD_DATA_ENABLE			2
	
	#define PORT_LCD_CLOCK			PORTD
	#define DDR_LCD_CLOCK			DDRD
	#define LCD_CLOCK			4
	
	#define PORT_LCD_DATA			PORTD
	#define DDR_LCD_DATA			DDRD
	#define LCD_DATA			3
	

	
	#define LCD_RS_PIN 			0
	#define LCD_LIGHT_PIN			3


#endif //USE_LCD_4Bit


	#define BUSY_WAIT				6000
	
	#define NOP()	asm("nop")
	#define WAIT(x) for (unsigned long count=0;count<x;count++){NOP();}

#endif //_LCD_H_
#endif //USE_LCD_PORTC
#endif //USE_SER_LCD
