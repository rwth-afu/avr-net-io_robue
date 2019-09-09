/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        03.11.2007
 Description:    Webserver Config-File

 ---

 Modifiziert für AVR-NET-IO von Pollin durch RoBue u.a.
 
 Grundidee: Konfiguration über EINE Datei (= config.h)
	- Taktfrequenz
	- PORTS
	- LCD
	- Bezeichnungen für die Schaltausgänge (-> webpage.h)
	- Netzwerkeinstellungen (MAC, IP. Subnet, Routing, NTP, WOL)
	- 1-Wire-Sensoren (Anzahl, ID)
	- ...
 Rest lässt sich über UART/Telnet und Webpage konfigurieren 
 
 ---

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

#ifndef _CONFIG_H_
	#define _CONFIG_H_	
	
	//Version
	#define SoftVers "V1.0.99"
	#define RoBueVers "Version 1.5 RoBue vom 21.01.2009 + SHT Mod von cni vom 05.02.2009"
	#define Version "1.5"
	

// Konfiguration des Systems
// -------------------------

	//Taktfrequenz (bei AVR-NET-IO i.d.R. 16MHz
	#define F_CPU 16000000UL	
	//#define F_CPU 14745600UL
	//#define F_CPU 11059200UL
	
	//Timertakt intern oder extern
	#define EXTCLOCK 0 //0=Intern 1=Externer Uhrenquarz

	//Baudrate der seriellen Schnittstelle
	//#define BAUDRATE 9600
	#define BAUDRATE 19200
	// define BAUDRATE 19200 // -> Handy Siemens C35i
	

// PORTS (HEX) (1=OUTPUT / 0=INPUT)
// --------------------------------

	// PORTA: 0-3 -> dig. input, 4-6 -> analog. input, 7 - 1-Wire
	// (PORTA4-7 ist mit blauen Schraubklemmen verbunden)
	#define OUTA 		0x00

	//PORTB -> ENC-Netzwerkkontroller
	//nur ändern wenn man weiß was man macht!

	// PORTC: Schaltausgaenge
	#define OUTC 		0xFF

	// PoRTD: 0,1 -> UART, 2-7 Schaltausgänge
	// PORTD2-7 als zusaetzliche Schaltausgaenge
	// Das bedeutet: #define USE_SER_LCD 0 (Kein LCD!)
	#define PORTD_SCHALT	0
	#define OUTD 		0xFC

	//AD-Wandler benutzen?
	#define USE_ADC		1


// LCD Einstellungen (-> PORTD)
// -----------------

	// LCD ja/nein
	#define USE_SER_LCD		0
	// LCD im 4Bit Mode oder seriell
	// Das bedeutet: #define PORTD_SCHALT 0 (PORTD kein Schaltausgang!)
	#define USE_LCD_4Bit	1
	// LCD im 4-Bit: 1 1; LCD seriell: 1 0 

	//Anzahl der Zeilen 1,2 oder 4
	//#define ONE_LINES					
	#define TWO_LINES					
	//#define THREE_LINES					
	//#define FOUR_LINES					

	//LCD-Belegung 4-Bit-Modus (PORTD, EXT):
	//EXT	LCD		PORT
	// 9	--> Pin 1 GND
	//10	--> Pin 2 +5V
	// 1	--> Pin  4 RS	PORTD2
	// 2	--> Pin  5 RW	PORTD3
	// 7	--> Pin  6 E	PORTB0
	//GND	--> Pin  7
	//GND	--> Pin  8
	//GND	--> Pin  9
	//GND	--> Pin 10
	// 3	--> Pin 11 DB4	PORTD4
	// 4	--> Pin 12 DB5	PORTD5
	// 5	--> Pin 13 DB6	PORTD6
	// 6	--> Pin 14 DB7	PORTD7
	// 8	--> Hintergrundbeleuchtung


// Namen für die dig. Eingänge (PORTA0-3) -> webpage.h
// ---------------------------------------------------

	#define Eing_A0 "A0"
	#define Eing_A1 "A1"
	#define Eing_A2 "A2"
	#define Eing_A3 "A3"


// Namen für die Schaltausgänge -> webpage.h (abhängig von LCD)
// -----------------------------------------

	// Zuordnung der Aufgaben von PORTC0-7 -> webpage.h
	// Achtung: Kein LCD an PORTC anschließen! Beschädigung möglich.
	#define Ausg_C0	"C0"
	#define Ausg_C1	"C1"
	#define Ausg_C2	"C2"
	#define Ausg_C3	"C3"
	#define Ausg_C4	"C4"
	#define Ausg_C5	"C5"
	#define Ausg_C6	"C6"
	#define Ausg_C7	"C7"	


// Netzwerkeinstellungen
// ---------------------

	//ETH_M32_EX (www.ulrichradig.de)
	#define USE_ENC28J60	1
	//Holger Buss (www.mikrocontroller.com) Mega32-Board
	#define USE_RTL8019		0

	//IP des Webservers
	#define MYIP		IP(192,168,178,46)

	//Netzwerkmaske
	#define NETMASK		IP(255,255,255,0)

	//IP des Routers
	#define ROUTER_IP	IP(192,168,178,1)

	//IP des NTP-Servers z.B. Server 1.de.pool.ntp.org
	#define USE_NTP		1 // 1= NTP Client on
	#define NTP_IP		IP(77,37,6,59)

	//Broadcast-Adresse für WOL
	#define USE_WOL		0 //1 = WOL on
	#define WOL_BCAST_IP	IP(192,168,178,255)
	#define WOL_MAC 	{0x00,0x1A,0xA0,0x9C,0xC6,0x0A}
	
	//MAC Adresse des Webservers	
	#define MYMAC1	0x00
	#define MYMAC2	0x22
	#define MYMAC3	0xF9
	#define MYMAC4	0x01
	#define MYMAC5	0x1C
	#define MYMAC6	0x04

	//Umrechnung von IP zu unsigned long
	#define IP(a,b,c,d) ((unsigned long)(d)<<24)+((unsigned long)(c)<<16)+((unsigned long)(b)<<8)+a

	//Webserver mit Passwort? (0 == mit Passwort)
	#define HTTP_AUTH_DEFAULT	1
	
	//AUTH String "USERNAME:PASSWORT" max 14Zeichen 
	//für Username:Passwort
	#define HTTP_AUTH_STRING "user:password"


// 1-Wire-Einstellungen
// --------------------

	// Konzipiert fuer 8 Sensoren, auch wenn weniger genutzt werden.
	// In webpage.h sind z.Z. nur Eintraege fuer 4 vorhanden.

	// 1-Wire ja/nein (0=nein, 1=ja)
	#define USE_OW		1
	// max. Anzahl der Sensoren
	#define MAXSENSORS	8
	// z.Z. nur EIN 1-Wire-Bus
	#define OW_ONE_BUS	1
	// Länge der ID (rom-code) des Sensors incl. CRC
	#define OW_ROMCODE_SIZE 8

	// Port fuer 1-Wire
	#define OW_PIN  PA7
	#define OW_IN   PINA
	#define OW_OUT  PORTA
	#define OW_DDR  DDRA

	// ROM-IDs der DS1820 müssen "per Hand" eingetragen werden!
	// Damit kann man am leichtesten einem Sensor eine Aufgabe zuordnen.
	// Die IDs kann man mit dem Flash-File "sensoren.hex" ermitteln.
	#define OW_ID_T01	{0x28,0xf5,0xfb,0x14,0x01,0x00,0x00,0xcc}
	#define OW_ID_T02	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T03	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T04	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T05	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T06	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T07	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T08	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_Last	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

	// Startwerte in ow_array
	#define OW_START	150
	#define OW_MINMAX	0

	// Entsprechende Ortszuweisung für die Sensoren fuer webpage.h:
	#define T01	"Keller"
	#define T02	"Reserve"
	#define T03	"Reserve"
	#define T04	"Reserve"
	#define T05	"Reserve"
	#define T06	"Reserve"
	#define T07	"Reserve"
	#define T08	"Reserve"

// cni
// SHT Sensor einbinden
// --------------------

	#define USE_SHT				1
	// SHT SCK and DATA Port and Pin definitions
	#define SHT_SCK_PORT		PORTA
	#define SHT_SCK_DDR			DDRA
	#define SHT_SCK_PIN			PA5
	#define SHT_DATA_PORT		PORTA 
	#define SHT_DATA_DDR		DDRA
	#define SHT_DATA_PORT_PIN	PINA
	#define SHT_DATA_PIN		PA6
	
	
// Kamera mit einbinden
// --------------------
//Achtung: Kamera arbeitet nur mit einem 14,7456Mhz Quarz!

	#define USE_CAM			0
	#define USE_SERVO		0
	//In cam.c können weitere Parameter eingestellt werde
	//z.B. Licht, Kompression usw.
	//Auflösungen
	//0 = 160x120 Pixel kürzer (zum testen OK ;-)
	//1 = 320x240 Pixel ca. 10 Sek. bei einem Mega644
	//2 = 640x480 Pixel länger (dauert zu lang!)
	#define CAM_RESELUTION	0
		

// Rest
// ----	

	//Emailversand benutzen? Konfiguration des
	//Emailclient in der Sendmail.h
	#define USE_MAIL        0
    
	//Empfang von Wetterdaten auf der Console (über HTTP_GET)
	#define GET_WEATHER     0

	// Lokale Wetterdaten
	#define WETTER24	0
	#define LOCALWEATHER24	"http://www.wetter24.de/nc/de/home/wetter/weltwetter/ortewetter.html?cityID=49X7241&type=98"
    
	//Commandos und Ausgaben erfolgen über Telnet
	//UART/RS232 geht dann nicht mehr!
	#define CMD_TELNET      0

	#define MAX_VAR_ARRAY	30	//    ursprünglich 10
	// RoBue:
	// Variablen-Array 
	// zum Abspeichern verschiedener Werte
	// und zum Einfügen in die Webseite %VA@00 bis %VA@29
	// VA0-3	-> ???
	// VA4-7	-> Analogwert von PORTA4-7
	// VA8		-> Reserve
	// VA9		-> Manueller Betrieb ein/aus
	// VA10-17	-> Schalttemperaturen
	// VA18,19	-> Schaltwert analog
	// VA20-23	-> Schaltzeiten ein/aus hh,mm,hh,mm
	// VA24-28	-> Reserve
	// VA-Ende	-> Counter
	
    
#endif //_CONFIG_H