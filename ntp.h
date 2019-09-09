/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        12.11.2007
 Description:    NTP Client

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

#if USE_NTP
#ifndef _NTPCLIENT_H
	#define _NTPCLIENT_H

	#define NTP_DEBUG usart_write
	//#define NTP_DEBUG(...)

	#include <avr/io.h>
	#include <avr/pgmspace.h>
	#include "stack.h"
	#include "usart.h"
	#include "timer.h"

	#define NTP_CLIENT_PORT		2300
	#define NTP_SERVER_PORT		123

	//Refresh alle 1800 Sekunden also jede halbe Stunde
	#define NTP_REFRESH 1140


	#define NTP_IP_EEPROM_STORE 	50

	unsigned char ntp_server_ip[4];
	volatile unsigned int ntp_timer;
	
	void ntp_init(void);
	void ntp_request(void); 
	void ntp_get(unsigned char);
	
	#define GMT_TIME_CORRECTION 3600	//1 Stunde in Sekunden
	
	struct NTP_GET_Header {
		char dummy[40];
		unsigned long rx_timestamp;
	};
	
#endif
#endif //USE_NTP