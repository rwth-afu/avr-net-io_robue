/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        09.11.2007
 Description:    Html Seiten

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
#ifndef _WEBPAGE_H
	#define _WEBPAGE_H

//****************************************************************************
//Dateien und Webseiten am Ende dieser Seite in Tabelle eintragen !!!!!!!
//****************************************************************************


//----------------------------------------------------------------------------
//Dazustellende Webseite
	PROGMEM char const Page1[] = {
	"<html><head><meta http-equiv=\"refresh\" content=\"30\" text/html; charset=iso-8859-1\">\r\n"

	"<title>DB0KWE ATV & Hamnet Relais</title>\r\n"

	"<style type=\"text/css\">\r\n"
  	".fett {font-weight: bold; margin-top:15px;}\r\n"
  	"td { cellpadding: 2px; }\r\n"
  	"body { font-family: arial;  font-size: 1.0 em; color: #000000; background-color: rgb(255, 255, 204)}\r\n"
  	"h1 {text-align: center; font-weight: bold; color: white; font-size: 1.2 em}\r\n"
	"</style>\r\n"

	"</head><body><center>\r\n"

	"<table style=\"width: 600px; text-align: left; background-color: rgb(153, 153, 255);\" cellpadding=\"5\">\r\n"
	"<tr><td><h1>DB0KWE ATV & Hamnet Relais</h1>\r\n"
	"<table style=\"width: 100%; font-weight: bold;\">\r\n"
        "<tr><td>Serverzeit: %TI@</td><td style=\"text-align: right;\">Pagecounter: %VA@29</td></tr></table>\r\n"
	"</td></tr></table>\r\n"


// Statusanzeige PORTA ------------------------------------------------------
	"<div class=\"fett\">Status PORTA:</div>\r\n"

	"<table style=\"width: 600px; text-align: left; background-color: rgb(255, 255, 153);\" border=\"0\">\r\n"

	"<tr><td>dig. Eingang</td><td style=\"width:35px;\">auf/zu</td><td align=\"center\">analog. Eingang</td><td>Wert (0-1023)</td></tr>\r\n"

	"<tr><td>"
	Eing_A0
	"</td><td style=\"background-color: rgb(%PINA0);\"></td><td align=\"center\">ADC1 Netzteil</td><td>%VA@05</td></tr>\r\n"

	"<tr><td>"
	Eing_A1
	"</td><td style=\"background-color: rgb(%PINA1);\"></td><td align=\"center\">ADC2 APRS</td><td>%VA@06</td></tr>\r\n"

	"<tr><td>"
	Eing_A2
	"</td><td style=\"background-color: rgb(%PINA2);\"></td><td align=\"center\">ADC3 Funkruf</td><td>%VA@04</td></tr>\r\n"

	"<tr><td>"
	Eing_A3
	"</td><td style=\"background-color: rgb(%PINA3);\"></td><td align=\"center\">AD-A7</td><td>(-> 1-Wire)</td></tr>\r\n"


	"</table>\r\n"

// Statusanzeige PORTA -----------------------------------------------------


// 1-Wire ------------------------------------------------------------------
#if USE_OW

  "<div class=\"fett\">1-Wire-Temperatursensoren:</div>\r\n"

  "<table style=\"width: 600px; text-align: left; background-color: rgb(255, 255, 102);\" border=\"0\">\r\n"
  "<tr><td>1-Wire</td><td>Ort</td><td align=\"right\">Wert</td><td align=\"left\">Einheit</td><td>Min/Max(Tag)</td></tr>\r\n"
  "<tr><td>Sensor 0</td><td>"T00"</td><td align=\"right\">%OW@00</td><td align=\"left\">&deg;C</td><td>%OW@08 / %OW@16</td></tr>\r\n"
  "<tr><td>Sensor 1</td><td>"T01"</td><td align=\"right\">%OW@01</td><td align=\"left\">&deg;C</td><td>%OW@09 / %OW@17</td></tr>\r\n"
  "<tr><td>Sensor 2</td><td>"T02"</td><td align=\"right\">%OW@02</td><td align=\"left\">&deg;C</td><td>%OW@10 / %OW@18</td></tr>\r\n"
  "<tr><td>Sensor 3</td><td>"T03"</td><td align=\"right\">%OW@03</td><td align=\"left\">&deg;C</td><td>%OW@11 / %OW@19</td></tr>\r\n"
  "<tr><td>Sensor 4</td><td>"T04"</td><td align=\"right\">%OW@04</td><td align=\"left\">&deg;C</td><td>%OW@12 / %OW@20</td></tr>\r\n"

// Je nach Bedarf weitere Zeilen einfuegen
//  "<tr><td>Sensor 5</td><td>"T05"</td><td align=\"right\">%OW@05</td><td align=\"left\">&deg;C</td><td>%OW@13 / %OW@21</td></tr>\r\n"
//  "<tr><td>Sensor 6</td><td>"T06"</td><td align=\"right\">%OW@06</td><td align=\"left\">&deg;C</td><td>%OW@14 / %OW@22</td></tr>\r\n"
//  "<tr><td>Sensor 7</td><td>"T07"</td><td align=\"right\">%OW@07</td><td align=\"left\">&deg;C</td><td>%OW@15 / %OW@23</td></tr>\r\n"	

  "</table>\r\n"

#endif
// 1-Wire -------------------------------------------------------------------


// cni
// Ausgabe der SHT Messwerte
// SHT ------------------------------------------------------------------
#if USE_SHT // Version an PORTD5,6

  "<div class=\"fett\">SHT-Sensor:</div>\r\n"

  "<table style=\"width: 600px; text-align: left; background-color: rgb(255, 204, 102);\" border=\"0\">\r\n"
  "<tr><td>Sensor</td><td>Typ</td><td align=\"right\">Wert</td><td align=\"left\">Einheit</td></tr>\r\n"
  "<tr><td>SHT 71</td><td>Temperatur</td><td align=\"right\">%VA@00</td><td align=\"left\">&deg;C</td></tr>\r\n"
  "<tr><td>SHT 71</td><td>Luftfeuchte</td><td align=\"right\">%VA@01</td><td align=\"left\">%rH</td></tr>\r\n"
  "<tr><td>SHT 71</td><td>Taupunkt</td><td align=\"right\">%VA@02</td><td align=\"left\">&deg;C</td></tr>\r\n"
  "</table>\r\n"

#endif
// SHT -------------------------------------------------------------------

// => Schalten mit PORTC
// checkbox-value-Werte A-H -> httpd

// Schaltanweisungen --------------------------------------------------------
#if USE_AUTOMATIK

	"<div class=\"fett\">Schaltanweisungen:</div>\r\n"

	"<table style=\"width: 600px; text-align: left; background-color: rgb(255, 204, 51);\" border=\"0\">\r\n"
    	"<tr><td><form name=\"form1\" method=\"post\" action=\" \">\r\n"
	
	"<table style=\"width: 100%; text-align: left;\" border=\"0\">\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"X\"%PORTX9 > Automatik<hr></td><td>%VA@09<hr></td><td>(ein/aus) <hr></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"A\"%PORTC0> "
	Ausg_C0
	"</td><td><input type=\"checkbox\" name=\"OUT\" value=\"a\">- / +<input type=\"checkbox\" name=\"OUT\" value=\"b\"> ab %VA@10 &deg;C aus</td><td> -> Sensor 0</td></tr>\r\n"
	
	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"B\"%PORTC1> "
	Ausg_C1
	"</td><td><input type=\"checkbox\" name=\"OUT\" value=\"c\">- / +<input type=\"checkbox\" name=\"OUT\" value=\"d\"> ab %VA@11 &deg;C aus</td><td> -> Sensor 1, A1</td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"C\"%PORTC2> "
	Ausg_C2
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"D\"%PORTC3> "
	Ausg_C3
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"E\"%PORTC4> "
	Ausg_C4
	"</td><td><input type=\"checkbox\" name=\"OUT\" value=\"i\">- / +<input type=\"checkbox\" name=\"OUT\" value=\"j\"> ab %VA@14 &deg;C Differenz ein</td><td>-> Sensor 3,4</td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"F\"%PORTC5> "
	Ausg_C5
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"G\"%PORTC6> "
	Ausg_C6
	"</td><td><input type=\"checkbox\" name=\"OUT\" value=\"m\">- / +<input type=\"checkbox\" name=\"OUT\" value=\"n\"> ab Wert %VA@18 ein </td><td>-> AD-A6</td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"H\"%PORTC7> "
	Ausg_C7
	"</td><td><input type=\"checkbox\" name=\"OUT\" value=\"u\">+h m+<input type=\"checkbox\" name=\"OUT\" value=\"v\"> ein: %T0@ Uhr</td><td><input type=\"checkbox\" name=\"OUT\" value=\"w\">+h m+<input type=\"checkbox\" name=\"OUT\" value=\"x\"> aus: %T1@ Uhr</td></tr>\r\n"	

#else

"<div class=\"fett\">Schalten:</div>\r\n"

	"<table style=\"width: 600px; text-align: left; background-color: rgb(255, 204, 51);\" border=\"0\">\r\n"
    	"<tr><td><form name=\"form1\" method=\"post\" action=\" \">\r\n"
	
	
	"<table style=\"width: 100%; text-align: left;\" border=\"0\">\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"A\"%PORTC0> "
	Ausg_C0
	"</td><td></td><td></td></tr>\r\n"
	
	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"B\"%PORTC1> "
	Ausg_C1
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"C\"%PORTC2> "
	Ausg_C2
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"D\"%PORTC3> "
	Ausg_C3
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"E\"%PORTC4> "
	Ausg_C4
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"F\"%PORTC5> "
	Ausg_C5
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"G\"%PORTC6> "
	Ausg_C6
	"</td><td></td><td></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"H\"%PORTC7> "
	Ausg_C7
	"</td><td></td><td></td></tr>\r\n"


#endif // USE_AUTOMATIK


// => Schalten mit PORTD -> config.h (kein LCD!)
// checkbox-value-Werte K-P (PORTD2-7) -> httpd
#if USE_PORTD_SCHALT

	"<tr><td><hr></td><td><hr></td><td><hr></td></tr>\r\n"

	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"K\"%PORTD2> D2</td><td><input type=\"checkbox\" name=\"OUT\" value=\"L\"%PORTD3> D3</td><td><input type=\"checkbox\" name=\"OUT\" value=\"M\"%PORTD4> D4</td></tr>\r\n"
	"<tr><td><input type=\"checkbox\" name=\"OUT\" value=\"N\"%PORTD5> D5</td><td><input type=\"checkbox\" name=\"OUT\" value=\"O\"%PORTD6> D6</td><td><input type=\"checkbox\" name=\"OUT\" value=\"P\"%PORTD7> D7</td></tr>\r\n"

#endif

// = Servo an PORTD7 -> config.h (kein LCD!)
// checkbox-value-Werte Q+R -> httpd
#if USE_SERVO
	"<tr><td><hr></td><td><hr></td><td><hr></td></tr>\r\n"
	"<tr><td>D7_Servo: </td><td><input type=\"checkbox\" name=\"OUT\" value=\"Q\"> -/+ <input type=\"checkbox\" name=\"OUT\" value=\"R\"> Position %VA@08</td><td>-> Webcam</td></tr>\r\n"
#endif

	
	"</table>\r\n"

	"<hr><p align=\"center\"><input type=\"submit\" name=\"SUB\" value=\"Senden\"></p></form>\r\n"

	"</td></tr></table>\r\n"

// Schaltanweisungen --------------------------------------------------------


// Lokales Wetter
# if WETTER24
	"<br><div class=\"fett\">Lokale Wetterinformationen: </div><a href="
	LOCALWEATHER24
	">-> www.wetter24.de</a>\r\n"
#endif

	"<br><hr>\r\n"

#if USE_SER_LCD
	"<div>PORTB -> ENC28J60<br>PORTD -> LCDisplay</div>\r\n"
#else
//	"<div>PORTB -> ENC28J60</div>\r\n"
//	"<div>Achtung: Kein LCD an PORTC anschließen! Beschädigung möglich.</div>\r\n"
#endif
	
	"<br>\r\n"

	"<div>Webserver von Ulrich Radig " SoftVers "</div>\r\n"
	"<div>Compiliert am "__DATE__" um "__TIME__" mit GCC Version "__VERSION__"</div>\r\n"
	"<div>RoBueVers für AVR-NET-IO</div>\r\n"
	"<div>Modifiziert von Ralf Wilke DH3WR und Moritz Holtz DF5MH Amateurfunkgruppe RWTH Aachen <a href=\"http://db0sda.ampr.org/\" target=\"_blank\">db0sda.ampr.org</a></div>\r\n"
	"</span></body></html>\r\n"
	"%END"};

// RoBue:
// Daten-Seite: 1-Wire-Werte, AD-Werte  
PROGMEM char const Page2[] = {

#if USE_OW
	// 1-Wire Sensoren -> Temperatur
	"%OW@00\r\n"
	"%OW@01\r\n"
	"%OW@02\r\n"
	"%OW@03\r\n"
	"%OW@04\r\n"

// Je nach Bedarf weitere Ausgabewerte einfuegen
//	"%OW@05\r\n"
//	"%OW@06\r\n"
//	"%OW@07\r\n"
#endif // USE_OW

#if USE_SHT
	"%VA@01\r\n"
	"%VA@02\r\n"
	"%VA@03\r\n"
#endif // USE_SHT

	// AD-Werte
	"%VA@04\r\n"
	"%VA@05\r\n"
	"%VA@06\r\n"
	"%END"};

//----------------------------------------------------------------------------
//Das GIF Bild für den Balken
//PROGMEM char bild_balken[] = {
//	0x47,0x49,0x46,0x38,0x39,0x61,0x02,0x00,0x0A,0x00,0xA2,0x00,0x00,0xA2,0xA5,0xED,
//	0x84,0x8F,0xE5,0x66,0x7C,0xDD,0x49,0x60,0xD6,0x21,0x44,0xDA,0xFE,0x01,0x02,0x00,
//	0x00,0x00,0x00,0x00,0x00,0x21,0xF9,0x04,0x05,0x14,0x00,0x05,0x00,0x2C,0x00,0x00,
//	0x00,0x00,0x02,0x00,0x0A,0x00,0x00,0x03,0x0A,0x48,0x34,0x23,0x12,0x01,0x48,0x00,
//	0x1D,0x53,0x09,0x00,0x3B,'%','E','N','D'};
//-----------------------------------------------------------------------------
//Nur Dateiname und Pointer eintragen	
	WEBPAGE_ITEM WEBPAGE_TABLE[] = // Befehls-Tabelle
	{
	{"index.htm",Page1},
	{"daten.html",Page2},
	#if USE_CAM
	{"camera.jpg",bild_balken},
	#endif //USE_CAM
//	{"balke.gif",bild_balken},
//	{"ledon.gif",led_on},
//	{"ledoff.gif",led_off},
	{NULL,NULL} 
	};

#endif //_WEBPAGE_H
