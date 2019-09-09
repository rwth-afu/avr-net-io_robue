/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        23.12.2007
 Description:    RS232 Camera Routinen
                 Kamera arbeitet nur mit einem 14,7456Mhz Quarz!
----------------------------------------------------------------------------*/
#include <config.h>

#if USE_CAM
#ifndef _CAM_H
	#define _CAM_H
	
	//----------------------------------------------------------------------------
	#include <avr/interrupt.h>
	#include <avr/io.h>
	
	//----------------------------------------------------------------------------	
	
	
	#if defined (__AVR_ATmega32__)
		#define DAT_BUFFER_SIZE	200
	#endif

	#if defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__)
		#define DAT_BUFFER_SIZE	1200
	#endif
	
	#define CMD_BUFFER_SIZE	17
	#define CAM_HEADER 16
	
	volatile unsigned long max_bytes;
	volatile long cam_dat_start;
	volatile long cam_dat_stop;
	volatile long cmd_buffercounter;
	volatile long dat_buffercounter;
	volatile unsigned char cam_cmd_buffer[CMD_BUFFER_SIZE];
	volatile unsigned char cam_dat_buffer[DAT_BUFFER_SIZE];
	
	//Anpassen der seriellen Schnittstellen Register wenn ein ATMega128 benutzt wird
	#if defined (__AVR_ATmega128__)
		#define USR UCSR0A
		#define UCR UCSR0B
		#define UDR UDR0
		#define UBRR UBRR0L
		#define USART_RX_CAM USART0_RX_vect 
	#endif
	
	#if defined (__AVR_ATmega644__) || defined (__AVR_ATmega644P__)
		#define USR UCSR0A
		#define UCR UCSR0B
		#define UBRR UBRR0L
		#define EICR EICRB
		#define TXEN TXEN0
		#define RXEN RXEN0
		#define RXCIE RXCIE0
		#define UDR UDR0
		#define UDRE UDRE0
		#define USART_RX_CAM USART0_RX_vect   
	#endif
	
	#if defined (__AVR_ATmega32__)
		#define USR UCSRA
		#define UCR UCSRB
		#define UBRR UBRRL
		#define EICR EICRB
		#define USART_RX_CAM USART_RXC_vect  
	#endif
	
	#if defined (__AVR_ATmega8__)
		#define USR UCSRA
		#define UCR UCSRB
		#define UBRR UBRRL
	#endif
	
	#if defined (__AVR_ATmega88__)
		#define USR UCSR0A
		#define UCR UCSR0B
		#define UBRR UBRR0L
		#define TXEN TXEN0
		#define UDR UDR0
		#define UDRE UDRE0
	#endif
	//----------------------------------------------------------------------------
	
	void cam_init (void);
	void cam_uart_init(void); 
	void cam_uart_write_char(char);
	void cam_command_send (unsigned char,unsigned char,unsigned char,unsigned char,unsigned char);
	unsigned long cam_picture_store (char mode);
	char cam_data_get (unsigned long byte);
	
	//----------------------------------------------------------------------------

#endif //_CAM_H
#endif //USE_CAM
