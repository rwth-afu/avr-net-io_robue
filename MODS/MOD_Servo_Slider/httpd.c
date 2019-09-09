/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        some changes by Michael Kleiber (slider function on webpage)
 known Problems: none
 Version:        06.12.2008
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

struct http_table http_entry[MAX_TCP_ENTRY];

//Hier wird das codierte Passwort aus config.h gespeichert.
unsigned char http_auth_passwort[20];
unsigned char PORT_tmp         = 0;

//----------------------------------------------------------------------------
//Variablenarry zum einfügen in Webseite %VA@00 bis %VA@09
unsigned int var_array[MAX_VAR_ARRAY] = {0,0,30,0,0,0,0,0,0,0};
//----------------------------------------------------------------------------

PROGMEM char http_header[]={ "HTTP/1.0 200 Document follows\r\n"
                              "Server: MINI_AVR\r\n"
                              "Content-Type: "};

PROGMEM char http_header1[]={ "text/html\r\n\r\n"};

PROGMEM char http_header2[]={ "image/jpg\r\n\r\n"};

PROGMEM char http_header3[]={ "HTTP/1.0 401 Unauthorized\r\n"
                              "Server: MINI_AVR\r\n"
                              "WWW-Authenticate: Basic realm=\"NeedPassword\""
                              "\r\nContent-Type: text/html\r\n\r\n"};

//----------------------------------------------------------------------------

char dstr[24]={"No Time...             "};

//----------------------------------------------------------------------------
void httpd_init (void)
{
  //HTTP_AUTH_STRING 
  decode_base64((unsigned char*)HTTP_AUTH_STRING,http_auth_passwort);
  
  //Serverport und Anwendung eintragen
  add_tcp_app (HTTPD_PORT, (void(*)(unsigned char))httpd);
}
//----------------------------------------------------------------------------
//http-server
void httpd (unsigned char index)
{
  //Verbindung wurde abgebaut!
  if (tcp_entry[index].status & FIN_FLAG)
  {
    http_entry[index].http_auth=0;
    return;
  }

  //Allererster Aufruf des Ports für diese Anwendung
  //HTTPD_Anwendungsstack löschen
  if(tcp_entry[index].app_status == 1)
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
  if ( (tcp_entry[index].app_status > 2) && (tcp_entry[index].app_status < 0xFFFE) )
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
  http_entry[index].http_header_type = 0;
  http_entry[index].first_switch     = 0;
  http_entry[index].http_auth        = USE_HTTP_AUTH;
  http_entry[index].new_page_pointer = 0;
  http_entry[index].old_page_pointer = 0;
  http_entry[index].post             = 0;
  http_entry[index].hdr_end       = 0;
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
  unsigned int  a;
  unsigned char page_index = 0;
           char *p;
  
  eth_buffer[TCP_DATA_END_VAR+1] = 0; //trailing 0 for string operations
  
  if( strcasestr_P((char*)&eth_buffer[TCP_DATA_START_VAR],PSTR("POST")) )
  {
    http_entry[index].post = 1;
  }

  if (strcasestr((char*)&eth_buffer[TCP_DATA_START_VAR],(char*)http_auth_passwort)) 
  {
    http_entry[index].http_auth = 1;
  }

  if( strcasestr_P((char*)&eth_buffer[TCP_DATA_START_VAR],PSTR("\r\n\r\n")) != 0 )
  {
    http_entry[index].hdr_end = 1;
  }

  p = (char*)&eth_buffer[TCP_DATA_START_VAR];
  do
  {
    p = strcasestr_P( p, PSTR("OUT=") );
    if( p )
    {
      p += 4;
      if ( (*p >= 'A') && (*p <= 'H') )
      {
        PORT_tmp |= 1 << (*p - 'A');
      }
      #if USE_WOL
      switch (*p)
      {
        case 'W':
          wol_enable = 1;
        break;
        case 'X':
          wol_enable = 2;
        break;
      }
      #endif //USE_WOL
    }
  }
  while (p);

  p = strcasestr_P( (char*)&eth_buffer[TCP_DATA_START_VAR], PSTR("SLI=") );
  if( p )
  {
    p += 4;
    var_array[8] = strtol(p,NULL,0);
  }


  if( strcasestr_P((char*)&eth_buffer[TCP_DATA_START_VAR],PSTR("SUB")) )
  {
      http_entry[index].post = 0;
      PORTC                  = PORT_tmp;
      PORT_tmp               = 0;
  }
  
  
  //Welche datei wird angefordert? Wird diese in der Flashspeichertabelle gefunden?
  
  if ( http_entry[index].new_page_pointer == 0 )
  {
    for(a=TCP_DATA_START_VAR+5; a<TCP_DATA_END_VAR; a++)
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
        http_entry[index].new_page_pointer = WEBPAGE_TABLE[page_index].page_pointer;
        break;
      }
      page_index++;
    }
  }

  //Wurde das Ende vom Header nicht erreicht kommen noch weitere Stücke vom Header!
  if ((http_entry[index].hdr_end == 0) || (http_entry[index].post == 1))
  {
    //Der Empfang wird Quitiert und es wird auf weiteres Headerstück gewartet
    tcp_entry[index].status =  ACK_FLAG;
    create_new_tcp_packet(0,index);
    //Warten auf weitere Headerpackete
    tcp_entry[index].app_status = 1;
    return;
  }  
  
  //Wurde das Passwort in den ganzen Headerpaketen gefunden?
  //Wenn nicht dann ausführen und Passwort anfordern!
  if( (http_entry[index].http_auth==0) && ((tcp_entry[index].status&PSH_FLAG)!=0) )
  {  
    //HTTP_AUTH_Header senden!
    http_entry[index].new_page_pointer = Page0;
    memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR],http_header3,(sizeof(http_header3)-1));
    tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
    create_new_tcp_packet((sizeof(http_header3)-1),index);
    tcp_entry[index].app_status = 2;
    return;
  }
  
  //Standart INDEX.HTM Seite wenn keine andere gefunden wurde
  if ( http_entry[index].new_page_pointer == 0 )
  {
    //Besucher Counter
    var_array[9]++;
    http_entry[index].new_page_pointer = Page1;
    http_entry[index].http_header_type = 0;
  }  
  
  tcp_entry[index].app_status = 2;

  //Seiten Header wird gesendet
  a = sizeof(http_header)-1;
  memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR],http_header,(sizeof(http_header)-1));
  if(http_entry[index].http_header_type == 0)
  {
    memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR+a],http_header1,(sizeof(http_header1)-1));
    a += sizeof(http_header1)-1;
  }
  else
  {
    memcpy_P((char*)&eth_buffer[TCP_DATA_START_VAR+a],http_header2,(sizeof(http_header2)-1));
    a += sizeof(http_header2)-1;
  }
  tcp_entry[index].status = ACK_FLAG | PSH_FLAG;
  create_new_tcp_packet(a,index);
  return;
}

//----------------------------------------------------------------------------
//Daten Pakete an Client schicken
void httpd_data_send (unsigned char index)
{  
  unsigned int  a;
  unsigned char b;
  unsigned char nr;
  unsigned char str_len;
  
  char var_conversion_buffer[CONVERSION_BUFFER_LEN];
  
  //Passwort wurde im Header nicht gefunden
  if( http_entry[index].http_auth == 0 )
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
  
  //kein Paket empfangen Retransmission des alten Paketes
  if (tcp_entry[index].status == 0) 
  {
    http_entry[index].new_page_pointer = http_entry[index].old_page_pointer;
  }
  http_entry[index].old_page_pointer = http_entry[index].new_page_pointer;

  for ( a = 0; a<(MTU_SIZE-(TCP_DATA_START)-150); a++ )
  {
    b = pgm_read_byte(http_entry[index].new_page_pointer++);
    eth_buffer[TCP_DATA_START + a] = b;
    
    //Insert values from var_array[] ===> %VA@00 bis %VA@09
    if (b == '%')
    {
      if (strncasecmp_P("VA@",http_entry[index].new_page_pointer,3)==0)
      {  
        b = (pgm_read_byte(http_entry[index].new_page_pointer+3)-'0')*10;
        b +=(pgm_read_byte(http_entry[index].new_page_pointer+4)-'0');  
        itoa (var_array[b],var_conversion_buffer,10);
        str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
        memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
        a = a + (str_len-1);
        http_entry[index].new_page_pointer=http_entry[index].new_page_pointer+5;
      }
      
      //Insertion of strings %STRx
      //x: number
      if (strncasecmp_P("STR",http_entry[index].new_page_pointer,3)==0)
      {
        nr = (pgm_read_byte(http_entry[index].new_page_pointer+3)-'0');  
        if ( nr == 0 ) //datestring for first start of the system
        {
          memmove(&eth_buffer[TCP_DATA_START+a],dstr,23);
          a += 23-1;
          http_entry[index].new_page_pointer = http_entry[index].new_page_pointer+4;
        }
        
      }
      
      //Einsetzen des Port Status %PORTxy durch "checked" wenn Portx.Piny = 1
      //x: A..G  y: 0..7 
      if (strncasecmp_P("PORT",http_entry[index].new_page_pointer,4)==0)
      {
        nr = (pgm_read_byte(http_entry[index].new_page_pointer+5)-'0');  
        b = 0;
        switch(pgm_read_byte(http_entry[index].new_page_pointer+4))
        {
          case 'A':
            b = (PORTA & (1<<nr));
            break;
          case 'B':
            b = (PORTB & (1<<nr));
            break;
          case 'C':
            b = (PORTC & (1<<nr));
            break;
          case 'D':
            b = (PORTD & (1<<nr));
            break; 
        }
        
        if( b != 0 )
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
      
      //Einsetzen des Pin Status %PI@xy bis %PINxy durch "ledon" oder "ledoff"
      //x = 0 : PINA / x = 1 : PINB / x = 2 : PINC / x = 3 : PIND
      if (strncasecmp_P("PIN",http_entry[index].new_page_pointer,3)==0)
      {
        nr  = (pgm_read_byte(http_entry[index].new_page_pointer+4)-'0');  
        b = 0;
        switch(pgm_read_byte(http_entry[index].new_page_pointer+3))
        {
          case 'A':
            b = (PINA & (1<<nr));
            break;
          case 'B':
            b = (PINB & (1<<nr));
            break;
          case 'C':
            b = (PINC & (1<<nr));
            break;
          case 'D':
            b = (PIND & (1<<nr));
            break;    
        }
        
        if( b != 0 )
        {
          strcpy_P(var_conversion_buffer, PSTR("ledon.gif"));
        }
        else
        {
          strcpy_P(var_conversion_buffer, PSTR("ledoff.gif"));
        }
        str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
        memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
        a += str_len-1;
        http_entry[index].new_page_pointer = http_entry[index].new_page_pointer+5;
      }
      
      //Einsetzen des WOL Status
      if (strncasecmp_P("WOL",http_entry[index].new_page_pointer,3)==0)
      {
        nr = (pgm_read_byte(http_entry[index].new_page_pointer+3)-'0');
        b = 0;
        
        if ( nr<8 )
        {
          b = (1 << nr);
        }
        
        if( (ping.result&b) != 0 )
        {
          strcpy_P(var_conversion_buffer, PSTR("ledon.gif"));
        }
        else
        {
          strcpy_P(var_conversion_buffer, PSTR("ledoff.gif"));
        }
        str_len = strnlen(var_conversion_buffer,CONVERSION_BUFFER_LEN);
        memmove(&eth_buffer[TCP_DATA_START+a],var_conversion_buffer,str_len);
        a += str_len-1;
        http_entry[index].new_page_pointer = http_entry[index].new_page_pointer+4;
      }

      //wurde das Ende des Paketes erreicht?
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


