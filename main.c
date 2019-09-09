/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        24.10.2007
 Description:    Webserver uvm.

 Modifiziert für AVR-NET-IO:	RoBue
 Version:			1.5 TEST-3
 Datum:				14.02.2009

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
----------------------------------------------------------------------------*/

#include <avr/io.h>
#include "config.h"
#include "usart.h"
#include "networkcard/enc28j60.h"
#include "networkcard/rtl8019.h"
#include "stack.h"
#include "timer.h"
#include "wol.h"
#include "httpd.h"
#include "cmd.h"
#include "telnetd.h"
#include "ntp.h"
#include "base64.h"
#include "http_get.h"
#include "lcd.h"
#include "udp_lcd.h"
#include "analog.h"
#include "camera/cam.h"
#include "camera/servo.h"
#include "sendmail.h"

volatile unsigned int variable[MAX_VAR];

// RoBue:
// Variablen-Array 
// zum Abspeichern verschiedener Werte
// und zum Einfügen in die Webseite %VA@00 bis %VA@19
// VA4-7	-> Analogwert von PORTA4-7
// VA8		-> Position Servo fuer Webcam
// VA9		-> Manueller Betrieb ein/aus
// VA10-17	-> Schaltwerte Temperaturen
// VA18,19	-> Schaltwerte analog
// VA20-23	-> Schaltzeiten ein/aus hh,mm,hh,mm
// VA24-28	-> Reserve
// VA-Ende	-> Counter
unsigned int var_array[MAX_VAR_ARRAY] = {10,50,30,0,0,0,0,0,25,0,15,15,15,15,15,15,15,15,750,750,12,0,12,0,0,0,0,0,0,0};

// RoBue:
// Die Uhrzeit wird zentral in main.c berechnet (s.u.)
// Die anderen Programmteile lesen nur noch hh,mm,ss aus.
// -> httpd.h/c, cmd.h/c'

unsigned char hh;
unsigned char mm;
unsigned char ss;

// RoBue:	
// Das periodische Auslesen der 1-Wire-Sensoren erfolgt in main.c (s.u.)
// Die anderen Programmteile lesen nur noch "ow_array" aus.
// -> httpd.h/c, cmd.h/c 

#if USE_OW
	#include "1-wire/ds18x20.h"
	// Variable
	// Anmerkung RoBue:
	// Anzahl der Sensoren (MAXSENSORS) und ID -> config.h
	// Speicherplatz für 1-wire Sensorwerte (mit Minuswerten)
	// und zusätzlich für die Tages-Min/Max-Werte
	// (min -> ow_array[MAXSENSORS] bis ow_array[MAXSENSORS*2-1])
	// (max -> ow_array[MAXSENSORS*2] bis ow_array[MAXSENSORS*3-1])
	int16_t ow_array[MAXSENSORS*3];
	// Speicherplatz für die ID der Sensoren
	// Position 0-4 eig. ID, Pos. 5+6 ist die Temperatur, Pos. 7 CRC-Byte
	PROGMEM	uint8_t	const	DS18B20IDs[MAXSENSORS+1][OW_ROMCODE_SIZE] = {
							OW_ID_T01,	// 1. DS18B20
							OW_ID_T02,
							OW_ID_T03,
							OW_ID_T04,
							OW_ID_T05,
							OW_ID_T06,
							OW_ID_T07,
							OW_ID_T08,
							OW_ID_Last };	// Endmarker

	uint8_t auslesen	= 0;	// 0 -> Sensoren nicht auslesen
	uint8_t messen		= 1;	// 1 -> Sensoren zum Messen auffordern
	uint8_t minmax		= 1;	// 1 -> Zurücksetzen der Min/Max-Werte 1x/Tag
	
#endif

// cni:
// SHT-Sensor
#if USE_SHT
	#include "sht/libsht.h"
#endif

// RoBue:
// Schalten ja/nein
uint8_t schalten;

//----------------------------------------------------------------------------

// Hier startet das Hauptprogramm
// ******************************

int main(void)
{  
	//Konfiguration der Ausgänge bzw. Eingänge
	//definition erfolgt in der config.h
	DDRA = OUTA;
	#if USE_SER_LCD
		DDRC = OUTC;
	#else
		DDRC = OUTC;
		#if PORTD_SCHALT
			DDRD = OUTD;
		#endif
	#endif

	// RoBue:
	// Pullups einschalten
	PORTA |= (1 << PORTA0) | (1 << PORTA1) | (1 << PORTA2) | (1 << PORTA3);
	#if USE_SHT
		PORTA |= (1 << PORTA4);
	#else
		PORTA |= (1 << PORTA4) | (1 << PORTA5) | (1 << PORTA6);
	#endif
	
    unsigned long a;

    usart_init(BAUDRATE); // setup the UART

	#if USE_SERVO
		servo_init ();
		servo_go_pos();
	#endif //USE_SERVO
	
	#if USE_ADC
		ADC_Init();
	#endif
	
	usart_write("\n\rSystem Ready\n\r");
    usart_write("Compiliert am "__DATE__" um "__TIME__"\r\n");
    usart_write("Compiliert mit GCC Version "__VERSION__"\r\n");
	for(a=0;a<1000000;a++){asm("nop");};

	//Applikationen starten
	stack_init();
	httpd_init();
	telnetd_init();
	
	//Spielerrei mit einem LCD
	#if USE_SER_LCD
		udp_lcd_init();
		lcd_init();
		// RoBue:
		// LCD-Ausgaben:
		lcd_clear();
		lcd_print(0,0,"*AVR-NET-IO "Version"*");
	#endif

	//Ethernetcard Interrupt enable
	ETH_INT_ENABLE;
	
	#if USE_SER_LCD
		// RoBue:
		// IP auf LCD
		lcd_print(2,0,"%1i.%1i.%1i.%1i",myip[0],myip[1],myip[2],myip[3]);
	#endif
	
	//Globale Interrupts einschalten
	sei(); 
	
	#if USE_CAM
		#if USE_SER_LCD
			lcd_print(3,0,"CAMERA INIT");
		#endif //USE_SER_LCD
		for(a=0;a<2000000;a++){asm("nop");};
		cam_init();
		max_bytes = cam_picture_store(CAM_RESELUTION);
		#if USE_SER_LCD
			back_light = 0;
			lcd_print(3,0,"CAMERA READY");
		#endif //USE_SER_LCD
	#endif // -> USE_CAM
	
	#if USE_NTP
	        ntp_init();
	        ntp_request();
	#endif //USE_NTP
	
	#if USE_WOL
		wol_init();
	#endif //USE_WOL
    
	#if USE_MAIL
	        mail_client_init();
	#endif //USE_MAIL  


	// Startwerte für ow_array setzen
#if USE_OW
		uint8_t i = 0;
		for (i=0;i<MAXSENSORS;i++){
			ow_array[i]=OW_START;
		}
		for (i=MAXSENSORS;i<MAXSENSORS*3;i++){
			ow_array[i]=OW_MINMAX;
		}
		DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL );
		for(a=0;a<1000000;a++){asm("nop");};
		auslesen = 0;
		minmax = 1;
#endif


//Hauptschlfeife
// *************
		
	while(1)
	{

	#if USE_ADC
		ANALOG_ON;
	#endif

	eth_get_data();
		
        //Terminalcommandos auswerten
	if (usart_status.usart_ready){
	usart_write("\r\n");
		if(extract_cmd(&usart_rx_buffer[0]))
		{
			usart_write("Ready\r\n\r\n");
		}
		else
		{
			usart_write("ERROR\r\n\r\n");
		}
		usart_status.usart_ready =0;
	
	}
	
	// RoBue:
	// Uhrzeit bestimmen
	hh = (time/3600)%24;
	mm = (time/60)%60;
	ss = time%60;


	// RoBue:
	// Uhrzeit und Counter auf LCD ausgeben
	#if USE_SER_LCD
		lcd_print(1,0,"%2i:%2i:%2i",hh,mm,ss);
		lcd_print(1,12,"%4i",var_array[MAX_VAR_ARRAY-1]);
		#if LCD_ZEILE3_PORTC
		// Status/Pin von PORTC
			lcd_print (3,0,"PORTC:%i%i%i%i%i%i%i%i",((PINC&0b00000001)>>0),((PINC&0b00000010)>>1),((PINC&0b00000100)>>2),((PINC&0b00001000)>>3),((PINC&0b00010000)>>4),((PINC&0b00100000)>>5),((PINC&0b01000000)>>6),((PINC&0b10000000)>>7));
			// Schaltautomatik ein/aus
			if (var_array[9] == 1) {
				lcd_print (3,15,"A");
			}
			else {
				lcd_print (3,15,"-");
			}

		#endif
	#endif

	#if USE_OW
	// RoBue:
	// Zurücksetzen der Min/Max-Werte um 00:00 Uhr einschalten
	if (( hh == 00 )&&( mm == 00 )) {
		minmax = 1;
	}
	#endif

	// ******************************************************************
	// RoBue:
	// 1-Wire-Temperatursensoren (DS18B20) abfragen
	// ******************************************************************

#if USE_OW

		uint8_t i = 0;
		uint8_t subzero, cel, cel_frac_bits;
		uint8_t tempID[OW_ROMCODE_SIZE];

		// Messen bei ss=5,15,25,35,45,55		
		if ( ss%10 == 5 ) {

		// Messen?
		if ( messen == 1 ) {

		// RoBue Anmerkung:
		// Hiermit werden ALLE Sensoren zum Messen aufgefordert.
		// Aufforderung nur bestimmter Sensoren:
		// "NULL" durch "tempID" ersetzen
		
		// RoBue Testausgabe UART:
		// usart_write("Starte Messvorgang ...\r\n");		 

		DS18X20_start_meas( DS18X20_POWER_PARASITE, NULL );
		
		// Kein Messen mehr bis ss=5,15,25,35,45,55
		messen = 0;

		// Jetzt kann ausgelesen werden
		auslesen = 1;

		}	// -> if messen
		
		}	// -> if ss

		// Auslesen bei ss=8,18,28,38,48,58
		if ( ss%10 == 8 ) {

		// Auslesen?
		if ( auslesen == 1 ) {

		// (erste) ID ins RAM holen
		memcpy_P(tempID,DS18B20IDs[0],OW_ROMCODE_SIZE);	
				
		while ( tempID[0] != 0 ) {
		//while ( tempID[0] == 0x10 ) {
				
			// RoBue Anmerkung:
			// Hiermit wird jeweils ein einzelner Sensor ausgelesen
			// und die Temperatur in ow_array abgelegt.
			// Achtung:
			// Pro Sekunde können max. ca. 10 Sensoren ausgelesen werden!
			if ( DS18X20_read_meas( tempID, &subzero,&cel, &cel_frac_bits) == DS18X20_OK ) {

				ow_array[i] = DS18X20_temp_to_decicel(subzero, cel, cel_frac_bits);

				// Minuswerte:
					if ( subzero )
						ow_array[i] *= (-1);
				
				// min/max:
				if ( minmax == 1 ) {
					// Zurücksetzen der Min/Max_Werte 1x/Tag
					// auf die gerade aktuellen Temperaturen 
					ow_array[i+MAXSENSORS] = ow_array[i];
					ow_array[i+MAXSENSORS*2] = ow_array[i];
				}
				else {
					// Abgleich der Temp. mit den gespeicherten Min/Max-Werten
					if (ow_array[i]  < ow_array[i+MAXSENSORS])
        					ow_array[i+MAXSENSORS] = ow_array[i];
               				if (ow_array[i]  > ow_array[i+MAXSENSORS*2])
						ow_array[i+MAXSENSORS*2] = ow_array[i];
        			}

				//TWert = DS18X20_temp_to_decicel(subzero, cel, cel_frac_bits);
				//ow_array[i] = TWert;
				
			}	// -> if
			else {
				usart_write("\r\nCRC Error (lost connection?) ");
				DS18X20_show_id_uart( tempID, OW_ROMCODE_SIZE );


			}	// -> else
		
		// nächste ID ins RAM holen
		memcpy_P(tempID,DS18B20IDs[++i],OW_ROMCODE_SIZE);

		}	// -> while
		
		// RoBue:
		// Temperatur auf LCD ausgeben (IP von Startausgabe (s.o.) wird Überschrieben)
		#if USE_SER_LCD
			
			lcd_print(2,0,"T:              ");	

			// Ausgabe 1. Temperatur auf LCD
			lcd_print(2,3,"%iß",(ow_array[TEMPWERT2]/10));
			// Ausgabe 2./3. Temperatur auf LCD
			lcd_print(2,8,"%iß/%iß",(ow_array[TEMPWERT3]/10),(ow_array[TEMPWERT4]/10));

			// Alternativ: Ausgabe von 2 Temperaturwerten mit Kommastellen:
			// int16_t TWert;
			// TWert = ow_array[TEMPWERT1];
			// if ( ow_array[TEMPWERT1] < 0 ) {
			//	TWert *= (-1);
			//	lcd_print(2,3,"-%i,%iß",(TWert/10),(TWert%10));
			// }
			// else
			//	lcd_print(2,3,"%i,%iß",(TWert/10),(TWert%10));
			// }
			// TWert = ow_array[TEMPWERT2];
			// if ( ow_array[TEMPWERT2] < 0 ) {
			//	TWert *= (-1);
			//	lcd_print(2,3,"-%i,%iß",(TWert/10),(TWert%10));
			// }
			// else
			//	lcd_print(2,3,"%i,%iß",(TWert/10),(TWert%10));
			// }		
	
			#if LCD_ZEILE3_TEMP
				// Weitere 2 Temperaturwerte auf LCD ausgeben (4. Zeile)
				lcd_print(3,0,"T:              ");

				TWert = ow_array[TEMPWERT3];
				// Ausgabe 1. Temperatur auf LCD
				lcd_print(3,3,"%iß",(TWert/10));

				TWert = ow_array[TEMPWERT4];
				// Ausgabe 2. Temperatur auf LCD
				lcd_print(3,9,"%iß",(TWert/10));

			#endif

		#endif

		}	// -> if auslesen

		auslesen = 0;	// Auslesen vorläufig abschalten
		messen = 1;	// Messen wieder ermöglichen
		minmax = 0;	// Min/Max-Werte vergleichen

		}	// -> if ss
		
#endif
	

	// ******************************************************************
	// cni
	// SHT-Sensor (SHT 71) abfragen
	// ******************************************************************
	
#if USE_SHT
		// liest humi und temp vom SHT Sensor und berechnet dewpoint
		unsigned char sht_checksum, sht_error=0;
		sht_value humidity;
		sht_value temperature;
		float dewpoint;
		unsigned char do_cycle=0;
		do
		{
			if (do_cycle>1) delay_ms(200);
			do_cycle++;
		  
			sht_error=sht_softreset(SHT_DATA_PIN);
			if (sht_error) {
				usart_write("SHT Error (SoftReset) %i\r\n", sht_error);
				break;
			}

			sht_error=sht_measure(&humidity, &sht_checksum, HUMI, SHT_DATA_PIN);
			if (sht_error) {
				usart_write("SHT Error (HUMI) %i\r\n", sht_error);
				break;
			}
		  
			sht_error=sht_measure(&temperature, &sht_checksum, TEMP, SHT_DATA_PIN);
			if (sht_error) {
				usart_write("SHT Error (TEMP) %i\r\n", sht_error);
				break;
			}
	   
			sht_raw_to_physical(&humidity, &temperature);
			
			if (do_cycle>=5) break;
		}
		while ( (humidity.f<100/*1*/) ||
			(humidity.f>9900/*99*/) ||
			(temperature.f<-3900/*-39*/) ||
			(temperature.f>10000/*100*/) );
	   
		dewpoint = calc_dewpoint(humidity.f,temperature.f);
	   
 		// Werte in VA00 bis VA02 schreiben
		var_array[0] = temperature.f;
	    var_array[1] = humidity.f;
		var_array[2] = dewpoint;

		// Werte evtl. auf LCD ausgeben
		#if LCD_ZEILE3_SHT
			lcd_print(3,0,"T:     H:       ");
			lcd_print(3,3,"%iß",var_array[0]);
			lcd_print(3,10,"%i%",var_array[2]);
		#endif	
#endif


	// **********************************************************
	// RoBue:
	// Schalten der Ports (PORTC) durch bestimmte Bedingungen
	// - Temperatur (1-Wire -> PORTA7) mit/ohne Lüftungsautomatik
	// - digital, analog (-> PORTA0-6)
	// - Zeit
	// **********************************************************

#if USE_AUTOMATIK

	int16_t TWert;
	
	// Automatik eingeschaltet?
	if ( var_array[9] == 1 ) {

	if ( ss%10 == 1 ) {
		schalten = 1;
	}

	// Abfrage bei ss =0,10,20,30,40,50
	if ( ss%10 == 0 ) {
	if ( schalten == 1 ) {
	
	// RoBue Testausgabe UART:
	// usart_write("%2i:%2i: Schaltfunktionen testen ...\r\n",hh,mm);	


	// PORTC0:
	// Über Temperatur: var_array[10] - Sensor0
	TWert = var_array[10]*10;		
	if ( ow_array[0] < TWert ) {
		PORTC |= (1 << PC0); // ein
	//
	// Über Temperatur: var_array[10] - Sensor0 - PORTA0
	// if ((PINA&0b00000001) == 0 ) {		// PORTA0: Fenster geschlossen?
	//	if ( ow_array[0] < TWert ) {
	//		PORTC |= (1 << PC0); // ein
	//	}
	//  	else { 
	//     		PORTC &= ~(1 << PC0); // aus
	//	}
	}
	else {
	      	PORTC &= ~(1 << PC0); // aus
	}

	// PORTC1:
	// Über Temperatur: var_array[11] - Sensor1
	TWert = var_array[11]*10;
	// if ( ow_array[1] < TWert ) {
	//	PORTC |= (1 << PC1); // ein
	//
	// Über Temperatur: var_array[11] - Sensor1 - PORTA1
	if ((PINA&0b00000010) == 0 ) {		// PORTA1: Fenster geschlossen?
		if ( ow_array[1] < TWert ) {
			PORTC |= (1 << PC1); // ein
		}
		else {
			PORTC &= ~(1 << PC1); // aus
		}
	}
	else {
		PORTC &= ~(1 << PC1); // aus
	}
		
	// PORTC2:
	// Über Temperatur: var_array[12] - Sensor2
	// TWert = var_array[12]*10;
	// if ( ow_array[2] < TWert ) {						
	//	PORTC |= (1 << PC2); // ein
	// Über Temperatur: var_array[12] - Sensor2 - PORTA2
	// if ((PINA&0b00000100) == 0 ) {		// PORTA2: Fenster geschlossen?
	//	if ( ow_array[2] < TWert )  {
	//		PORTC |= (1 << PC2); // ein
	//	}
	//	else {
	//		PORTC &= ~(1 << PC2); // aus
	//	}
	//}
	//else {
	//	PORTC &= ~(1 << PC2); // aus
	//}

	// PORTC4:
	// Über 2 Temperaturen (Differenz)
	// var_array[13] - Sensor3 Vorlauf, Sensor4 Ruecklauf
	//
	// z.B. Zirkulationspumpe für Warmwasser
	// Achtung: Temp. von Sensor3 MUSS höher/gleich sein als Temp. von Sensor4
	// Ansonsten laeuft die Pumpe immer
	// Ausserdem sollte die Pumpe 1x pro Stunde anlaufen (Leginollengefahr!)
	//
		if ( (mm == 00) || ((ow_array[3]/10 - ow_array[4]/10) >= var_array[14]) ) {						
		PORTC |= (1 << PC4); // ein
	}
	else {
		PORTC &= ~(1 << PC4); // aus
	}

		// PORTC6:
	// Über Analogwert:
	if ( var_array[6] > var_array[18] ) {
		PORTC |= (1 << PC6); // ein
	}
	else {
		PORTC &= ~(1 << PC6); // aus
	}

	// PORTC7:
	// Über Zeit: ein/aus
	if ( hh == var_array[20] ) {
		if ( mm == var_array[21] ) {
			PORTC |= (1 << PC7); // ein
		}
	}
	if ( hh == var_array[22] ) {
		if ( mm == var_array[23] ) {
			PORTC &= ~(1 << PC7); // aus
		}
	}


	schalten = 0;	// Vorerst nicht mehr schalten

	} // -> schalten == 1
	} // -> if ss == ...
	} // -> if var_array == 1
	
#endif
	
        //Wetterdaten empfangen (Testphase)
        #if GET_WEATHER
        http_request ();
        #endif
        
        //Empfang von Zeitinformationen
	#if USE_NTP
		if(!ntp_timer){
		ntp_timer = NTP_REFRESH;
		ntp_request();
		}
	#endif //USE_NTP
		
        //Versand von E-Mails
        #if USE_MAIL
	        if (mail_enable == 1)
	        {
	            mail_enable = 0;
	            mail_send();
	        }
        #endif //USE_MAIL
        
        //Rechner im Netzwerk aufwecken
        #if USE_WOL
	        if (wol_enable == 1)
	        {
	            wol_enable = 0;
	            wol_request();
	        }
        #endif //USE_WOL
           
	//USART Daten für Telnetanwendung?
	telnetd_send_data();
    }

return(0);
}
