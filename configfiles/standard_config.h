/*----------------------------------------------------------------------------
Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
Author:         Radig Ulrich
Remarks:        
known Problems: none
Version:        03.11.2007
Description:    Webserver Config-File

-----------------------------------------------------------------------------
 
Modifiziert für AVR-NET-IO von Pollin durch RoBue u.a.

**************************************************
*Achtung: Bitte alles hier sorgfaeltig durchlesen*
**************************************************

Grundidee: Konfiguration über EINE Datei (= config.h)
-----------------------------------------------------

- Taktfrequenz
- PORTS
- LCD
- Bezeichnungen für die Schaltausgänge (-> webpage.h)
- Netzwerkeinstellungen (MAC, IP. Subnet, Routing, NTP, WOL)
- 1-Wire-Sensoren (Anzahl, ID)
- ...
Rest lässt sich über UART/Telnet und Webpage konfigurieren

 
Konfiguration (vgl. main.c, httpd.c, webpage.h):
-----------------------------------------------
 
PORTA -> Eingabe (A0-3 digital, A4-6 analog, A7 1-Wire)
PORTB -> ENC-Netwerkcontroller (PB0 -> LCD)
PORTC -> Schaltausgaenge (z.B. Pollin Relaiskarte)
PORTD -> D0,1 -> UART, RS232, WebCam
          D2-7 -> LCD (4-Bit Mode), Schaltausgaenge, Servo

PORTD ist fuer zusaetzliche "Spielereien" gedacht.
Dabei ist jedoch nicht alles zugleich möglich!
 

Standard-Beispiel-Konfiguration von config.h fuer V1.5:
------------------------------------------------------

UART/RS232 (PORTD0,1)
LCD im 4-Bit Mode an PORTD2-7, PORTB0 (= EXT-Port) 

Temperaturmessung mit 5 Sensoren (DS1820)
	- Raum_1
	- Raum_2
	- Aussen
	- Zirkulationspumpe (Rücklauf, Vorlauf)
(Die IDs der Sensoren muessen in config.h. manuell eingegeben werden.
Es gibt bisher noch keine sinnvolle automatische Suchfunktion.
Platz ist fuer max. 8 Sensoren reserviert.)
 

Schaltfunktionen (vgl. main.c, httpd.c, webpage.h):
	- PORTC0: Raum_1 abhaengig von Sensor 0
	- PORTC1: Raum_2 abhaengig von Sensor 1 und Fenster offen/zu (PORTA1)
	- PORTC4: Zirkulationspumpe mit Sensor 3,4
	- PORTC6: Licht ein/aus abhaengig von LDR an PORTA6 (AD-Wert)
	- PORTC7: Zeitschaltuhr


Erweiterungsmoeglichkeiten in V1.5 ueber config.h:
-------------------------------------------------

- minimal (nur Schaltfunktionen)
	#define OUTC 			0xFF
	#define OUTD 			0xFC
	#define USE_PORTD_SCHALT	1
	#define USE_SER_LCD		0
	#define USE_OW			0
	#define USE_AUTOMATIK		0

- WebCam
	#define F_CPU 14745600UL
	#define USE_CAM			1
	#define CAM_RESELUTION		0
(UART/RS232 faellt automatisch weg)

- Servo (evtl. in Zusammenarbeit mit WebCam)
	#define USE_SERVO		1
	#define OUTD 			0xFC
(PORTD7 wird benoetigt, d.h. kein LCD mehr moeglich)
	#define USE_SER_LCD		0 (LCD im seriellen Mode geht)
	#define USE_LCD_4Bit		0 (LCD im 4-Bit Mode geht auf keinen Fall!)
(Schaltfunktion fuer PORTD abschalten)
	#define USE_PORTD_SCHALT	0
	
- PORTD2-7 fuer zusaetzliche Schaltfunktionen
	#define USE_PORTD_SCHALT	1	
	#define OUTD 			0xFC
(Kein LCD, kein Servo moeglich)
	#define USE_SERVO		0
	#define USE_SER_LCD		0

- SHT71-Sensor
	#define USE_SHT			1
(In makefile bitte dann aendern:
	# cni:
	# Add LibSHT
	SRC += sht/libsht.c
Braucht viel Platz!)

Natürlich sind auch andere Kombinationen möglich,
dies erfordert aber den Eingriff in andere Programmteile (Sourcen)
-> main.c, httpd.c, webpage.h.
Solche Änderungen sollten über sog. MOD-Files erfolgen,
die die vorhandenen Source-File durch entsprechend angepasste ersetzen.

Achtung:
Falls bestimmte Routinen gar nicht gebraucht werden (z.B. LCD, 1-Wire),
spart es Platz, diese auch in "makefile" zu deaktivieren ("#" davorsetzen).

Viel Spaß und Erfolg, RoBue
	
-----------------------------------------------------------------------------

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
	#define RoBueVers "Version 1.5 RoBue vom 18.02.2009"
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
	#define BAUDRATE 9600
	// define BAUDRATE 19200 // -> Handy Siemens C35i
	

// PORTS (HEX) (1=OUTPUT / 0=INPUT)
// --------------------------------

	// PORTA: 0-3 -> dig. input, 4-6 -> analog. input, 7 - 1-Wire
	// (PORTA4-7 ist mit blauen Schraubklemmen verbunden)
	#define OUTA 			0x00

	//PORTB -> ENC-Netzwerkkontroller
	//nur ändern wenn man weiß was man macht!

	// PORTC: Schaltausgaenge
	#define OUTC 			0xFF

	// PoRTD: 0,1 -> UART, 2-7 Schaltausgänge
	// PORTD2-7 als zusaetzliche Schaltausgaenge
	// Das bedeutet: #define USE_SER_LCD 0 (Kein LCD!)
	#define USE_PORTD_SCHALT	0
	// Fuer LCD, Schaltausgaenge, Servo:
	#define OUTD 			0xFC

	//AD-Wandler benutzen?
	#define USE_ADC			1


// LCD Einstellungen (-> PORTD)
// -----------------

	// LCD ja/nein
	#define USE_SER_LCD		1
	// LCD im 4Bit Mode oder seriell
	// Das bedeutet: 
	// #define USE_PORTD_SCHALT	0 (PORTD kein Schaltausgang!)
	// #define USE_SERVO		0 (Kein Servo an PORTD7!)
	#define USE_LCD_4Bit		1
	// LCD im 4-Bit: 1 1; LCD seriell: 1 0 

	//Anzahl der Zeilen 1,2 oder 4
	//#define ONE_LINES					
	//#define TWO_LINES					
	//#define THREE_LINES					
	#define FOUR_LINES					

	//LCD-Belegung im seriellen Modus (PORTD2-4):
	//LCD_DATA_ENABLE	-> PORTD2
	//LCD_DATA	-> PORTD3
	//LCD_CLOCK	-> PORTD4

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

	// Einstellungen zur Ausgabe -> main.c
	// Welche Sensoren:
	#define TEMPWERT1	0	// Raum_1
	#define TEMPWERT2	1	// Raum_2
	#define TEMPWERT3	3	// Pumpe VL
	#define TEMPWERT4	4	// Pumpe RL
	// Was erscheint in der 4.Zeile:
	// Nur alternativ moeglich !!!
	#define LCD_ZEILE3_TEMP	 0	// 1 -> TEMPWERT3+4
	#define LCD_ZEILE3_PORTC 1	// 1 -> Status/Pins PORTC
	#define LCD_ZEILE3_SHT	 0	// 1 -> SHT71-Ausgabe


// Namen für die dig. Eingänge (PORTA0-3) -> webpage.h
// ---------------------------------------------------

	#define Eing_A0 "A0_Fenster_1"
	#define Eing_A1 "A1_Fenster_2"
	#define Eing_A2 "A2_"
	#define Eing_A3 "A3_"


// Namen für die Schaltausgänge -> webpage.h (abhängig von LCD)
// -----------------------------------------

	// Zuordnung der Aufgaben von PORTC0-7 -> webpage.h
	// Achtung: Kein LCD an PORTC anschließen! Beschädigung möglich.
	#define Ausg_C0	"C0_Relais_1"
	#define Ausg_C1	"C1_Relais_2"
	#define Ausg_C2	"C2_Relais_3"
	#define Ausg_C3	"C3_Relais_4"
	#define Ausg_C4	"C4_Relais_5"
	#define Ausg_C5	"C5_Relais_6"
	#define Ausg_C6	"C6_Relais_7"
	#define Ausg_C7	"C7_Relais_8"	


// Netzwerkeinstellungen
// ---------------------

	//ETH_M32_EX (www.ulrichradig.de)
	#define USE_ENC28J60		1
	//Holger Buss (www.mikrocontroller.com) Mega32-Board
	#define USE_RTL8019		0

	//IP des Webservers
	#define MYIP		IP(192,168,0,99)

	//Netzwerkmaske
	#define NETMASK		IP(255,255,255,0)

	//IP des Routers
	#define ROUTER_IP	IP(192,168,0,3)

	//IP des NTP-Servers z.B. Server 1.de.pool.ntp.org
	#define USE_NTP		1 // 1= NTP Client on
	#define NTP_IP		IP(77,37,6,59)

	//Broadcast-Adresse für WOL
	#define USE_WOL		0 //1 = WOL on
	#define WOL_BCAST_IP	IP(192,168,0,255)
	#define WOL_MAC 	{0x00,0x1A,0xA0,0x9C,0xC6,0x0A}
	
	//MAC Adresse des Webservers	
	#define MYMAC1	0x00
	#define MYMAC2	0x22
	#define MYMAC3	0xF9
	#define MYMAC4	0x01
	#define MYMAC5	0x00
	#define MYMAC6	0x01

	//Umrechnung von IP zu unsigned long
	#define IP(a,b,c,d) ((unsigned long)(d)<<24)+((unsigned long)(c)<<16)+((unsigned long)(b)<<8)+a

	//Webserver mit Passwort? (0 == mit Passwort)
	#define HTTP_AUTH_DEFAULT	1
	
	//AUTH String "USERNAME:PASSWORT" max 14Zeichen 
	//für Username:Passwort
	#define HTTP_AUTH_STRING "admin:uli1"


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
	#define OW_ID_T01	{0x10,0x7a,0x8c,0x48,0x01,0x08,0x00,0x67}
	#define OW_ID_T02	{0x10,0x44,0x9e,0x53,0x01,0x08,0x00,0xca}
	#define OW_ID_T03	{0x10,0x02,0xaF,0x53,0x01,0x08,0x00,0x44}
	#define OW_ID_T04	{0x10,0x9e,0x7d,0x98,0x01,0x08,0x00,0xf8}
	#define OW_ID_T05	{0x10,0x00,0x6c,0x98,0x01,0x08,0x00,0xeb}
	#define OW_ID_T06	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T07	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_T08	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00} // leer
	#define OW_ID_Last	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}

	// Startwerte in ow_array
	#define OW_START	150
	#define OW_MINMAX	0

	// Entsprechende Ortszuweisung für die Sensoren fuer webpage.h:
	#define T00	"Raum_1"
	#define T01	"Raum_2"
	#define T02	"Aussen"
	#define T03	"Pumpe VL"
	#define T04	"Pumpe RL"
	#define T05	"Reserve"
	#define T06	"Reserve"
	#define T07	"Reserve"


// cni
// SHT Sensor einbinden
// --------------------
//
// In makefile bitte dann aendern bzw. aktivieren:
//	# cni:
//	# Add LibSHT
//	SRC += sht/libsht.c
//
// Braucht viel Platz!!!
//
// 2 Versionen:
// - Version an PORTD5,6 wird hier in config.h konfiguriert
// - Version an PORTA5,6 wird über eigene MOD-Files eingebunden

	#define USE_SHT			0
	// SHT SCK and DATA Port and Pin definitions
	#define SHT_SCK_PORT		PORTD
	#define SHT_SCK_DDR		DDRD
	#define SHT_SCK_PIN		PD5
	#define SHT_DATA_PORT		PORTD
	#define SHT_DATA_DDR		DDRD
	#define SHT_DATA_PORT_PIN	PIND
	#define SHT_DATA_PIN		PD6

	

// Kamera und Servo mit einbinden
// ------------------------------
//Achtung: Kamera arbeitet nur mit einem 14,7456Mhz Quarz!

	#define USE_CAM		0
	//In cam.c können weitere Parameter eingestellt werde
	//z.B. Licht, Kompression usw.
	//Auflösungen
	//0 = 160x120 Pixel kürzer (zum testen OK ;-)
	//1 = 320x240 Pixel ca. 10 Sek. bei einem Mega644
	//2 = 640x480 Pixel länger (dauert zu lang!)
	#define CAM_RESELUTION	0

	#define USE_SERVO	0
	// Das bedeutet:
	// #define USE_SER_LCD		0 (LCD im seriellen Mode geht evtl.)
	// #define USE_LCD_4Bit		0 (LCD im 4-Bit Mode geht auf keinen Fall!)
	// #define USE_PORTD_SCHALT	0 (PORTD geht nicht zum Schalten)
	//
	// Weitere Einstellungen Servo (varrieren je nach Hersteller leicht):
	#define SERVO_MIN	14	// linker Anschlag	
	#define SERVO_MAX	36	// rechter Anschlag


// Rest
// ----

	// Schaltautomatik installieren (-> main.c, httpd.c, webpage.h)
	#define USE_AUTOMATIK	1

	// Feuchtesensor HIH4000 (experimentell)
	#define USE_HIH4000	0
	#define VA_IN_HIH4000	4
	#define VA_OUT_HIH4000	24
	
	//Emailversand benutzen? Konfiguration des
	//Emailclient in der Sendmail.h
	#define USE_MAIL        0
    
	//Empfang von Wetterdaten auf der Console (über HTTP_GET)
	#define GET_WEATHER     0

	// Lokale Wetterdaten
	#define WETTER24	1
	#define LOCALWEATHER24	"http://www.wetter24.de/nc/de/home/wetter/weltwetter/ortewetter.html?cityID=49X7241&type=98"
    
	//Commandos und Ausgaben erfolgen über Telnet
	//UART/RS232 geht dann nicht mehr!
	#define CMD_TELNET      0

	#define MAX_VAR_ARRAY	30	//    ursprünglich 10
	// RoBue:
	// Variablen-Array 
	// zum Abspeichern verschiedener Werte
	// und zum Einfügen in die Webseite %VA@00 bis %VA@29
	// VA0-3	-> ??? / SHT71
	// VA4-7	-> Analogwert von PORTA4-7
	// VA8		-> Reserve (Servo-Position)
	// VA9		-> Schaltautomatik ein/aus
	// VA10-17	-> Schalttemperaturen
	// VA18,19	-> Schaltwert analog
	// VA20-23	-> Schaltzeiten ein/aus hh,mm,hh,mm
	// VA24-28	-> Reserve / HIH4000
	// VA-Ende	-> Counter
	
    
#endif //_CONFIG_H