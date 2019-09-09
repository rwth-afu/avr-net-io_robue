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

#include "ntp.h"

#if USE_NTP
volatile unsigned int ntp_timer = NTP_REFRESH;

//----------------------------------------------------------------------------
//
PROGMEM char const NTP_Request[] = {	0xd9,0x00,0x0a,0xfa,0x00,0x00,0x00,0x00,
									0x00,0x01,0x04,0x00,0x00,0x00,0x00,0x00,
									0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
									0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
									0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
									0xc7,0xd6,0xac,0x72,0x08,0x00,0x00,0x00,
									'%','E','N','D' };

unsigned char ntp_server_ip[4];

//----------------------------------------------------------------------------
//Initialisierung des NTP Ports (für Daten empfang)
void ntp_init (void)
{
	//Port in Anwendungstabelle eintragen für eingehende NTP Daten!
	add_udp_app (NTP_CLIENT_PORT, (void(*)(unsigned char))ntp_get);
	
	//NTP IP aus EEPROM auslesen
	(*((unsigned long*)&ntp_server_ip[0])) = get_eeprom_value(NTP_IP_EEPROM_STORE,NTP_IP);
	
	return;
}

//----------------------------------------------------------------------------
//Anforderung der aktuellen Zeitinformationen von einem NTP Server
void ntp_request (void)
{
	//oeffnet eine Verbindung zu einem NTP Server
	unsigned int byte_count;
		
	//ARP Request senden
	unsigned long tmp_ip = (*(unsigned long*)&ntp_server_ip[0]);
	if (arp_request(tmp_ip) == 1)
	{	
		//Interrupt Deaktivieren da Buffer gerade zum senden benutzt wird!
		//ETH_INT_DISABLE;
		PGM_P ntp_data_pointer = NTP_Request;
		for (byte_count = 0;byte_count<(MTU_SIZE-(UDP_DATA_START));byte_count++)
		{
			unsigned char b;
			b = pgm_read_byte(ntp_data_pointer++);
			eth_buffer[UDP_DATA_START + byte_count] = b;
			//wurde das Ende des Packetes erreicht?
			//Schleife wird abgebrochen keine Daten mehr!!
			if (strncasecmp_P("%END",ntp_data_pointer,4)==0)
			{	
				byte_count++;
				break;
			}
		}
		
		create_new_udp_packet(byte_count,NTP_CLIENT_PORT,NTP_SERVER_PORT,tmp_ip);
		//ETH_INT_ENABLE;
		NTP_DEBUG("** NTP Request gesendet! **\r\n");
		return;
	}
	NTP_DEBUG("Kein NTP Server gefunden!!\r\n");
	return;
}

//----------------------------------------------------------------------------
//Empfang der Zeitinformationen von einem NTP Server
void ntp_get (unsigned char index)
{  
	NTP_DEBUG("** NTP DATA GET! **\r\n");
		
	struct NTP_GET_Header *ntp;
	ntp = (struct NTP_GET_Header *)&eth_buffer[UDP_DATA_START];

	ntp->rx_timestamp = LBBL_ENDIAN_LONG(ntp->rx_timestamp);
	ntp->rx_timestamp += GMT_TIME_CORRECTION; //  UTC +1h
	time = ntp->rx_timestamp;
	
	unsigned char hh = (time/3600)%24;
	unsigned char mm = (time/60)%60;
	unsigned char ss = time %60;
	
	NTP_DEBUG("\n\rNTP TIME: %2i:%2i:%2i\r\n",hh,mm,ss);
}

#endif //USE_NTP
