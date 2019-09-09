/*------------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        31.12.2007
 Description:    Analogeingänge Abfragen
 
 Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
 GNU General Public License, wie von der Free Software Foundation veröffentlicht, 
 weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
 (nach Ihrer Option) jeder späteren Version. 

 Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
 daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
 FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
 Programm erhalten haben. 
 Falls nicht, schreiben Sie an die Free Software Foundation, 
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
------------------------------------------------------------------------------*/
#include "analog.h"

#if USE_ADC
volatile unsigned char channel = 4;

//------------------------------------------------------------------------------
//
void ADC_Init(void)
{ 
	ADMUX = (1<<REFS0) + channel;
	//Free Running Mode, Division Factor 128, Interrupt on
	ADCSRA=(1<<ADEN)|(1<<ADSC)|(1<<ADATE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADIE);
}

//------------------------------------------------------------------------------
//
ISR (ADC_vect)
{
    ANALOG_OFF; //ADC OFF
	var_array[channel++] = ADC;
	//usart_write("Kanal(%i)=%i\n\r",(channel-1),var_array[(channel-1)]);

	// RoBue:
	// A7 steht nicht mehr zur Verfügung -> 1-Wire
	if (channel > 4) channel = 4;
	// if (channel > 7) channel = 4;

    ADMUX =(1<<REFS0) + channel;
    //ANALOG_ON;//ADC ON
}

#endif //USE_ADC