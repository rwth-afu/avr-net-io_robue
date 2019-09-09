/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Thomas
 Remarks:        
 known Problems: none
 Version:        23.01.2008
 Description:    WOL

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
#include <config.h>

#if USE_WOL

    unsigned char wol_bcast_ip[4];
	unsigned char wol_mac[6];
    unsigned char wol_enable;

	#define WOL_DEBUG usart_write
	//#define WOL_DEBUG(...)

	#include <avr/io.h>
	#include <avr/pgmspace.h>
	#include "stack.h"
	#include "usart.h"

	#define WOL_PORT				9

	#define WOL_MAC_EEPROM_STORE	60 //6 Byte
	#define WOL_BCAST_EEPROM_STORE	66 //4 Byte
	
	void wol_init(void);
	void wol_request(void); 
	
#endif //USE_NTP