/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        02.12.2007
 Description:    empfangene UDP Daten auf Port 345 
				 werden auf dem LCD ausgegeben

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
#include "udp_lcd.h"

//----------------------------------------------------------------------------
//Initialisierung des NTP Ports (für Daten empfang)
void udp_lcd_init (void)
{
	//Port in Anwendungstabelle eintragen für eingehende NTP Daten!
	add_udp_app (UDP_LCD_PORT, (void(*)(unsigned char))udp_lcd_get);
	return;
}

//----------------------------------------------------------------------------
//Empfang der Zeitinformationen von einem NTP Server
void udp_lcd_get (unsigned char index)
{
	UDP_LCD_DEBUG("** LCD DATA GET Bytes: %i **\r\n",((UDP_DATA_END_VAR)-(UDP_DATA_START)));
#if USE_SER_LCD
	lcd_clear();
	for (int a = UDP_DATA_START;a < UDP_DATA_END_VAR;a++)
	{
		lcd_write (eth_buffer[a],1);
	}	
#endif
}
