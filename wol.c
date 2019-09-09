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
#include "wol.h"

#if USE_WOL
	
//Default MAC
unsigned char wol_mac[] = WOL_MAC;
unsigned char wol_enable = 0;
	
//----------------------------------------------------------------------------
//
void wol_init (void)
{
	//Broadcast-Adresse aus EEPROM auslesen, oder default verwenden
	(*((unsigned long*)&wol_bcast_ip[0])) = get_eeprom_value(WOL_BCAST_EEPROM_STORE,WOL_BCAST_IP);
	
	//MAC aus EEPROM auslesen, oder bei default bleiben
	unsigned char value[6];
	for (unsigned char count = 0; count<6;count++)
	{
		eeprom_busy_wait ();	
		value[count] = eeprom_read_byte((unsigned char *)(WOL_MAC_EEPROM_STORE + count));
	}
	if ((*((unsigned long*)&value[0])) != 0xFFFFFFFF)
	{
		wol_mac[0] = value[0];
		wol_mac[1] = value[1];
		wol_mac[2] = value[2];
		wol_mac[3] = value[3];
		wol_mac[4] = value[4];
		wol_mac[5] = value[5];
	}
	WOL_DEBUG("** WOL-INIT IP:%1i.%1i.%1i.%1i  MAC:0x%2x-0x%2x-0x%2x-0x%2x-0x%2x-0x%2x **\r\n",
								wol_bcast_ip[0],wol_bcast_ip[1],wol_bcast_ip[2],wol_bcast_ip[3],
								wol_mac[0],wol_mac[1],wol_mac[2],wol_mac[3],wol_mac[4],wol_mac[5]);
}

//----------------------------------------------------------------------------
//MagicPacket versenden (immer 102 Byte)
void wol_request ()
{
        unsigned int byte_count = 0;

		//sync stream erstellen (sechs mal FF)
		for (; byte_count < 6; byte_count++)
		{
			eth_buffer[UDP_DATA_START + byte_count] = 0xFF;
			
		}
        //16 mal die MAC-Adresse anhängen / darauf achten, dass nicht zu viel auf dem buffer landet
		for (; byte_count < 102 && byte_count < (MTU_SIZE-(UDP_DATA_START));)
		{
			for (unsigned char i=0; i < 6; i++)
			{
				eth_buffer[UDP_DATA_START + byte_count] = wol_mac[i];
				byte_count++;
			}
		}
		create_new_udp_packet(byte_count,WOL_PORT,WOL_PORT,(*(unsigned long*)&wol_bcast_ip[0]));
		
		WOL_DEBUG("** MagicPacket gesendet an: %1i.%1i.%1i.%1i (0x%2x-0x%2x-0x%2x-0x%2x-0x%2x-0x%2x) **\r\n",
											wol_bcast_ip[0],wol_bcast_ip[1],wol_bcast_ip[2],wol_bcast_ip[3],
											wol_mac[0],wol_mac[1],wol_mac[2],wol_mac[3],wol_mac[4],wol_mac[5]);
		return;

}
#endif //USE_WOL