/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        24.10.2007
 Description:    RS232 Camera Routinen
 
 Udo Matthe 15.1.08 max_bytes Berechnung korrigiert
                    Zaehl/Warteschleifen ersetzt durch Bufferfuellstandsabfragen.
                    22.1184MHz eingebaut (funktioniert nicht fehlerfrei) 
					
----------------------------------------------------------------------------*/
#include "cam.h"


#if USE_CAM
volatile unsigned long max_bytes;
volatile long cmd_buffercounter = 0;
volatile long dat_buffercounter = 0;
volatile long cam_dat_start = CAM_HEADER;
volatile long cam_dat_stop = CAM_HEADER + DAT_BUFFER_SIZE;
volatile unsigned char cam_cmd_buffer[CMD_BUFFER_SIZE];
volatile unsigned char cam_dat_buffer[DAT_BUFFER_SIZE];

//----------------------------------------------------------------------------
//Routine für die Serielle Ausgabe eines Zeichens (Schnittstelle)
void cam_uart_write_char(char c)
{
	//Warten solange bis Zeichen gesendet wurde
	while(!(USR & (1<<UDRE)));
	//Ausgabe des Zeichens
	UDR = c;
	return;
}

//----------------------------------------------------------------------------
//Sendet Commando an die Kamera
void cam_command_send (unsigned char cmd1,unsigned char cmd2,unsigned char cmd3,
						unsigned char cmd4,unsigned char cmd5)
{
	unsigned char error_count = 0;
	while(1){
		cmd_buffercounter = 0;
		dat_buffercounter = 0;
		cam_uart_write_char(0xFF);
		cam_uart_write_char(0xFF);
		cam_uart_write_char(0xFF);
		cam_uart_write_char(cmd1);
		cam_uart_write_char(cmd2);
		cam_uart_write_char(cmd3);
		cam_uart_write_char(cmd4);
		cam_uart_write_char(cmd5);
		for(unsigned long a = 0;a<100000;a++)
		{
			if(cmd_buffercounter>7 && cam_cmd_buffer[3]== 0x0E)
			{
				return;
			}
		}
		if(error_count++ > 3) return;
	}
}

//----------------------------------------------------------------------------
//Ausgabe eines Bytes vom Bild
char cam_data_get (unsigned long byte)
{
	byte = byte + 1;
	unsigned long timeout = 300000;
	unsigned long byte_tmp1 = byte / DAT_BUFFER_SIZE;
	
	if(byte < (cam_dat_start-CAM_HEADER) || byte >= (cam_dat_stop-CAM_HEADER))
	{
		cam_dat_start = CAM_HEADER + (byte_tmp1 * DAT_BUFFER_SIZE);
		cam_dat_stop = CAM_HEADER + DAT_BUFFER_SIZE + (byte_tmp1 * DAT_BUFFER_SIZE);
		//request Data
		PORTD |= (1<<6);
		cam_command_send (0x04,0x01,0x00,0x00,0x00);
        while(cmd_buffercounter<cam_dat_stop && cmd_buffercounter<max_bytes && (timeout--) != 0);
	}

	byte = byte % DAT_BUFFER_SIZE;
        return (cam_dat_buffer[byte]);
}

//----------------------------------------------------------------------------
//Speichert ein neues Bild im Kameraspeicher
unsigned long cam_picture_store (char mode)
{
	unsigned long bytes = 0;
	unsigned long timeout = 300000;
	cam_command_send (0x08,0x01,0x00,0x00,0x00);
	cam_command_send (0x10,0x01,0x00,0x00,0x00);
	cam_command_send (0x12,0x00,0x00,0x00,0x00);
	cam_command_send (0x18,0x01,0x00,0x00,0x00);
	cam_command_send (0x17,0x00,0x00,0x00,0x00);
	cam_command_send (0x21,0x03,0x00,0x00,0x00);
	//Auflösung 160x120
	if (mode == 0)
	{
		cam_command_send (0x01,0x01,0x07,0x19,0x13);
	}
	//Auflösung 320x240
	if (mode == 1)
	{
		cam_command_send (0x01,0x01,0x07,0x09,0x05);
	}
	//Auflösung 640x480
	if (mode == 2)
	{
		cam_command_send (0x01,0x01,0x07,0x09,0x07);
	}
	
	cam_command_send (0x05,0x00,0x00,0x00,0x00);
	
	cam_dat_start = CAM_HEADER;
	cam_dat_stop = CAM_HEADER + DAT_BUFFER_SIZE;
	
	cam_command_send (0x04,0x01,0x00,0x00,0x00);
	
	while((cmd_buffercounter) < CMD_BUFFER_SIZE && (timeout--) != 0);
	
	bytes = (unsigned long)cam_cmd_buffer[13];
	bytes+= ((unsigned long)cam_cmd_buffer[14]<<8);
	bytes+= ((unsigned long)cam_cmd_buffer[15]<<16);

	return(bytes+1);
}

//----------------------------------------------------------------------------
//Sendet Commando an die Kamera
void cam_init (void)
{
	//Enable TXEN im Register UCR TX-Data Enable
	UCR =(1 << TXEN | 1 << RXEN | 1<< RXCIE);
	//Teiler wird gesetzt 
#if (F_CPU == 14745600)
	UBRR=0;
#elif (F_CPU == 22118400)
	USR =(1<<U2X0);
	UBRR=2;
#else
      #error "Kamera funktioniert nur mit F_CPU 14.745600MHz oder 22.118400MHz"
#endif
	
	unsigned char sync = 1;
	do {
		cam_command_send (0x0D,0x00,0x00,0x00,0x00);
		for(unsigned long a = 0;a<100000;a++)
		{
			if (cmd_buffercounter >= 16){
				for(unsigned long a = 0;a<100000;a++){asm("nop");};
				cam_uart_write_char(0xFF);
				cam_uart_write_char(0xFF);
				cam_uart_write_char(0xFF);
				cam_uart_write_char(0x0E);
				cam_uart_write_char(0x0D);
				cam_uart_write_char(0x00);
				cam_uart_write_char(0x00);
				cam_uart_write_char(0x00);
				sync = 0;
				break;
				};
		};
    }while(sync);
}

//----------------------------------------------------------------------------
//Empfang eines Zeichens
ISR (USART_RX_CAM)
{
	char tmp = UDR;
	if(cmd_buffercounter < CMD_BUFFER_SIZE) cam_cmd_buffer[cmd_buffercounter] = tmp;
	if(cmd_buffercounter >= cam_dat_start && cmd_buffercounter < cam_dat_stop) cam_dat_buffer[dat_buffercounter++] = UDR;
	cmd_buffercounter++;
}
#endif //USE_CAM


