/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        12.11.2007
 Description:    HTTP-Client

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

//!!!!!!*************EXPERIMENTAL*************!!!!!!

#ifndef _HTTPCLIENT_H
	#define _HTTPCLIENT_H

	#define HTTPC_DEBUG usart_write
	//#define HTTPC_DEBUG(...)

    #define WEATHER_SERVER_IP IP(209,162,186,60)
    //#define WEATHER_SERVER_IP IP(87,106,58,153)

	#include <avr/io.h>
	#include <avr/pgmspace.h>
	#include "stack.h"
	#include "usart.h"
	#include "timer.h"
	

	void http_request (void);
	void test (unsigned char);
	
#endif