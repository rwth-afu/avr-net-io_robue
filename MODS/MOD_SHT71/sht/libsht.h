/***********************************************************************************
Sensirion SHTxx Sensor Library 0v2.2


***********************************************************************************/
/* Copyright Notice
 	This library for the SHT temperature and humidity sensors is based on the 
	application datasheet "Sample Code humidity sensor SHTxx" from Sensirion. 
	(c) Timo Dittmar
		(For now. If I get enough feedback and a positive answer from
		Sensirion I plan to release the code under GPL)
	Use without any warranty
*/


/* History
	Date, Version, Comment 
	2006-07-14	NA		- initial conversion from Sensirion application note
						- inserted test code
	2006-07-17	NA		- adjusted timing in measurement
						- changed data transfer in calc_sht; 
	2006-07-20	0v1rc	- included heating element code 
	2006-08-05	0v1		- Some additional cleanup
						- homogenized function names;
	2006-08-13	ov2rc		- included functions for measuring with recurring interrupt
						- fewer orthographic errors :-)
						- first implementation of crc check
	2006-08-27	0v2		- second release
	
	2009-01-15	0v2.1	- changed float params to scaled int's
						- removed functions for interupt stuff
	2009-01-16	0V2.2	- move Pin definitions in config.h
	
*/

#ifndef _SHT_LIB_

#define _SHT_LIB_

//----------------------------------------------------------------------------------
// modul-var
//----------------------------------------------------------------------------------


/* SHT SCK and DATA Port and Pin definitions 
#define SHT_SCK_PORT PORTA
#define SHT_SCK_DDR DDRA
#define SHT_SCK_PIN PA5
#define SHT_DATA_PORT PORTA 
#define SHT_DATA_DDR DDRA
#define SHT_DATA_PORT_PIN PINA
#define SHT_DATA_PIN PA6
*/

/* IO Macros */
#define MAKE_SHT_SCK_PIN_OUTPUT SHT_SCK_DDR |= ( 1 << SHT_SCK_PIN)
#define SET_SHT_SCK SHT_SCK_PORT |= ( 1 << SHT_SCK_PIN)
#define CLEAR_SHT_SCK SHT_SCK_PORT &= ~( 1 << SHT_SCK_PIN)
#define SET_SHT_DATA(SHT_DATA_PIN) SHT_DATA_PORT |= ( 1 << SHT_DATA_PIN)
#define CLEAR_SHT_DATA(SHT_DATA_PIN) SHT_DATA_PORT &= ~( 1 << SHT_DATA_PIN)
#define MAKE_SHT_DATA_PIN_OUTPUT(SHT_DATA_PIN) SHT_DATA_DDR |= ( 1 << SHT_DATA_PIN) 
#define MAKE_SHT_DATA_PIN_INPUT(SHT_DATA_PIN) SHT_DATA_DDR &= ~( 1 << SHT_DATA_PIN)
#define SHT_DATA(SHT_DATA_PIN) (SHT_DATA_PORT_PIN & ( 1<< SHT_DATA_PIN))


/* Setting the offset of the bandgap temperature sensor according to the datasheet */
   #define SHT_TEMP_OFFSET -40  	//@ 5V
// #define SHT_TEMP_OFFSET -39.75  //@ 4V
// #define SHT_TEMP_OFFSET -39.66  //@ 3.5V
// #define SHT_TEMP_OFFSET -39.60  //@ 3V
// #define SHT_TEMP_OFFSET -39.55  //@ 2.5V

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* You should not have to change anything below */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef union
{ 
	unsigned int i;	// raw sensor data
	//int p;			// calculated physical, scaled
	float f;		// calculated physical, scaled
} 
	sht_value;

/* emum for switching measurement mode */
enum {TEMP,HUMI}; //

/* SHT bit and values definitions */

#define noACK 0
#define ACK 1
//adr command r/w
#define STATUS_REG_W 0x06 //000 0011 0
#define STATUS_REG_R 0x07 //000 0011 1
#define MEASURE_TEMP 0x03 //000 0001 1
#define MEASURE_HUMI 0x05 //000 0010 1
#define RESET_SHT 0x1e //000 1111 0
#define HEATER_BIT 0x04 // 00000100


//Function definitions
unsigned char sht_write_byte (unsigned char sht_value, unsigned char pin);
unsigned char sht_read_byte (unsigned char ack, unsigned char pin);
void sht_transstart (unsigned char pin);
void sht_connectionreset (unsigned char pin);
unsigned char sht_softreset (unsigned char pin);
unsigned char sht_read_statusreg(unsigned char *p_value, unsigned char *p_checksum, unsigned char pin);
unsigned char sht_write_statusreg(unsigned char *p_value, unsigned char pin);
unsigned char sht_measure(sht_value *p_sht_value, unsigned char *p_checksum, unsigned char mode, unsigned char pin);
void sht_raw_to_physical(sht_value *p_humidity ,sht_value *p_temperature);
float calc_dewpoint(float humi,float temp);
void sht_switch_heating_element(unsigned char onoff, unsigned char pin);
void delay_ms(unsigned short ms);
unsigned char sht_check_crc(sht_value *p_sht_value, unsigned char *p_checksum, unsigned char mode, unsigned char statusreg);

#endif // _SHT_LIB_
