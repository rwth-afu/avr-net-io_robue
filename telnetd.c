/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        24.10.2007
 Description:    Webserver Applikation

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
#include "telnetd.h"
char *retransmission_pointer;

//----------------------------------------------------------------------------
//Initialisierung des telnetd Testservers
void telnetd_init (void)
{
	//Serverport und Anwendung in TCP Anwendungsliste eintragen
	add_tcp_app (TELNET_APP_PORT, (void(*)(unsigned char))telnetd);
}

//----------------------------------------------------------------------------
//telnet Testserver
void telnetd (unsigned char index)
{
	TELNET_DEBUG("TELNET Anwendung gestartet\r\n");
	
    //Verbindung wurde abgebaut!
    if (tcp_entry[index].status & FIN_FLAG)
    {
        usart_status.usart_disable = 0;
        return;
    }
    
	if (tcp_entry[index].app_status == 0 || tcp_entry[index].app_status == 1)
	{
		tcp_entry[index].app_status = 1; 
		memcpy_P(&eth_buffer[TCP_DATA_START],PSTR("Start Telnet Server:\r\n"),22);
		create_new_tcp_packet(22,index);
		usart_status.usart_disable = 1;
		telnetd_status.ack_wait = 0;
		tcp_entry[index].time = TCP_TIME_OFF;
		return;
	}	

	if ((tcp_entry[index].app_status > 1) && (tcp_entry[index].status&PSH_FLAG))
	{
		tcp_entry[index].app_status = 2;	
		for (int a = TCP_DATA_START_VAR;a<(TCP_DATA_END_VAR);a++)
		{
            #if CMD_TELNET
                unsigned char receive_char;
                receive_char = eth_buffer[a];
        
                if (receive_char == 0x08)
                {
                    if (buffercounter) buffercounter--;
                }
                else if (receive_char == '\r' && (!(usart_rx_buffer[buffercounter-1] == '\\')))
                {
                    usart_rx_buffer[buffercounter] = 0;
                    buffercounter = 0;
                    usart_status.usart_ready = 1;
                    break;    
                }
                else if (buffercounter < BUFFER_SIZE - 1)
                {
                    usart_rx_buffer[buffercounter++] = receive_char;    
                }
            #else
                #if !USE_CAM  
                    //Warten solange bis Zeichen gesendet wurde
                    while(!(USR & (1<<UDRE)));
                    //Ausgabe des Zeichens
                    UDR = eth_buffer[a];
                #endif
            #endif
		}	
		tcp_entry[index].status =  ACK_FLAG;
		create_new_tcp_packet(0,index);
		return;
	}
	
	//Ack erhalten vom gesendeten Packet
	if ((tcp_entry[index].app_status > 1)&&(tcp_entry[index].status&ACK_FLAG)&&(telnetd_status.ack_wait))
	{
		tcp_entry[index].time = TCP_TIME_OFF;
		rx_buffer_pointer_out = retransmission_pointer;
		telnetd_status.ack_wait = 0;
		return;
	}
	
	//Time out kein ack angekommen
	if (tcp_entry[index].status == 0) 
	{
		//Daten nochmal senden
		telnetd_status.ack_wait = 0;
		telnetd_send_data ();
		return;
	}
	
	return;
}

//----------------------------------------------------------------------------
//telnet Testserver
void telnetd_send_data (void)
{
	unsigned char index;
	if (rx_buffer_pointer_in != retransmission_pointer)
	{
		for (index = 0;index<MAX_TCP_ENTRY;index++)
		{
			if(tcp_entry[index].dest_port == LBBL_ENDIAN_INT(23))
			{
				if(!telnetd_status.ack_wait)
					{
					telnetd_status.ack_wait = 1;
					unsigned int count = 0;
					retransmission_pointer = rx_buffer_pointer_out; 
					
					while(rx_buffer_pointer_in != retransmission_pointer)
					{
						eth_buffer[TCP_DATA_START+count] = *retransmission_pointer++;
						count++;
						if (retransmission_pointer == &usart_rx_buffer[BUFFER_SIZE-1])
						{
							retransmission_pointer = &usart_rx_buffer[0];
						}
					}
					tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
					create_new_tcp_packet(count,index);
					tcp_entry[index].time = TCP_MAX_ENTRY_TIME;
					return;
				}
				else
				{
				return;
				}
			}
		}
		if (index >= MAX_TCP_ENTRY)
		{
			if(usart_status.usart_disable)
			{
				usart_status.usart_disable = 0;
			}
		}
	}
}