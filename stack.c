/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        24.10.2007
 Description:    Ethernet Stack

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

#include "stack.h"

TCP_PORT_ITEM TCP_PORT_TABLE[MAX_APP_ENTRY] = // Port-Tabelle
{
	{0,0},
	{0,0},
	{0,0} 
};

UDP_PORT_ITEM UDP_PORT_TABLE[MAX_APP_ENTRY] = // Port-Tabelle
{
	{0,0},
	{0,0},
	{0,0} 
};

unsigned char myip[4];
unsigned char netmask[4];
unsigned char router_ip[4];

unsigned int IP_id_counter = 0;

unsigned char eth_buffer[MTU_SIZE+1];

struct arp_table arp_entry[MAX_ARP_ENTRY];

//TCP Stack Size
//+1 damit eine Verbindung bei vollen Stack abgewiesen werden kann
struct tcp_table tcp_entry[MAX_TCP_ENTRY+1]; 


//----------------------------------------------------------------------------
//Trägt Anwendung in Anwendungsliste ein
void stack_init (void)
{
	//Timer starten
	timer_init();

	//IP, NETMASK und ROUTER_IP aus EEPROM auslesen	
    (*((unsigned long*)&myip[0])) = get_eeprom_value(IP_EEPROM_STORE,MYIP);
	(*((unsigned long*)&netmask[0])) = get_eeprom_value(NETMASK_EEPROM_STORE,NETMASK);
	(*((unsigned long*)&router_ip[0])) = get_eeprom_value(ROUTER_IP_EEPROM_STORE,ROUTER_IP);
	
	//MAC Adresse setzen
	mymac[0] = MYMAC1;
    mymac[1] = MYMAC2;
    mymac[2] = MYMAC3;
    mymac[3] = MYMAC4;
    mymac[4] = MYMAC5;
    mymac[5] = MYMAC6;
	
	/*NIC Initialisieren*/
	usart_write("\n\rNIC init:");
	ETH_INIT();
	usart_write("READY!\r\n");
	
#if USE_ENC28J60
	ETH_PACKET_SEND(60,eth_buffer);
	ETH_PACKET_SEND(60,eth_buffer);
#endif
	
	usart_write("My IP: %1i.%1i.%1i.%1i\r\n\r\n",myip[0],myip[1],myip[2],myip[3]);	
}

//----------------------------------------------------------------------------
//
unsigned long get_eeprom_value (unsigned int eeprom_adresse,unsigned long default_value)
{
	unsigned char value[4];
	
	for (unsigned char count = 0; count<4;count++)
	{
		//eeprom_busy_wait ();	
		value[count] = eeprom_read_byte((unsigned char *)(eeprom_adresse + count));
	}

	//Ist der EEPROM Inhalt leer?
	if ((*((unsigned long*)&value[0])) == 0xFFFFFFFF)
	{
		return(default_value);
	}
	return((*((unsigned long*)&value[0])));
}

//----------------------------------------------------------------------------
//Verwaltung des TCP Timers
void tcp_timer_call (void)
{
	for (unsigned char index = 0;index<MAX_TCP_ENTRY;index++)
	{
		if (tcp_entry[index].time == 0)
		{
			if (tcp_entry[index].ip != 0)
			{
				tcp_entry[index].time = TCP_MAX_ENTRY_TIME;
				if ((tcp_entry[index].error_count++) > MAX_TCP_ERRORCOUNT)
				{
					DEBUG("Eintrag wird entfernt MAX_ERROR STACK:%i\r\n",index);
					ETH_INT_DISABLE;
					tcp_entry[index].status =  RST_FLAG | ACK_FLAG;
					create_new_tcp_packet(0,index);
					ETH_INT_ENABLE;
					tcp_index_del(index);
				}
				else
				{
					DEBUG("Packet wird erneut gesendet STACK:%i\r\n",index);
					find_and_start (index);
				}
			}
		}
		else
		{
			if (tcp_entry[index].time != TCP_TIME_OFF)
			{
				tcp_entry[index].time--;
			}
		}
	}
}

//----------------------------------------------------------------------------
//Verwaltung des ARP Timers
void arp_timer_call (void)
{
	for (unsigned char a = 0;a<MAX_ARP_ENTRY;a++)
	{
		if (arp_entry[a].arp_t_time == 0)
		{
			for (unsigned char b = 0;b<6;b++)
			{
				arp_entry[a].arp_t_mac[b]= 0;
			}
			arp_entry[a].arp_t_ip = 0;
		}
		else
		{
			arp_entry[a].arp_t_time--;
		}
	}
}

//----------------------------------------------------------------------------
//Trägt TCP PORT/Anwendung in Anwendungsliste ein
void add_tcp_app (unsigned int port, void(*fp1)(unsigned char))
{
	unsigned char port_index = 0;
	//Freien Eintrag in der Anwendungliste suchen
	while (TCP_PORT_TABLE[port_index].port)
	{ 
		port_index++;
	}
	if (port_index >= MAX_APP_ENTRY)
	{
		DEBUG("TCP Zuviele Anwendungen wurden gestartet\r\n");
		return;
	}
	DEBUG("TCP Anwendung wird in Liste eingetragen: Eintrag %i\r\n",port_index);
	TCP_PORT_TABLE[port_index].port = port;
	TCP_PORT_TABLE[port_index].fp = *fp1;
	return;
}

//----------------------------------------------------------------------------
//Änderung der TCP PORT/Anwendung in Anwendungsliste
void change_port_tcp_app (unsigned int port_old, unsigned int port_new)
{
	unsigned char port_index = 0;
	//Freien Eintrag in der Anwendungliste suchen
	while (TCP_PORT_TABLE[port_index].port && TCP_PORT_TABLE[port_index].port != port_old)
	{ 
		port_index++;
	}
	if (port_index >= MAX_APP_ENTRY)
	{
		DEBUG("(Portänderung) Port wurde nicht gefunden\r\n");
		return;
	}
	DEBUG("TCP Anwendung Port ändern: Eintrag %i\r\n",port_index);
	TCP_PORT_TABLE[port_index].port = port_new;
	return;
}

//----------------------------------------------------------------------------
//Trägt UDP PORT/Anwendung in Anwendungsliste ein
void add_udp_app (unsigned int port, void(*fp1)(unsigned char))
{
	unsigned char port_index = 0;
	//Freien Eintrag in der Anwendungliste suchen
	while (UDP_PORT_TABLE[port_index].port)
	{ 
		port_index++;
	}
	if (port_index >= MAX_APP_ENTRY)
	{
		DEBUG("Zuviele UDP Anwendungen wurden gestartet\r\n");
		return;
	}
	DEBUG("UDP Anwendung wird in Liste eingetragen: Eintrag %i\r\n",port_index);
	UDP_PORT_TABLE[port_index].port = port;
	UDP_PORT_TABLE[port_index].fp = *fp1;
	return;
}

//----------------------------------------------------------------------------
//Interrupt von der Netzwerkkarte
ISR (ETH_INTERRUPT)
{
	eth.data_present = 1;
    time_watchdog = 0;
	ETH_INT_DISABLE;
}

//----------------------------------------------------------------------------
//ETH get data
void eth_get_data (void)
{ 	
	if(eth.timer)
	{
		tcp_timer_call();
		arp_timer_call();
		eth.timer = 0;
	}	
	if(eth.data_present)
	{
	#if USE_ENC28J60
		while( (PINB &(1<<PB2)) == 0)
		{	
	#endif

	#if USE_RTL8019
		if ( (ReadRTL(RTL_ISR)&(1<<OVW)) != 0)
			{
			DEBUG ("Overrun!\n");
			}

		if ( (ReadRTL(RTL_ISR) & (1<<PRX)) != 0)
		{
			unsigned char ByteH = 0;
			unsigned char ByteL = 1;
		
			while (ByteL != ByteH) //(!= bedeutet ungleich)
			{	
	#endif	
				unsigned int packet_lenght;
				  
				packet_lenght = ETH_PACKET_RECEIVE(MTU_SIZE,eth_buffer);
				/*Wenn ein Packet angekommen ist, ist packet_lenght =! 0*/
				packet_lenght = packet_lenght - 4;
				eth_buffer[packet_lenght+1] = 0;
				check_packet();
			
	#if USE_RTL8019
				//auslesen des Empfangsbuffer BNRY = CURR
				ByteL = ReadRTL(BNRY); //auslesen NIC Register bnry
				WriteRTL ( CR ,(1<<STA|1<<RD2|1<<PS0));
			
				ByteH = ReadRTL(CURR); //auslesen NIC Register curr
				WriteRTL ( CR ,(1<<STA|1<<RD2));
			}
	#endif
		}
	#if USE_RTL8019
		Networkcard_INT_RES();
		Networkcard_Start();
	#endif
		eth.data_present = 0;
		ETH_INT_ENABLE;
	}
	return;
}
//----------------------------------------------------------------------------
//Check Packet and call Stack for TCP or UDP
void check_packet (void)
{
	//Pointer auf Ethernet_Header
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&eth_buffer[ETHER_OFFSET];
	//Pointer auf IP_Header
	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
	//Pointer auf TCP_Header
	struct TCP_Header *tcp;
	tcp = (struct TCP_Header *)&eth_buffer[TCP_OFFSET];
	//Pointer auf ICMP_Header
	struct ICMP_Header *icmp;
	icmp = (struct ICMP_Header *)&eth_buffer[ICMP_OFFSET];
		
	if(ETHERNET_ARP_DATAGRAMM)
	{
		//Erzeugt ein ARP Reply Packet
		arp_reply();
	}
	else
	{
		if(ETHERNET_IP_DATAGRAMM && IF_MYIP)
		{
			//Refresh des ARP Eintrages
			arp_entry_add();
			//Ist protokoll Byte = 1 dann ist es ein ICMP Packet
			if(IP_ICMP_PACKET)
			{
				switch ( icmp->ICMP_Type )
				{
				case (0x08):
				
					//Echo-Request empfangen, erzeugen eines ICMP Reply Packet (PING Echo)
					icmp_send(ip->IP_Srcaddr,0x00,0x00,icmp->ICMP_SeqNum,icmp->ICMP_Id);
					break;
				
				case (0x00):
					//Echo-Reply Packet empfangen, Empfang melden
					//TODO: Erst Sequenznummer vergleichen?, Zeitmessung?
					usart_write("%i",(ip->IP_Srcaddr&0x000000FF));
					usart_write(".%i",((ip->IP_Srcaddr&0x0000FF00)>>8));
					usart_write(".%i",((ip->IP_Srcaddr&0x00FF0000)>>16));
					usart_write(".%i",((ip->IP_Srcaddr&0xFF000000)>>24));
					usart_write(": PONG!\r\n");
					break;
				}
				return;
			}
			else
			{
				if(IP_TCP_PACKET) tcp_socket_process();
				if(IP_UDP_PACKET) udp_socket_process();
			}
		}
	}
	return;
}

//----------------------------------------------------------------------------
//erzeugt einen ARP - Eintrag wenn noch nicht vorhanden 
void arp_entry_add (void)
{
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&eth_buffer[ETHER_OFFSET];
	
	struct ARP_Header *arp;
	arp = (struct ARP_Header *)&eth_buffer[ARP_OFFSET];
	
	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
	
	//Eintrag schon vorhanden?
	for (unsigned char a = 0;a<MAX_ARP_ENTRY;a++)
	{
		if(ETHERNET_ARP_DATAGRAMM)
		{
			if(arp_entry[a].arp_t_ip == arp->ARP_SIPAddr)
			{
			//Eintrag gefunden Time refresh
			arp_entry[a].arp_t_time = ARP_MAX_ENTRY_TIME;
			return;
			}
		} 
		if(ETHERNET_IP_DATAGRAMM)
		{
			if(arp_entry[a].arp_t_ip == ip->IP_Srcaddr)
			{
			//Eintrag gefunden Time refresh
			arp_entry[a].arp_t_time = ARP_MAX_ENTRY_TIME;
			return;
			}
		}
	}
	
	//Freien Eintrag finden
	for (unsigned char b = 0;b<MAX_ARP_ENTRY;b++)
	{
		if(arp_entry[b].arp_t_ip == 0)
		{
			if(ETHERNET_ARP_DATAGRAMM)
			{
				for(unsigned char a = 0; a < 6; a++)
				{
					arp_entry[b].arp_t_mac[a] = ethernet->EnetPacketSrc[a]; 
				}
				arp_entry[b].arp_t_ip = arp->ARP_SIPAddr;
				arp_entry[b].arp_t_time = ARP_MAX_ENTRY_TIME;
				return;
			}
			if(ETHERNET_IP_DATAGRAMM)
			{
				for(unsigned char a = 0; a < 6; a++)
				{
					arp_entry[b].arp_t_mac[a] = ethernet->EnetPacketSrc[a]; 
				}
				arp_entry[b].arp_t_ip = ip->IP_Srcaddr;
				arp_entry[b].arp_t_time = ARP_MAX_ENTRY_TIME;
				return;
			}
			
		DEBUG("Kein ARP oder IP Packet!\r\n");
		return;
		}
	}
	//Eintrag konnte nicht mehr aufgenommen werden
	DEBUG("ARP entry tabelle voll!\r\n");
	return;
}

//----------------------------------------------------------------------------
//Diese Routine such anhand der IP den ARP eintrag
char arp_entry_search (unsigned long dest_ip)
{
	for (unsigned char b = 0;b<MAX_ARP_ENTRY;b++)
	{
		if(arp_entry[b].arp_t_ip == dest_ip)
		{
			return(b);
		}
	}
	return (MAX_ARP_ENTRY);
}

//----------------------------------------------------------------------------
//Diese Routine Erzeugt ein neuen Ethernetheader
void new_eth_header (unsigned char *buffer,unsigned long dest_ip)
{
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&buffer[ETHER_OFFSET];
	
	unsigned char b = arp_entry_search (dest_ip);
	
	if (b != MAX_ARP_ENTRY) //Eintrag gefunden wenn ungleich
	{
		for(unsigned char a = 0; a < 6; a++)
		{
			//MAC Destadresse wird geschrieben mit MAC Sourceadresse
			ethernet->EnetPacketDest[a] = arp_entry[b].arp_t_mac[a];
			//Meine MAC Adresse wird in Sourceadresse geschrieben
			ethernet->EnetPacketSrc[a] = mymac[a];
		}
		return;
	}
	DEBUG("ARP Eintrag nicht gefunden*\r\n");
	
	for(unsigned char a = 0; a < 6; a++)
	{
		//MAC Destadresse wird geschrieben mit MAC Sourceadresse
		ethernet->EnetPacketDest[a] = 0xFF;
		//Meine MAC Adresse wird in Sourceadresse geschrieben
		ethernet->EnetPacketSrc[a] = mymac[a];
	}
	return;

}

//----------------------------------------------------------------------------
//Diese Routine Antwortet auf ein ARP Packet
void arp_reply (void)
{
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&eth_buffer[ETHER_OFFSET];

	struct ARP_Header *arp;
	arp = (struct ARP_Header *)&eth_buffer[ARP_OFFSET];

	//2 Byte Hardware Typ: Enthält den Code für Ethernet
	if(		arp->ARP_HWType == 0x0100 &&

			//2 Byte Protokoll Typ: Enthält den Code für IP
			arp->ARP_PRType == 0x0008  &&
		
			//1Byte Länge der Hardwareadresse:Enthält 6 für 6 Byte MAC Addresse
			arp->ARP_HWLen == 0x06 && 
		
			//1Byte Länge der Protokolladresse:Enthält 4 für 4 Byte Adressen 
			arp->ARP_PRLen == 0x04 &&
		
			//Ist das ARP Packet für meine IP Addresse bestimmt
			//Vergleiche ARP Target IP Adresse mit meiner IP
			arp->ARP_TIPAddr == *((unsigned long*)&myip[0]))
	{
		//Operation handelt es sich um eine anfrage
		if (arp->ARP_Op == 0x0100)
		{
			//Rechner Eingetragen wenn noch nicht geschehen?
			arp_entry_add(); 
					
			new_eth_header (eth_buffer, arp->ARP_SIPAddr); //Erzeugt ein neuen Ethernetheader
			
			ethernet->EnetPacketType = 0x0608; //Nutzlast 0x0800=IP Datagramm;0x0806 = ARP
			
			unsigned char b = arp_entry_search (arp->ARP_SIPAddr);
			if (b != MAX_ARP_ENTRY) //Eintrag gefunden wenn ungleich
			{
				for(unsigned char a = 0; a < 6; a++)
				{
					//ARP MAC Targetadresse wird geschrieben mit ARP Sourceadresse
					arp->ARP_THAddr[a] = arp_entry[b].arp_t_mac[a];
					//ARP MAC Sourceadresse wird geschrieben mit My MAC Adresse
					arp->ARP_SHAddr[a] = mymac[a];
				}
			}
			else
			{
			DEBUG("ARP Eintrag nicht gefunden\r\n");//Unwarscheinlich das das jemals passiert!
			}
			
			//ARP operation wird auf 2 gesetzt damit der andere merkt es ist ein ECHO	
			arp->ARP_Op = 0x0200;	
			//ARP Target IP Adresse wird geschrieben mit ARP Source IP Adresse 
			arp->ARP_TIPAddr = arp->ARP_SIPAddr;
			//Meine IP Adresse wird in ARP Source IP Adresse geschrieben
			arp->ARP_SIPAddr = *((unsigned long *)&myip[0]);
			
			//Nun ist das ARP-Packet fertig zum Senden !!!
			//Sendet das erzeugte ARP Packet 
			ETH_PACKET_SEND(ARP_REPLY_LEN,eth_buffer);
			return;
		}
		//es handelt sich um ein REPLY von einem anderen Client
		if (arp->ARP_Op == 0x0200)
		{
			//Rechner Eingetragen wenn noch nicht geschehen?
			arp_entry_add();
			
			DEBUG("ARP REPLY EMPFANGEN!\r\n");
		}
	}
	return;
}

//----------------------------------------------------------------------------
//Diese Routine erzeugt ein ARP Request
char arp_request (unsigned long dest_ip)
{
	unsigned char buffer[ARP_REQUEST_LEN];
	unsigned char index = 0;
	unsigned long dest_ip_store;

	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&buffer[ETHER_OFFSET];

	struct ARP_Header *arp;
	arp = (struct ARP_Header *)&buffer[ARP_OFFSET];

	dest_ip_store = dest_ip;

	if ((dest_ip & (*((unsigned long *)&netmask[0])))==
		((*((unsigned long *)&myip[0]))&(*((unsigned long *)&netmask[0]))))
	{
		DEBUG("MY NETWORK!\r\n");
	}
	else
	{
		DEBUG("ROUTING!\r\n");
		dest_ip = (*((unsigned long *)&router_ip[0]));
	}

	//Nutzlast 0x0800=IP Datagramm;0x0806 = ARP
	ethernet->EnetPacketType = 0x0608; 
	
	new_eth_header (buffer,dest_ip);
	
	//Meine IP Adresse wird in ARP Source IP Adresse geschrieben
	arp->ARP_SIPAddr = *((unsigned long *)&myip[0]);
	
	//Ziel IP wird in Dest IP geschrieben
	arp->ARP_TIPAddr = dest_ip; 
	
	for(unsigned char count = 0; count < 6; count++)
	{
		  arp->ARP_SHAddr[count] = mymac[count];
		  arp->ARP_THAddr[count] = 0x00;
	}
	
	arp->ARP_HWType = 0x0100;
	arp->ARP_PRType = 0x0008;
	arp->ARP_HWLen 	= 0x06;
	arp->ARP_PRLen 	= 0x04;
	arp->ARP_Op 	= 0x0100;

	//Nun ist das ARP-Packet fertig zum Senden !!!
	//Sendet das erzeugte ARP Packet 
	ETH_PACKET_SEND(ARP_REQUEST_LEN, buffer);
	
	for(unsigned char count = 0;count<20;count++)
	{
		unsigned char index_tmp = arp_entry_search(dest_ip_store);
		index = arp_entry_search(dest_ip);
		if (index < MAX_ARP_ENTRY || index_tmp < MAX_ARP_ENTRY)
		{
			DEBUG("ARP EINTRAG GEFUNDEN!\r\n");
			if (index_tmp < MAX_ARP_ENTRY) return(1);//OK
			arp_entry[index].arp_t_ip = dest_ip_store;
			return(1);//OK
		}
		for(unsigned long a=0;a<10000;a++){asm("nop");};
		eth_get_data();
		DEBUG("**KEINEN ARP EINTRAG GEFUNDEN**\r\n");
	}
	return(0);//keine Antwort
}

//----------------------------------------------------------------------------
//Diese Routine erzeugt ein neues ICMP Packet
void icmp_send (unsigned long dest_ip, unsigned char icmp_type, 
                unsigned char icmp_code, unsigned int icmp_sn, 
                unsigned int icmp_id)
{
	//Variablen zur Berechnung der Checksumme
	unsigned int result16;

	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];

	struct ICMP_Header *icmp;
	icmp = (struct ICMP_Header *)&eth_buffer[ICMP_OFFSET];

	//Das ist ein Echo Reply Packet
	icmp->ICMP_Type   = icmp_type;
	icmp->ICMP_Code   = icmp_code;
	icmp->ICMP_Id     = icmp_id;
	icmp->ICMP_SeqNum = icmp_sn;
	
	//Berechnung der ICMP Checksumme
	//Alle Daten im ICMP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	icmp->ICMP_Cksum = 0x0000;

	//Hier wird erstmal der IP Header neu erstellt

	ip->IP_Pktlen = 0x5400;                 // 0x54 = 84 
 	ip->IP_Proto  = PROT_ICMP;
	make_ip_header (eth_buffer,dest_ip);

	//Berechnung der ICMP Header lÃ¤nge
	result16 = LBBL_ENDIAN_INT(ip->IP_Pktlen);
	result16 = result16 - ((ip->IP_Vers_Len & 0x0F) << 2);

	//pointer wird auf das erste Packet im ICMP Header gesetzt
	//jetzt wird die Checksumme berechnet
	result16 = checksum (&icmp->ICMP_Type, result16, 0);
	
	//schreibt Checksumme ins Packet
	icmp->ICMP_Cksum = LBBL_ENDIAN_INT(result16);
	
	//Sendet das erzeugte ICMP Packet 

    ETH_PACKET_SEND(ICMP_REPLY_LEN,eth_buffer);
}

//----------------------------------------------------------------------------
//Diese Routine erzeugt eine Cecksumme
unsigned int checksum (unsigned char *pointer,unsigned int result16,unsigned long result32)
{
	unsigned int result16_1 = 0x0000;
	unsigned char DataH;
	unsigned char DataL;
	
	//Jetzt werden alle Packete in einer While Schleife addiert
	while(result16 > 1)
	{
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer++;

		//schreibt Inhalt Pointer nach DATAL danach inc Pointer
		DataL=*pointer++;

		//erzeugt Int aus Data L und Data H
		result16_1 = ((DataH << 8)+DataL);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
		//decrimiert Länge von TCP Headerschleife um 2
		result16 -=2;
	}

	//Ist der Wert result16 ungerade ist DataL = 0
	if(result16 > 0)
	{
		//schreibt Inhalt Pointer nach DATAH danach inc Pointer
		DataH=*pointer;
		//erzeugt Int aus Data L ist 0 (ist nicht in der Berechnung) und Data H
		result16_1 = (DataH << 8);
		//Addiert packet mit vorherigen
		result32 = result32 + result16_1;
	}
	
	//Komplementbildung (addiert Long INT_H Byte mit Long INT L Byte)
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));
	result32 = ((result32 & 0x0000FFFF)+ ((result32 & 0xFFFF0000) >> 16));	
	result16 =~(result32 & 0x0000FFFF);
	
	return (result16);
}

//----------------------------------------------------------------------------
//Diese Routine erzeugt ein IP Packet
void make_ip_header (unsigned char *buffer,unsigned long dest_ip)
{

	//------------------------------------------------------------------------
	struct Ethernet_Header *ethernet;
	ethernet = (struct Ethernet_Header *)&buffer[ETHER_OFFSET];
	new_eth_header (buffer, dest_ip); //Erzeugt ein neuen Ethernetheader
	ethernet->EnetPacketType = 0x0008; //Nutzlast 0x0800=IP
	struct IP_Header *ip;
	//------------------------------------------------------------------------
	
	//Variablen zur Berechnung der Checksumme
	unsigned int result16;
	
	ip = (struct IP_Header *)&buffer[IP_OFFSET];
	
	//don't fragment
	ip->IP_Frag_Offset = 0x0040;
	
	//max. hops
	ip->IP_ttl = 128;
	IP_id_counter++;
	ip->IP_Id = LBBL_ENDIAN_INT(IP_id_counter);
	ip->IP_Vers_Len = 0x45;	//4 BIT Die Versionsnummer von IP, 
							//meistens also 4 + 4Bit Headergröße 
	ip->IP_Tos = 0x00;
	
	//unsigned int	IP_Pktlen;		//16 Bit Komplette Läng des IP Datagrams in Bytes
	//unsigned char	IP_Proto;		//Zeigt das höherschichtige Protokoll an 
									//(TCP, UDP, ICMP)
	
	//IP Destadresse wird geschrieben mit IP Sourceadresse 
	//das packet soll ja zurückgeschickt werden :-)
	ip->IP_Destaddr	= dest_ip;
	ip->IP_Srcaddr	= *((unsigned long *)&myip[0]);
		
	//Berechnung der IP Checksumme
	//Alle Daten im IP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	ip->IP_Hdr_Cksum = 0x0000;
	
	//Berechnung der IP Header länge	
	result16 = (ip->IP_Vers_Len & 0x0F) << 2;

	//jetzt wird die Checksumme berechnet
	result16 = checksum (&ip->IP_Vers_Len, result16, 0);

	//schreibt Checksumme ins Packet
	ip->IP_Hdr_Cksum = LBBL_ENDIAN_INT(result16);
	return;
}

//----------------------------------------------------------------------------
//Diese Routine verwaltet TCP-Einträge
void tcp_entry_add (unsigned char *buffer)
{
	unsigned long result32;

	struct TCP_Header *tcp;
	tcp = (struct TCP_Header *)&buffer[TCP_OFFSET];
	
	struct IP_Header *ip;
	ip = (struct IP_Header *)&buffer[IP_OFFSET];
	
	//Eintrag schon vorhanden?
	for (unsigned char index = 0;index<(MAX_TCP_ENTRY);index++)
	{
		if(	tcp_entry[index].ip == ip->IP_Srcaddr &&
			tcp_entry[index].src_port == tcp->TCP_SrcPort)
		{
		//Eintrag gefunden Time refresh
		tcp_entry[index].ack_counter = tcp->TCP_Acknum;
		tcp_entry[index].seq_counter = tcp->TCP_Seqnum;
		tcp_entry[index].status = tcp->TCP_HdrFlags;
		if (tcp_entry[index].time!=TCP_TIME_OFF)
		{
			tcp_entry[index].time = TCP_MAX_ENTRY_TIME;
		}
		result32 = LBBL_ENDIAN_INT(ip->IP_Pktlen) - IP_VERS_LEN - ((tcp->TCP_Hdrlen& 0xF0) >>2);
		result32 = result32 + LBBL_ENDIAN_LONG(tcp_entry[index].seq_counter);
		tcp_entry[index].seq_counter = LBBL_ENDIAN_LONG(result32);
		
		DEBUG("TCP Entry gefunden %i\r\n",index);
		return;
		}
	}
	
	//Freien Eintrag finden
	for (unsigned char index = 0;index<(MAX_TCP_ENTRY);index++)
	{
		if(tcp_entry[index].ip == 0)
		{
			tcp_entry[index].ip = ip->IP_Srcaddr;
			tcp_entry[index].src_port = tcp->TCP_SrcPort;
			tcp_entry[index].dest_port = tcp->TCP_DestPort;
			tcp_entry[index].ack_counter = tcp->TCP_Acknum;
			tcp_entry[index].seq_counter = tcp->TCP_Seqnum;
			tcp_entry[index].status = tcp->TCP_HdrFlags;
			tcp_entry[index].app_status = 0;
			tcp_entry[index].time = TCP_MAX_ENTRY_TIME;
			tcp_entry[index].error_count = 0;
			tcp_entry[index].first_ack = 0;
			DEBUG("TCP Entry neuer Eintrag %i\r\n",index);
			return;	
		}
	}
	//Eintrag konnte nicht mehr aufgenommen werden
	DEBUG("Server Busy (NO MORE CONNECTIONS)!\r\n");
	return;
}

//----------------------------------------------------------------------------
//Diese Routine sucht den etntry eintrag
char tcp_entry_search (unsigned long dest_ip,unsigned int SrcPort)
{
	for (unsigned char index = 0;index<MAX_TCP_ENTRY;index++)
	{
		if(	tcp_entry[index].ip == dest_ip &&
			tcp_entry[index].src_port == SrcPort)
		{
			return(index);
		}
	}
	return (MAX_TCP_ENTRY);
}

//----------------------------------------------------------------------------
//Diese Routine verwaltet die UDP Ports
void udp_socket_process(void)
{
	unsigned char port_index = 0;
		
	struct UDP_Header *udp;
	udp = (struct UDP_Header *)&eth_buffer[UDP_OFFSET];

	//UDP DestPort mit Portanwendungsliste durchführen
	while (UDP_PORT_TABLE[port_index].port && UDP_PORT_TABLE[port_index].port!=(LBBL_ENDIAN_INT(udp->udp_DestPort)))
	{ 
		port_index++;
	}
	
	// Wenn index zu gross, dann beenden keine vorhandene Anwendung für den Port
	if (!UDP_PORT_TABLE[port_index].port)
	{ 
		//Keine vorhandene Anwendung eingetragen! (ENDE)
		DEBUG("UDP Keine Anwendung gefunden!\r\n");
		return;
	}

	//zugehörige Anwendung ausführen
	UDP_PORT_TABLE[port_index].fp(0); 
	return;
}

//----------------------------------------------------------------------------
//Diese Routine Erzeugt ein neues UDP Packet
void create_new_udp_packet(	unsigned int data_length,
							unsigned int src_port,
							unsigned int dest_port,
							unsigned long dest_ip)
{
	DEBUG("UDP wird gesendet!\r\n");
    unsigned int result16;
	unsigned long result32;

	struct UDP_Header *udp;
	udp = (struct UDP_Header *)&eth_buffer[UDP_OFFSET];
	
	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
	
	udp->udp_SrcPort = LBBL_ENDIAN_INT(src_port);
	udp->udp_DestPort = LBBL_ENDIAN_INT(dest_port);

	//UDP Packetlänge
	data_length = UDP_HDR_LEN + data_length;
	udp->udp_Hdrlen = LBBL_ENDIAN_INT(data_length);
	//IP Headerlänge + UDP Headerlänge
	data_length = IP_VERS_LEN + data_length;	
	//Hier wird erstmal der IP Header neu erstellt
	ip->IP_Pktlen = LBBL_ENDIAN_INT(data_length);
	data_length += ETH_HDR_LEN;
	ip->IP_Proto = PROT_UDP;
	make_ip_header (eth_buffer,dest_ip);

	//Alle Daten im UDP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	udp->udp_Chksum = 0;
	
	//Berechnet Headerlänge und Addiert Pseudoheaderlänge 2XIP = 8
	result16 = LBBL_ENDIAN_INT(ip->IP_Pktlen) + 8;
	result16 = result16 - ((ip->IP_Vers_Len & 0x0F) << 2);
	result32 = result16 + 0x09;
	
	//Routine berechnet die Checksumme
	result16 = checksum ((&ip->IP_Vers_Len+12), result16, result32);
	udp->udp_Chksum = LBBL_ENDIAN_INT(result16);

	//Sendet das erzeugte UDP Packet 
    ETH_PACKET_SEND(data_length,eth_buffer);
	return;
}
//----------------------------------------------------------------------------
//Diese Routine verwaltet die TCP Ports
void tcp_socket_process(void)
{
	unsigned char index = 0;
	unsigned char port_index = 0;
	unsigned long result32 = 0;

	struct TCP_Header *tcp;
	tcp = (struct TCP_Header *)&eth_buffer[TCP_OFFSET];

	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];

	//TCP DestPort mit Portanwendungsliste durchführen
	while (TCP_PORT_TABLE[port_index].port && TCP_PORT_TABLE[port_index].port!=(LBBL_ENDIAN_INT(tcp->TCP_DestPort)))
	{ 
		port_index++;
	}
	
	// Wenn index zu gross, dann beenden keine vorhandene Anwendung für Port
	//Geht von einem Client was aus? Will eine Clientanwendung einen Port öffnen?
	if (!TCP_PORT_TABLE[port_index].port)
	{ 
		//Keine vorhandene Anwendung eingetragen! (ENDE)
		DEBUG("TCP Keine Anwendung gefunden!\r\n");
		return;
	}	
	
	//Server öffnet Port
	if((tcp->TCP_HdrFlags & SYN_FLAG) && (tcp->TCP_HdrFlags & ACK_FLAG))
	{	
		//Nimmt Eintrag auf da es eine Client - Anwendung für den Port gibt
		tcp_entry_add (eth_buffer);
		//War der Eintrag erfolgreich?
		index = tcp_entry_search (ip->IP_Srcaddr,tcp->TCP_SrcPort);
		if (index >= MAX_TCP_ENTRY) //Eintrag gefunden wenn ungleich
		{
			DEBUG("TCP Eintrag nicht erfolgreich!\r\n");
			return;
		}
	
		tcp_entry[index].time = MAX_TCP_PORT_OPEN_TIME;
		DEBUG("TCP Port wurde vom Server geöffnet STACK:%i\r\n",index);
		result32 = LBBL_ENDIAN_LONG(tcp_entry[index].seq_counter) + 1;
		tcp_entry[index].seq_counter = LBBL_ENDIAN_LONG(result32);
		tcp_entry[index].status =  ACK_FLAG;
		create_new_tcp_packet(0,index);
		//Server Port wurde geöffnet App. kann nun daten senden!
		tcp_entry[index].app_status = 1;
		return;
	}
	
	//Verbindungsaufbau nicht für Anwendung bestimmt
	if(tcp->TCP_HdrFlags == SYN_FLAG)
	{
		//Nimmt Eintrag auf da es eine Server - Anwendung für den Port gibt
		tcp_entry_add (eth_buffer);
		//War der Eintrag erfolgreich?
		index = tcp_entry_search (ip->IP_Srcaddr,tcp->TCP_SrcPort);
		if (index >= MAX_TCP_ENTRY) //Eintrag gefunden wenn ungleich
		{
			DEBUG("TCP Eintrag nicht erfolgreich!\r\n");
			return;
		}
	
		DEBUG("TCP New SERVER Connection! STACK:%i\r\n",index);
		
		tcp_entry[index].status =  ACK_FLAG | SYN_FLAG;
		create_new_tcp_packet(0,index);
		return;
	}

	//Packeteintrag im TCP Stack finden!
	index = tcp_entry_search (ip->IP_Srcaddr,tcp->TCP_SrcPort);
	
	if (index >= MAX_TCP_ENTRY) //Eintrag nicht gefunden
	{
		DEBUG("TCP Eintrag nicht gefunden\r\n");
		tcp_entry_add (eth_buffer);
		if(tcp->TCP_HdrFlags & FIN_FLAG || tcp->TCP_HdrFlags & RST_FLAG)
		{	
			result32 = LBBL_ENDIAN_LONG(tcp_entry[index].seq_counter) + 1;
			tcp_entry[index].seq_counter = LBBL_ENDIAN_LONG(result32);
			
			if (tcp_entry[index].status & FIN_FLAG)
			{
				tcp_entry[index].status = ACK_FLAG;
				create_new_tcp_packet(0,index);
			}
			tcp_index_del(index);
			DEBUG("TCP-Stack Eintrag gelöscht! STACK:%i\r\n",index);
			return;
		}
		return;
	}


	//Refresh des Eintrages
	tcp_entry_add (eth_buffer);
	
	//Host will verbindung beenden!
	if(tcp_entry[index].status & FIN_FLAG || tcp_entry[index].status & RST_FLAG)
	{	
		result32 = LBBL_ENDIAN_LONG(tcp_entry[index].seq_counter) + 1;
		tcp_entry[index].seq_counter = LBBL_ENDIAN_LONG(result32);
		
		if (tcp_entry[index].status & FIN_FLAG)
		{
			// Ende der Anwendung mitteilen !
			TCP_PORT_TABLE[port_index].fp(index);

			tcp_entry[index].status = ACK_FLAG;
			create_new_tcp_packet(0,index);
		}
		tcp_index_del(index);
		DEBUG("TCP-Stack Eintrag gelöscht! STACK:%i\r\n",index);
		return;
	}
	
	//Daten für Anwendung PSH-Flag gesetzt?
	if((tcp_entry[index].status & PSH_FLAG) && 
		(tcp_entry[index].status & ACK_FLAG))
	{
		//zugehörige Anwendung ausführen
		tcp_entry[index].app_status++;	
		tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
		TCP_PORT_TABLE[port_index].fp(index); 
		return;
	}
	
	//Empfangene Packet wurde bestätigt keine Daten für Anwendung
	//z.B. nach Verbindungsaufbau (SYN-PACKET)
	if((tcp_entry[index].status & ACK_FLAG) && (tcp_entry[index].first_ack == 0))
	{
		//keine weitere Aktion
		tcp_entry[index].first_ack = 1;
		return;
	}
	
	//Empfangsbestätigung für ein von der Anwendung gesendetes Packet (ENDE)
	if((tcp_entry[index].status & ACK_FLAG) && (tcp_entry[index].first_ack == 1))
	{
		//ACK für Verbindungs abbau
		if(tcp_entry[index].app_status == 0xFFFF)
		{
			return;
		}

		//zugehörige Anwendung ausführen
		tcp_entry[index].status =  ACK_FLAG;
		tcp_entry[index].app_status++;
		TCP_PORT_TABLE[port_index].fp(index);
		return;
	}
	return;
}
//----------------------------------------------------------------------------
//Diese Routine Erzeugt ein neues TCP Packet
void create_new_tcp_packet(unsigned int data_length,unsigned char index)
{
	unsigned int result16;
	unsigned long result32;

	struct TCP_Header *tcp;
	tcp = (struct TCP_Header *)&eth_buffer[TCP_OFFSET];
	
	struct IP_Header *ip;
	ip = (struct IP_Header *)&eth_buffer[IP_OFFSET];
	
	tcp->TCP_SrcPort = tcp_entry[index].dest_port;
	tcp->TCP_DestPort = tcp_entry[index].src_port;
	tcp->TCP_UrgentPtr = 0;
	tcp->TCP_Window = LBBL_ENDIAN_INT(MAX_WINDOWS_SIZE);
	tcp->TCP_Hdrlen = 0x50;
	
	DEBUG("TCP SrcPort %4i\r\n", LBBL_ENDIAN_INT(tcp->TCP_SrcPort));

	result32 = LBBL_ENDIAN_LONG(tcp_entry[index].seq_counter); 

	tcp->TCP_HdrFlags = tcp_entry[index].status;

	//Verbindung wird aufgebaut
	if(tcp_entry[index].status & SYN_FLAG)
	{
		result32 ++;

		// MSS-Option (siehe RFC 879) wil.
		eth_buffer[TCP_DATA_START] = 2;
		eth_buffer[TCP_DATA_START+1] = 4;
		eth_buffer[TCP_DATA_START+2] = (MAX_WINDOWS_SIZE >> 8) & 0xff;
		eth_buffer[TCP_DATA_START+3] = MAX_WINDOWS_SIZE & 0xff;
		data_length = 0x04;
		tcp->TCP_Hdrlen = 0x60;
	}
	
	tcp->TCP_Acknum =  LBBL_ENDIAN_LONG(result32);
	tcp->TCP_Seqnum = tcp_entry[index].ack_counter;

	//IP Headerlänge + TCP Headerlänge
	unsigned int bufferlen = IP_VERS_LEN + TCP_HDR_LEN + data_length;	
	//Hier wird erstmal der IP Header neu erstellt
	ip->IP_Pktlen = LBBL_ENDIAN_INT(bufferlen);
	bufferlen += ETH_HDR_LEN;
	ip->IP_Proto = PROT_TCP;
	make_ip_header (eth_buffer,tcp_entry[index].ip);

	//Alle Daten im TCP Header werden addiert checksum wird deshalb
	//ersteinmal auf null gesetzt
	tcp->TCP_Chksum = 0;
	
	//Berechnet Headerlänge und Addiert Pseudoheaderlänge 2XIP = 8
	result16 = LBBL_ENDIAN_INT(ip->IP_Pktlen) + 8;
	result16 = result16 - ((ip->IP_Vers_Len & 0x0F) << 2);
	result32 = result16 - 2;
	
	//Routine berechnet die Checksumme
	result16 = checksum ((&ip->IP_Vers_Len+12), result16, result32);
	tcp->TCP_Chksum = LBBL_ENDIAN_INT(result16);

	//Sendet das erzeugte TCP Packet 
    ETH_PACKET_SEND(bufferlen,eth_buffer);
	//Für Retransmission
	tcp_entry[index].status =  0;
	return;
}

//----------------------------------------------------------------------------
//Diese Routine schließt einen offenen TCP-Port
void tcp_Port_close (unsigned char index)
{
	DEBUG("Port wird im TCP Stack geschlossen STACK:%i\r\n",index);
	tcp_entry[index].app_status = 0xFFFF;
	tcp_entry[index].status =  ACK_FLAG | FIN_FLAG;
	create_new_tcp_packet(0,index);
	return;
}

//----------------------------------------------------------------------------
//Diese Routine findet die Anwendung anhand des TCP Ports (fas find and start)
void find_and_start (unsigned char index)
{
	unsigned char port_index = 0;
	//Port mit Anwendung in der Liste suchen
	while (TCP_PORT_TABLE[port_index].port && 
			TCP_PORT_TABLE[port_index].port!=(LBBL_ENDIAN_INT(tcp_entry[index].dest_port)))
	{ 
		port_index++;
	}

	//zugehörige Anwendung ausführen (Packet senden wiederholen)
	TCP_PORT_TABLE[port_index].fp(index);
	return;
}

//----------------------------------------------------------------------------
//Diese Routine öffnet einen TCP-Port
void tcp_port_open (unsigned long dest_ip,unsigned int port_dst,unsigned int port_src)
{
	unsigned char index;
	ETH_INT_DISABLE;
	DEBUG("Oeffen eines Ports mit Server\r\n");
	
	//Freien Eintrag finden
	for (index = 0;index<MAX_TCP_ENTRY;index++)
	{
		if(tcp_entry[index].ip == 0)
		{
			tcp_index_del(index);
			tcp_entry[index].ip = dest_ip;
			tcp_entry[index].src_port = port_dst;
			tcp_entry[index].dest_port = port_src;
			tcp_entry[index].ack_counter = 1234;
			tcp_entry[index].seq_counter = 2345;
			tcp_entry[index].time = MAX_TCP_PORT_OPEN_TIME;
			DEBUG("TCP Open neuer Eintrag %i\r\n",index);
			break;
		}
	}
	if (index >= MAX_TCP_ENTRY)
	{
		//Eintrag konnte nicht mehr aufgenommen werden
		DEBUG("Busy (NO MORE CONNECTIONS)!\r\n");
	}
	
	tcp_entry[index].status =  SYN_FLAG;
	create_new_tcp_packet(0,index);
	ETH_INT_ENABLE;
	return;
}

//----------------------------------------------------------------------------
//Diese Routine löscht einen Eintrag
void tcp_index_del (unsigned char index)
{
	if (index<MAX_TCP_ENTRY + 1)
	{
		tcp_entry[index].ip = 0;
		tcp_entry[index].src_port = 0;
		tcp_entry[index].dest_port = 0;
		tcp_entry[index].ack_counter = 0;
		tcp_entry[index].seq_counter = 0;
		tcp_entry[index].status = 0;
		tcp_entry[index].app_status = 0;
		tcp_entry[index].time = 0;
		tcp_entry[index].first_ack = 0;
	}
	return;
}