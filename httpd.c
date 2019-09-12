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
#include "httpd.h"
#include "webpage.h"
#include "lcd.h"

#if USE_SERVO
	#include "camera/servo.h"
#endif

struct http_table http_entry[MAX_TCP_ENTRY];

//Hier wird das codierte Passwort aus config.h gespeichert.
unsigned char http_auth_passwort[20];

unsigned char post_in[5] = {'O','U','T','='};
unsigned char post_ready[5] = {'S','U','B','='};
unsigned char PORT_tmp_C = 0;
unsigned char PORT_tmp_D = 0;

char time_string_Buffer[10];

// unsigned char internal_message[10];

// -> main.c (Speicherplatz für 1-wire Sensorwerte)
extern volatile int16_t ow_array[MAXSENSORS];

PROGMEM char const http_header1[]={	"HTTP/1.0 200 Document follows\r\n"
								"Server: AVR_WEB_Switch\r\n"
								"Content-Type: text/html\r\n\r\n"};

PROGMEM char const http_header2[]={	"HTTP/1.0 200 Document follows\r\n"
								"Server: AVR_WEB_Switch\r\n"
								"Content-Type: image/jpg\r\n\r\n"};

PROGMEM char const http_header3[]={	"HTTP/1.0 401 Unauthorized\r\n"
								"Server: AVR_WEB_Switch\r\n"
								"WWW-Authenticate: Basic realm=\"NeedPassword\""
								"\r\nContent-Type: text/html\r\n\r\n"};

PROGMEM char const http_header4[]={	"HTTP/1.0 200 Document follows\r\n"
								"Server: AVR_WEB_Switch\r\n"
								"Content-Type: application/json\r\n\r\n"};

//----------------------------------------------------------------------------
//Kein Zugriff Seite bei keinem Passwort
PROGMEM char const Page0[] = {"401 Unauthorized%END"};

unsigned char rx_header_end[5] = {"\r\n\r\n\0"};

//----------------------------------------------------------------------------
//Initialisierung des Httpd Testservers
void httpd_init (void)
{
	//HTTP_AUTH_STRING 
	decode_base64((unsigned char*)HTTP_AUTH_STRING,http_auth_passwort);

	//Serverport und Anwendung eintragen
	add_tcp_app (HTTPD_PORT, (void(*)(unsigned char))httpd);
}
   
//----------------------------------------------------------------------------
//http Testserver
void httpd (unsigned char index)
{
    //Verbindung wurde abgebaut!
    if (tcp_entry[index].status & FIN_FLAG)
    {
        return;
    }

	//Allererste Aufruf des Ports für diese Anwendung
	//HTTPD_Anwendungsstack löschen
	if(tcp_entry[index].app_status==1)
	{
		httpd_stack_clear(index);
	}
	
	//HTTP wurde bei dieser Verbindung zum ersten mal aufgerufen oder
	//HTML Header Retransmission!
	if (tcp_entry[index].app_status <= 2)
	{	
		httpd_header_check (index);
		return;
	}
	
	//Der Header wurde gesendet und mit ACK bestätigt (tcp_entry[index].app_status+1)
	//war das HTML Packet fertig, oder müssen weitere Daten gesendet werden, oder Retransmission?
	if (tcp_entry[index].app_status > 2 && tcp_entry[index].app_status < 0xFFFE)
	{
		httpd_data_send (index);
		return;
	}
	
	//Verbindung kann geschlossen werden! Alle HTML Daten wurden gesendet TCP Port kann
	//geschlossen werden (tcp_entry[index].app_status >= 0xFFFE)!!
	if (tcp_entry[index].app_status >= 0xFFFE)
	{
		tcp_entry[index].app_status = 0xFFFE;
		tcp_Port_close(index);
		return;
	}
	return;
}

//----------------------------------------------------------------------------
//HTTPD_STACK löschen
void httpd_stack_clear (unsigned char index)
{
	http_entry[index].http_header_type =0;
	http_entry[index].first_switch = 0;
	http_entry[index].http_auth = HTTP_AUTH_DEFAULT;
	http_entry[index].new_page_pointer = 0;
	http_entry[index].old_page_pointer = 0;
	http_entry[index].post = 0;
	http_entry[index].auth_ptr = http_auth_passwort;
	http_entry[index].post_ptr = post_in;
	http_entry[index].post_ready_ptr = post_ready;
	http_entry[index].hdr_end_pointer = rx_header_end;
	#if USE_CAM
	http_entry[index].cam = 0;
	#endif //USE_CAM				
	HTTP_DEBUG("\r\n**** NEUE HTTP ANFORDERUNG ****\r\n\r\n");	
	return;
}

//----------------------------------------------------------------------------
//Eintreffenden Header vom Client checken
void httpd_header_check (unsigned char index)
{
	unsigned int a = 0;
	
	if(strcasestr_P((char*)&eth_buffer[TCP_DATA_START_VAR],PSTR("POST"))!=0)
		{
		http_entry[index].post = 1;
		}
	
	//finden der Authorization und das Ende im Header auch über mehrere Packete hinweg!!	
	if(*http_entry[index].hdr_end_pointer != 0)
	{		
		for(a=TCP_DATA_START_VAR;a<(TCP_DATA_END_VAR);a++)
		{	
			HTTP_DEBUG("%c",eth_buffer[a]);
			
			if(!http_entry[index].http_auth) 
			{
				if (eth_buffer[a] != *http_entry[index].auth_ptr++)
				{
					http_entry[index].auth_ptr = http_auth_passwort;
				}
				if(*http_entry[index].auth_ptr == 0) 
				{
					http_entry[index].http_auth = 1;
					HTTP_DEBUG("  <---LOGIN OK!--->\r\n");
				}
			}
			
			if (eth_buffer[a] != *http_entry[index].hdr_end_pointer++)
			{
				http_entry[index].hdr_end_pointer = rx_header_end;
			}
			
			//Das Headerende wird mit (CR+LF+CR+LF) angezeigt!
			if(*http_entry[index].hdr_end_pointer == 0) 
			{
				HTTP_DEBUG("<---HEADER ENDE ERREICHT!--->\r\n");
				break;
			}
		}
	}
	
	//Einzelne Postpacket (z.B. bei firefox)
	if(http_entry[index].http_auth && http_entry[index].post == 1)
	{


/*
// ---
	char* nPos=strstr_P((char*)&eth_buffer[TCP_DATA_START], PSTR("IMES="));

         // die nächsten 3 Zeichen nach "IMES=" werden übergeben:
         strlcpy(internal_message, nPos+5,3); 
         for(int i=0;i<strlen(internal_message);i++)
         {   
            if(internal_message[i]=='&')
               {
                  internal_message[i]=0x00;
                  break;
               }
	
	// RoBue:
	// Testausgabe auf UART
	// &b: Umwandlung von ASCII -> Zahlenwert
	usart_write("%i",internal_message[i]&0b00001111);
         }

// ---
*/


		uint8_t schaltautomatik = 0;

		for(a = TCP_DATA_START_VAR;a<(TCP_DATA_END_VAR);a++)
		{	
			// Schaltanweisung finden!
			if (eth_buffer[a] != *http_entry[index].post_ptr++)
			{
				http_entry[index].post_ptr = post_in;
			}
			if(*http_entry[index].post_ptr == 0) 
			{
				switch (eth_buffer[a+1])
				  {

				// RoBue:
				// Schaltanweisungen von PORTC1-7 auslesen 
				// und in PORT_tmp_C zusammenfügen
				// checkbox-value-Wert A-H in webpage.h
				case ('A'):
					PORT_tmp_C = PORT_tmp_C + 1;
					break;
				
				case ('B'):
					PORT_tmp_C = PORT_tmp_C + 2;
					break;
				
				case ('C'):
					PORT_tmp_C = PORT_tmp_C + 4;
					break;
					  
				case ('D'):
					PORT_tmp_C = PORT_tmp_C + 8;
					break;
					  
				case ('E'):
					PORT_tmp_C = PORT_tmp_C + 16;
					break;
					
				case ('F'):
					PORT_tmp_C = PORT_tmp_C + 32;
					break;
					  
				case ('G'):
					PORT_tmp_C = PORT_tmp_C + 64;
					break;
					  
				case ('H'):
					PORT_tmp_C = PORT_tmp_C + 128;
					break;


#if USE_PORTD_SCHALT
				// RoBue:
				// Schaltanweisungen von PORTD2-7 auslesen 
				// und in PORT_tmp_D zusammenfügen
				// checkbox-value-wert I-P in webpage.h
				case ('I'):
					PORT_tmp_D = PORT_tmp_D + 1;
					break;
				
				case ('J'):
					PORT_tmp_D = PORT_tmp_D + 2;
					break;
				
				case ('K'):
					PORT_tmp_D = PORT_tmp_D + 4;
					break;
					  
				case ('L'):
					PORT_tmp_D = PORT_tmp_D + 8;
					break;
					  
				case ('M'):
					PORT_tmp_D = PORT_tmp_D + 16;
					break;
					
				case ('N'):
					PORT_tmp_D = PORT_tmp_D + 32;
					break;
					  
				case ('O'):
					PORT_tmp_D = PORT_tmp_D + 64;
					break;
					  
				case ('P'):
					PORT_tmp_D = PORT_tmp_D + 128;
					break;
#endif // USE_PORTD_SCHALT


#if USE_SERVO
				case ('Q'):
					if (var_array[8] > SERVO_MIN) {
						var_array[8] --;
						servo_go_pos();
					}
					break;

				case ('R'):
					if (var_array[8] < SERVO_MAX) {
						var_array[8] ++;
						servo_go_pos();
					}
					break;
#endif // USE_SERVO


#if USE_AUTOMATIK				
				// RoBue:
				// Automatik ein/aus
				case ('X'):
					schaltautomatik = 1;
					break;


				// RoBue:
				// Schaltbedingungen (Temperatur, analoger Wert, Zeit, ...)
				// Für jeden Port sind 2-4 Buchstaben reserviert, 
				// die je nach Bedarf in "webpage.h" als "value" übergeben werden können
				// C0 -> a,b
				// C1 -> c,d
				// C2 -> e,f
				// C3 -> g,h
				// C4 -> i,j
				// C5 -> k,l
				// C6 -> m,n,o,p
				// C7 -> u,v,w,x

				// Einschalttemperaturen (min. 5 bis max. 30 Grad) setzen
				case ('a'):
					if ( var_array[10] > 5 ) {
						var_array[10]--;
					}
					break;
				case ('b'):
					if ( var_array[10] < 30 ) {
						var_array[10]++;
					}
					break;
				case ('c'):
					if ( var_array[11] > 5 ) {
						var_array[11]--;
					}
					break;
				case ('d'):
					if ( var_array[11] < 30 ) {
						var_array[11]++;
					}
					break;


				// Einschalt-Differnz-Temperatur setzen
				case ('i'):
					if ( var_array[14] > 0 ) {
						var_array[14]--;
					}
					break;
				case ('j'):
					if ( var_array[14] < 20 ) {
						var_array[14]++;
					}
					break;


				// RoBue:
				// Einschaltwert analog (min. 0 bis max. 1050) einstellen
				case ('m'):
					if ( var_array[18] > 0 ) {
						var_array[18] -= 50;
					}
					break;
				case ('n'):
					if ( var_array[18] < 1050 ) {
						var_array[18] += 50;
					}
					break;

				// RoBue:
				// Schaltuhr einstellen
				case ('u'):
					if ( var_array[20] < 23 )
						var_array[20] += 1;
					else 
						var_array[20] = 0;
					break;
				case ('v'):
					if ( var_array[21] < 45 )
						var_array[21] += 15;
					else 
						var_array[21] = 0;
					break;
				case ('w'):
					if ( var_array[22] < 23 )
						var_array[22] += 1;
					else 
						var_array[22] = 0;
					break;
				case ('x'):
					if ( var_array[23] < 45 )
						var_array[23] += 15;
					else 
						var_array[23] = 0;
					break;

#endif // USE_AUTOMATIK


                   #if USE_WOL
                    case 'W':
                        wol_enable = 1;
						break;
                    #endif //USE_WOL
				  }
				http_entry[index].post_ptr = post_in;
				//Schaltanweisung wurde gefunden
			}
		
			//Submit schließt die suche ab!
			if (eth_buffer[a] != *http_entry[index].post_ready_ptr++)
			{
				http_entry[index].post_ready_ptr = post_ready;
			}
			if(*http_entry[index].post_ready_ptr == 0) 
			{
				http_entry[index].post = 0;

				// RoBue
				// Achtung PORTC -> PORTD
				// Schaltanweisung ausführen = PORTD setzen
				PORTC = PORT_tmp_C;
		                PORT_tmp_C = 0;
#if USE_PORTD_SCHALT
				PORTD = PORT_tmp_D;
		                PORT_tmp_D = 0;
#endif // USE_PORTD_SCHALT

#if USE_AUTOMATIK
				var_array[9] = schaltautomatik;

#endif // USE_AUTOMATIK

				break;
				//Submit gefunden
			}
		}
	}

	
	
	//Welche datei wird angefordert? Wird diese in der Flashspeichertabelle gefunden?
	unsigned char page_index = 0;
	
	if (!http_entry[index].new_page_pointer)
	{
		for(a = TCP_DATA_START_VAR+5;a<(TCP_DATA_END_VAR);a++)
		{
			if (eth_buffer[a] == '\r')
			{
				eth_buffer[a] = '\0';
				break;
			}
		}
	
		while(WEBPAGE_TABLE[page_index].filename)
		{
			if (strcasestr((char*)&eth_buffer[TCP_DATA_START_VAR],WEBPAGE_TABLE[page_index].filename)!=0) 
				{
					http_entry[index].http_header_type = 1;
					HTTP_DEBUG("\r\n\r\nDatei gefunden: ");
					HTTP_DEBUG("%s",(char*)WEBPAGE_TABLE[page_index].filename);
					HTTP_DEBUG("<----------------\r\n\r\n");	
					if (strcasestr(WEBPAGE_TABLE[page_index].filename,".jpg")!=0)
					{
						#if USE_CAM
						if (strcasestr(WEBPAGE_TABLE[page_index].filename,"camera")!=0)
						{	
							http_entry[index].cam = 1;
						}
						#endif //USE_CAM
						http_entry[index].http_header_type = 1;
					}
					if (strcasestr(WEBPAGE_TABLE[page_index].filename,".gif")!=0)
					{
						http_entry[index].http_header_type = 1;
					}	
					if (strcasestr(WEBPAGE_TABLE[page_index].filename,".htm")!=0)
					{
						http_entry[index].http_header_type = 0;	
					}	
					if (strcasestr(WEBPAGE_TABLE[page_index].filename,".json")!=0)
					{
						http_entry[index].http_header_type = 2;	
					}	
					http_entry[index].new_page_pointer = WEBPAGE_TABLE[page_index].page_pointer;
					break;
				}
			page_index++;
		}
	}

	//Wurde das Ende vom Header nicht erreicht
	//kommen noch weitere Stücke vom Header!
	if ((*http_entry[index].hdr_end_pointer != 0) || (http_entry[index].post == 1))
	{
		//Der Empfang wird Quitiert und es wird auf weiteres Headerstück gewartet
		tcp_entry[index].status =  ACK_FLAG;
		create_new_tcp_packet(0,index);
		//Warten auf weitere Headerpackete
		tcp_entry[index].app_status = 1;
		return;
	}	
	
	//Wurde das Passwort in den ganzen Headerpacketen gefunden?
	//Wenn nicht dann ausführen und Passwort anfordern!
	if((!http_entry[index].http_auth) && tcp_entry[index].status&PSH_FLAG)
	{	
		//HTTP_AUTH_Header senden!
		http_entry[index].new_page_pointer = Page0;
		memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR],http_header3,(sizeof(http_header3)-1));
		tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
		create_new_tcp_packet((sizeof(http_header3)-1),index);
		tcp_entry[index].app_status = 2;
		return;
	}
	
	//Standart INDEX.HTM Seite wenn keine andere gefunden wurde
	if (!http_entry[index].new_page_pointer)
	{
		//Besucher Counter
		var_array[MAX_VAR_ARRAY-1]++;
			// RoBue:
			// Counter zuruecksetzen
			if (var_array[MAX_VAR_ARRAY-1] == 10000)
				var_array[MAX_VAR_ARRAY-1] = 0;
		http_entry[index].new_page_pointer = Page1;
		http_entry[index].http_header_type = 0;
	}	
	
	tcp_entry[index].app_status = 2;
	//Seiten Header wird gesendet
	if(http_entry[index].http_header_type == 1)
	{
		memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR],http_header2,(sizeof(http_header2)-1));
        tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
        create_new_tcp_packet((sizeof(http_header2)-1),index);
        return;
	}
     
	if(http_entry[index].http_header_type == 0)
	{
		memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR],http_header1,(sizeof(http_header1)-1));
        tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
        create_new_tcp_packet((sizeof(http_header1)-1),index);
        return;
	}
     
	if(http_entry[index].http_header_type == 2)
	{
		memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR],http_header4,(sizeof(http_header4)-1));
        tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
        create_new_tcp_packet((sizeof(http_header4)-1),index);
        return;
	}
    return;
}

//----------------------------------------------------------------------------
//Daten Packete an Client schicken
void httpd_data_send (unsigned char index)
{	
	unsigned int a;
	unsigned char str_len;
	
	char var_conversion_buffer[CONVERSION_BUFFER_LEN];
	
	//Passwort wurde im Header nicht gefunden
	if(!http_entry[index].http_auth)
	{
		http_entry[index].new_page_pointer = Page0;
		#if USE_CAM
		http_entry[index].cam = 0;
		#endif //USE_CAM
	}
	
	#if USE_CAM //*****************************************************************
	unsigned long byte_counter = 0;
	
	if(http_entry[index].cam > 0)
	{
        //Neues Bild wird in den Speicher der Kamera geladen!
		if(http_entry[index].cam == 1){
			max_bytes = cam_picture_store(CAM_RESELUTION);
			http_entry[index].cam = 2;
		}
        
		for (a = 0;a < (MTU_SIZE-(TCP_DATA_START)-10);a++)
		{
			byte_counter = ((tcp_entry[index].app_status - 3)*(MTU_SIZE-(TCP_DATA_START)-10)) + a;
			
			eth_buffer[TCP_DATA_START + a] = cam_data_get(byte_counter);
			
			if(byte_counter > max_bytes)
			{
				tcp_entry[index].app_status = 0xFFFD;
				a++;
				break;
			}
		}
		//Erzeugte Packet kann nun gesendet werden!
		tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
		create_new_tcp_packet(a,index);
		return;	
	}
	#endif //USE_CAM***************************************************************

	//kein Packet empfangen Retransmission des alten Packetes
	if (tcp_entry[index].status == 0) 
	{
		http_entry[index].new_page_pointer = http_entry[index].old_page_pointer;
	}
	http_entry[index].old_page_pointer = http_entry[index].new_page_pointer;

	for (a = 0;a<(MTU_SIZE-(TCP_DATA_START)-150);a++)
	{
		unsigned char b;
		b = pgm_read_byte(http_entry[index].new_page_pointer++);
		eth_buffer[TCP_DATA_START + a] = b;
		
		//Müssen Variablen ins Packet eingesetzt werden? ===> %VA@00 bis %VA@19
		if (b == '%')
		{
			if (strncasecmp_P("VA@",http_entry[index].new_page_pointer,3)==0)
			{	
				b = (pgm_read_byte(http_entry[index].new_page_pointer+3)-48)*10;
				b +=(pgm_read_byte(http_entry[index].new_page_pointer+4)-48);	
				itoa (var_array[b],var_conversion_buffer,10);
				str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
				memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
				a = a + (str_len-1);
				http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+5;
			}
			
			
			// Volt measurements
			if (strncasecmp_P("VV@",http_entry[index].new_page_pointer,3)==0)
			{	
				b = (pgm_read_byte(http_entry[index].new_page_pointer+3)-48)*10;
				b +=(pgm_read_byte(http_entry[index].new_page_pointer+4)-48);	
				if (b >=4 && b <= 6) { // Analog Value
					uint32_t vconv = var_array[b];
					vconv *= 1425;
					vconv /= 100;
					itoa (vconv,var_conversion_buffer, 10);
				} else {
					itoa (var_array[b],var_conversion_buffer,10);
				}
				str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
				memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
				a = a + (str_len-1);
				http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+5;
			}
			

			// Zeitstring
			if (strncasecmp_P("TI@",http_entry[index].new_page_pointer,3)==0)
			{
				//unsigned char hh = (time/3600)%24;
				itoa (hh,var_conversion_buffer,10);
 	      			strcpy(time_string_Buffer,var_conversion_buffer);
        			strcat (time_string_Buffer,":");
				if ( mm < 10 ) {
					strcat (time_string_Buffer,"0");
						}
        			itoa (mm,var_conversion_buffer,10);
        			strcat(time_string_Buffer,var_conversion_buffer);
               			str_len = strnlen(time_string_Buffer,10);
        			memmove(&eth_buffer[TCP_DATA_START+a],time_string_Buffer,str_len);
	       			a = a + (str_len-1);
       			http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+3;
			}



			// Schaltzeit
			if (strncasecmp_P("T0@",http_entry[index].new_page_pointer,3)==0)
			{	
			itoa (var_array[20],var_conversion_buffer,10);
 	      			strcpy(time_string_Buffer,var_conversion_buffer);
        			strcat (time_string_Buffer,":");
				if ( var_array[21] < 10 ) {
					strcat (time_string_Buffer,"0");
						}
        			itoa (var_array[21],var_conversion_buffer,10);
        			strcat(time_string_Buffer,var_conversion_buffer);
               			str_len = strnlen(time_string_Buffer,10);
        			memmove(&eth_buffer[TCP_DATA_START+a],time_string_Buffer,str_len);
	       			a = a + (str_len-1);
       			http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+3;	
			}
			if (strncasecmp_P("T1@",http_entry[index].new_page_pointer,3)==0)
			{	
			itoa (var_array[22],var_conversion_buffer,10);
 	      			strcpy(time_string_Buffer,var_conversion_buffer);
        			strcat (time_string_Buffer,":");
				if ( var_array[23] < 10 ) {
					strcat (time_string_Buffer,"0");
						}
        			itoa (var_array[23],var_conversion_buffer,10);
        			strcat(time_string_Buffer,var_conversion_buffer);
               			str_len = strnlen(time_string_Buffer,10);
        			memmove(&eth_buffer[TCP_DATA_START+a],time_string_Buffer,str_len);
	       			a = a + (str_len-1);
       			http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+3;		
			}				



#if USE_OW
	
			/*
			*	1-Wire Temperatursensoren
			*	-------------------------
			*	OW@nn	nn = 00 bis MAXSENSORS-1 gibt Werte in 1/10 °C aus
			*	OW@mm	mm = 20 bis MAXSENSORS-1+20 gibt Werte in °C mit einer Nachkommastelle aus
			*	d.h. OW@nn für Balkenbreite verwenden und OW@mm für Celsius-Anzeige
			*/
			if (strncasecmp_P("OW@",http_entry[index].new_page_pointer,3)==0)	
			{
				b = (pgm_read_byte(http_entry[index].new_page_pointer+3)-48)*10;
				b +=(pgm_read_byte(http_entry[index].new_page_pointer+4)-48);
					
					// RoBue:
					// Wert auslesen
					int16_t ow_temp = ow_array[b];
					str_len = 0;
	
					// evtl. Vorzeichen einfuegen:	
					if ( ow_temp < 0 ) {
						ow_temp *= (-1);
						var_conversion_buffer[0] = '-';
						memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,1);
						a ++;
					}
					
					// RoBue:
					// Wert vor dem Komma einfügen
					itoa (ow_temp/10,var_conversion_buffer,10);
					str_len += strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
					
					// RoBue:
					// Komma einfügen
					var_conversion_buffer[str_len] = ',';
					str_len++;
					memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
					a += str_len;

					// RoBue:
					// Wert nach dem Komma einfügen					
					itoa (ow_temp%10,var_conversion_buffer,10);
					str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
					memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
					a += str_len-1;

					http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+5;

			}

#endif	// USE_OW


			//Einsetzen des Port Status %PORTxy durch "checked" wenn Portx.Piny = 1
			//x: A..G  y: 0..7 
			if (strncasecmp_P("PORT",http_entry[index].new_page_pointer,4)==0)
			{
				unsigned char pin  = (pgm_read_byte(http_entry[index].new_page_pointer+5)-48);	
				b = 0;
				switch(pgm_read_byte(http_entry[index].new_page_pointer+4))
				{
					case 'A':
						b = (PORTA & (1<<pin));
						break;
					case 'B':
						b = (PORTB & (1<<pin));
						break;
					case 'C':
						b = (PORTC & (1<<pin));
						break;
					case 'D':
						b = (PORTD & (1<<pin));
						break;
					// RoBue:
					// Checkbox Automatik
					case 'X':
						b = var_array[9];
						break; 
				}
				
				if(b)
				{
					strcpy_P(var_conversion_buffer, PSTR("checked"));
				}
				else
				{
					strcpy_P(var_conversion_buffer, PSTR("\0"));
				}
				str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
				memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
				a += str_len-1;
				http_entry[index].new_page_pointer = http_entry[index].new_page_pointer+6;
			}

			//Einsetzen des Port Status %PBOOLxy durch "true" oder "false"
			//PIN for PORTA, PORT for B-D
			//x: A..G  y: 0..7 
			if (strncasecmp_P("PBOOL",http_entry[index].new_page_pointer,5)==0)
			{
				unsigned char pin  = (pgm_read_byte(http_entry[index].new_page_pointer+6)-'0');	
				b = 0;
				switch(pgm_read_byte(http_entry[index].new_page_pointer+5))
				{
					case 'A':
						b = (PINA & (1<<pin));
						break;
					case 'B':
						b = (PORTB & (1<<pin));
						break;
					case 'C':
						b = (PORTC & (1<<pin));
						break;
					case 'D':
						b = (PORTD & (1<<pin));
						break;
					// RoBue:
					// Checkbox Automatik
					case 'X':
						b = var_array[9];
						break; 
				}
				
				if(b)
				{
					strcpy_P(var_conversion_buffer, PSTR("true"));
				}
				else
				{
					strcpy_P(var_conversion_buffer, PSTR("false"));
				}
				str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
				memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
				a += str_len-1;
				http_entry[index].new_page_pointer = http_entry[index].new_page_pointer+7;
			}
						
			// RoBue:
			// Einsetzen des Pin Status %PI@xy bis %PI@xy 
			// durch grüne oder graue Hintergrundarbe der Tabellenzelle

			//x = 0 : PINA / x = 1 : PINB / x = 2 : PINC / x = 3 : PIND

			if (strncasecmp_P("PIN",http_entry[index].new_page_pointer,3)==0)
			{
				unsigned char pin  = (pgm_read_byte(http_entry[index].new_page_pointer+4)-48);	
				b = 0;
				switch(pgm_read_byte(http_entry[index].new_page_pointer+3))
				{
					case 'A':
						b = (PINA & (1<<pin));
						break;
					case 'B':
						b = (PINB & (1<<pin));
						break;
					case 'C':
						b = (PINC & (1<<pin));
						break;
					case 'D':
						b = (PIND & (1<<pin));
						break;    
				}
				
				if(b)
				{
					strcpy_P(var_conversion_buffer, PSTR("153, 0, 0")); // rot
				}
				else
				{
					strcpy_P(var_conversion_buffer, PSTR("0, 153, 0")); // grün
				}
				str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
				memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
				a += str_len-1;
				http_entry[index].new_page_pointer = http_entry[index].new_page_pointer+5;
			}
			//wurde das Ende des Packetes erreicht?
			//Verbindung TCP Port kann beim nächsten ACK geschlossen werden
			//Schleife wird abgebrochen keine Daten mehr!!
			if (strncasecmp_P("END",http_entry[index].new_page_pointer,3)==0)
			{	
				tcp_entry[index].app_status = 0xFFFD;
				break;
			}
		}
	}
	//Erzeugte Packet kann nun gesendet werden!
	tcp_entry[index].status =  ACK_FLAG | PSH_FLAG;
	create_new_tcp_packet(a,index);
	return;
}
