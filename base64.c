/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        24.10.2007
 Description:    BASE64 decodieren/encodieren

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
#include "base64.h"

PROGMEM char BASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

void decode_base64 (unsigned char *str1_in,unsigned char *str2_out)
{
	unsigned char *strp;
    unsigned char end_byte = 0;
	
	strp = str2_out;
	
	while (*str1_in != 0)
	{
		*str2_out++= (*str1_in & 0xFC)>>2;
		
		*str2_out = (((*str1_in++)&0x03)<<4); 
		if (*str1_in==0)
		{
			str2_out++;
            end_byte = 2;
			break;
		}
		else
		{
			*str2_out++ += ((*str1_in & 0xF0)>>4);
			*str2_out = (((*str1_in++)& 0x0F)<<2);
			if (*str1_in==0)
			{
				str2_out++;
                end_byte = 1;
                break;
			}
			else
			{
				*str2_out++	+= ((*str1_in & 0xC0)>>6);
				*str2_out++= *str1_in++ & 0x3F;
			}
		}
	}
	*str2_out = 0;
    
	while(strp != str2_out)
	{
			*strp = pgm_read_byte(&BASE64CODE[(*strp) & 0x3F]);
			strp++;
	}
 
    while (end_byte--)
    {
        *strp = '=';
        strp++;
    }
    *strp = 0; 
}