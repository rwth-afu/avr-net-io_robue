/*------------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        22.11.2007
 Description:    Programm zur Ansteuerung eines Standart LCD
				 (HD44870),(SED1278F) und kompatible
Addons :  USE_LCD_4Bit wird zur Ansteuerung für Paraelle Ansteuerung benutzt (Ralf Figge)
------------------------------------------------------------------------------*/

#include "lcd.h"

#if USE_SER_LCD

#if USE_LCD_PORTC
//nix

#else

const char LINE_ADDRESS[] ={0x80,0xC0,0x90,0xD0}; //2 x 16;4 x 16 uvm.
//const char LINE_ADDRESS[] ={0x80,0xC0,0x94,0xD4}; //4 mal 20
volatile unsigned char back_light = 0;

//------------------------------------------------------------------------------
void lcd_init (void)
{	
#if USE_LCD_4Bit
	//Set Port Direction Register to Output for LCD Databus und LCD Steuerbus
	LCD_Port_DDR = LCD_DataOutput+(1<<LCD_RS | 1<<LCD_RW );

	// PORTB0+B3 auf Ausgabe schalten
	DDRB |= 9;
	
#else

	//Set Port Direction Register
	DDR_LCD_DATA_ENABLE	|= (1<<LCD_DATA_ENABLE);
	DDR_LCD_CLOCK |= (1<<LCD_CLOCK);
	DDR_LCD_DATA |= (1<<LCD_DATA);
	PORT_LCD_DATA_ENABLE &= ~(1<<LCD_DATA_ENABLE);

#endif //USE_LCD_4Bit


	//Wait a short Time after Power on	
	WAIT(10000);
		
	for (unsigned char a=0;a<3;a++)
		{
		lcd_write (0x22,0);	//Init in 4 Bit Mode
		lcd_write (0x80,0);	//Set DD-Ram Adresse = 0
		}		
#ifdef ONE_LINES
	lcd_write (0x20,0);	//1 Zeilen
#else
	lcd_write (0x28,0);	//mehrere Zeilen
#endif
	lcd_write (0x0C,0);	//Display On

	// Hintergrundbeleuchtung an
	// PORTB |= 8;

}

//------------------------------------------------------------------------------
void lcd_write (char data,char cd)
{
#if USE_LCD_4Bit
	unsigned int timeout = 0;
	//Wait until LCD is ready
	while ((Read_LCD(0)&(1<<BUSYBIT)) > 0 && timeout++ < 500) {};
	//Set Port Direction Register to Output for LCD Databus and LCD Controlbus
	LCD_Port_DDR = LCD_DataOutput+(1<<LCD_RS | 1<<LCD_RW );

	// PORTB0+B3 auf Ausgabe schalten
	DDRB |= 9;

	//Soll ins Steuer oder Datenregister geschrieben werden?
	if (cd == 0)
		{
		LCD_Port_Write &=~ (1<<LCD_RS); //RS = 0 Steuerregister
		}
		else
		{
		LCD_Port_Write |= (1<<LCD_RS); //RS = 1 Dataregister
		}
	//Schreibsignal setzen
	LCD_Port_Write &=~ (1<<LCD_RW);//Zum Schreiben RW-Pin = Low
	
	//Schreiben der 1. 4Bit an das LCD Display
	LCD_Port_Write = (LCD_Port_Write&0x0F) + (data&0xF0); //Write Nibbel MSB
	//LCD_Port_Write |= (1<<LCD_E);
	PORTB |= 1;
	WAIT(200);
	//LCD_Port_Write &=~ (1<<LCD_E);
	PORTB &=~ 1;	

	//Schreiben der 2. 4Bit an das LCD Display	
	LCD_Port_Write = (LCD_Port_Write&0x0F) + ((data&0x0F)<<4); //Write Nibbel LSB
	//LCD_Port_Write |= (1<<LCD_E);
	PORTB |= 1;
	WAIT(200);
	//LCD_Port_Write &=~ (1<<LCD_E);
	PORTB &=~ 1;
#else
	WAIT(BUSY_WAIT);
	
	unsigned char h_byte = 0;
	unsigned char l_byte = 0;
	
	//Soll ins Seuer oder Datenregister geschrieben werden?
	if(cd)
	{
		h_byte |= (1<<LCD_RS_PIN);
		l_byte |= (1<<LCD_RS_PIN);
	}
	
	if(back_light)
	{
		h_byte |= (1<<LCD_LIGHT_PIN);
		l_byte |= (1<<LCD_LIGHT_PIN);
	}
	
	h_byte |= (data&0xF0)>>1; //Write Nibbel MSB
	l_byte |= (data&0x0F)<<3; //Write Nibbel LSB
		
	//Schreiben der 1. 4Bit an das LCD Display
	shift_data_out(h_byte);

	//Schreiben der 2. 4Bit an das LCD Display	
	shift_data_out(l_byte);
#endif //USE_LCD_4Bit
}

#if USE_LCD_4Bit
//------------------------------------------------------------------------------
char Read_LCD (char CD)
//------------------------------------------------------------------------------
{
	char Data;
	//Set Port Direction Register to Output for LCD Databus und LCD Steuerbus
	LCD_Port_DDR = LCD_DataInput+(1<<LCD_RS | 1<<LCD_RW);

	// PORTB0+B3 auf Ausgabe schalten
	DDRB |= 9;
	
	//Lesesignal setzen
	LCD_Port_Write |= (1<<LCD_RW); //Zum Lesen RW-Pin = High
	//Soll ins Seuer oder Datenregister geschrieben werden?
	if (CD == 0)
		{
		LCD_Port_Write &=~ (1<<LCD_RS); //RS = 0 Steuerregister
		}
		else
		{
		LCD_Port_Write |= (1<<LCD_RS); //RS = 1 Dataregister
		}		
	//LCD_Port_Write |= (1<<LCD_E);//Daten stehen an wenn Enable = High
	PORTB |= 1;
	WAIT(200);
	Data = (LCD_Port_Read&0xF0); //Lesen des 1. Nibble (MSB)
	//LCD_Port_Write &=~ (1<<LCD_E);
	PORTB &=~ 1;	

	//LCD_Port_Write |= (1<<LCD_E);//Daten stehen an wenn Enable = High
	PORTB |= 1;	
	WAIT(200);
	Data += ((LCD_Port_Read&0xF0)>>4); //Lesen des 2. Nibble (LSB)
	//LCD_Port_Write &=~ (1<<LCD_E);
	PORTB &=~ 1;
return(Data);
}

#else
//------------------------------------------------------------------------------
void shift_data_out (char byte)
{
	for (unsigned char a=8; a>0; a--)
	{	
		if ( (byte & (1<<(a-1))) != 0)
		{
			PORT_LCD_DATA |= (1<<LCD_DATA); //Data PIN (High)
		}
		else
		{
			PORT_LCD_DATA &= ~(1<<LCD_DATA); //Data PIN (LOW)
		}
		PORT_LCD_CLOCK |= (1<<LCD_CLOCK); //Clock PIN (High)
		NOP();
		NOP();
		PORT_LCD_CLOCK &= ~(1<<LCD_CLOCK); //Clock PIN (Low)
	}
	
	PORT_LCD_DATA_ENABLE |= (1<<LCD_DATA_ENABLE);
	NOP();
	NOP();
	PORT_LCD_DATA_ENABLE &= ~(1<<LCD_DATA_ENABLE);
}
#endif //USE_LCD_4Bit
//------------------------------------------------------------------------------
void lcd_print_P (unsigned char zeile,unsigned char spalte,const char *Buffer,...)
{
	va_list ap;
	va_start (ap, Buffer);	
	
	int format_flag;
	char str_buffer[10];
	char str_null_buffer[10];
	char move = 0;
	char Base = 0;
	int tmp = 0;
	char by;
	char *ptr;
	
	//Berechnet Adresse für die Zeile und schreibt sie ins DD-Ram
	zeile = LINE_ADDRESS[zeile];
	zeile += spalte;
	lcd_write(zeile,0);
	
	//Ausgabe der Zeichen
    for(;;)
	{
		by = pgm_read_byte(Buffer++);
		if(by==0) break; // end of format string
		
		if (by == '%')
		{
            by = pgm_read_byte(Buffer++);
			if (isdigit(by)>0)
				{
                                 
 				str_null_buffer[0] = by;
				str_null_buffer[1] = '\0';
				move = atoi(str_null_buffer);
                by = pgm_read_byte(Buffer++);
				}
				
			switch (by)
				{
				case 's':
                    ptr = va_arg(ap,char *);
                    while(*ptr) {lcd_write(*ptr++,1); }
                    break;
				case 'b':
					Base = 2;
					goto ConversionLoop;
				case 'c':
					//Int to char
					format_flag = va_arg(ap,int);
					lcd_write (format_flag++,1); 
					break;
				case 'i':
					Base = 10;
					goto ConversionLoop;
				case 'o':
					Base = 8;
					goto ConversionLoop;
				case 'x':
					Base = 16;
					//****************************
					ConversionLoop:
					//****************************
					itoa(va_arg(ap,int),str_buffer,Base);
					int b=0;
					while (str_buffer[b++] != 0){};
					b--;
					if (b<move)
						{
						move -=b;
						for (tmp = 0;tmp<move;tmp++)
							{
							str_null_buffer[tmp] = '0';
							}
						//tmp ++;
						str_null_buffer[tmp] = '\0';
						strcat(str_null_buffer,str_buffer);
						strcpy(str_buffer,str_null_buffer);
						}
					lcd_print_str (str_buffer);
					move =0;
					break;
				}
			
			}	
		else
			{
			lcd_write (by,1);
			}
		}
	va_end(ap);
}

//------------------------------------------------------------------------------
void lcd_print_str (char *Buffer)
{
	while (*Buffer != 0)
		{
		lcd_write (*Buffer++,1);
		};
};

//------------------------------------------------------------------------------
void lcd_clear (void)
{
	lcd_write (1,0); //Clear Display
	lcd_write (0x80,0);	//Set DD-Ram Adresse = 0
}

#endif
#endif

