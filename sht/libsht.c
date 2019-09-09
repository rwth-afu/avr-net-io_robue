
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
	2006-07-14	NA		- Initial conversion from Sensirion application note
						- inserted test code
	2006-07-17	NA		- Adjusted timing in measurement
						- changed data transfer in calc_sht; 
	2006-07-20	0v1rc	- Included heating element code 
	2006-08-05	0v1		- Some additional cleanup
						- homogenized function names;
	2006-08-13	0v2rc	- included functions for measuring with recurring interrupt
						- fewer orthographic errors :-)
						- first implementation of crc check
	2006-08-27	0v2		- second release
	2009-02-05	0V2.2	- include delay_ms function (cni)
*/

#include <avr/io.h> //Microcontroller specific library, e.g. port definitions

#include "config.h"

#include <math.h> //
#include <stdlib.h> //
#include <inttypes.h> // 

#include <util/delay.h>
#include "sht/libsht.h"
#include <avr/pgmspace.h>

#include "usart.h"


//----------------------------------------------------------------------------------
unsigned char sht_write_byte(unsigned char sht_value, unsigned char pin)
//----------------------------------------------------------------------------------
// writes a byte on the Sensibus and checks the acknowledge
{
	unsigned char i;
	unsigned char error=0;
	
	MAKE_SHT_DATA_PIN_OUTPUT(pin);
	asm volatile ("nop"::);
	
	for (i=0x80;i>0;i/=2) 				//shift bit for masking
	{
		if (i & sht_value) 
		{	
			SET_SHT_DATA(pin);			//masking value with i , write to SENSI-BUS
		}
	 	else 
		{
			CLEAR_SHT_DATA(pin);
		}
		SET_SHT_SCK; //clk for SENSI-BUS
		asm volatile ("nop"::);
		asm volatile ("nop"::);
		CLEAR_SHT_SCK;
		asm volatile ("nop"::);
		asm volatile ("nop"::);
	}
	
	SET_SHT_DATA(pin); //release DATA-line
	MAKE_SHT_DATA_PIN_INPUT(pin);
	asm volatile ("nop"::);
	SET_SHT_SCK; 					//clk #9 for ACK
	asm volatile ("nop"::);
	asm volatile ("nop"::);

	if (SHT_DATA(pin)) 
	{
		error =1; //check ack (DATA will be pulled down by SHT11)
	}
	CLEAR_SHT_SCK; // move to pre-if
	return error; //error=1 in case of no acknowledge
}


//----------------------------------------------------------------------------------
unsigned char sht_read_byte(unsigned char ack, unsigned char pin)
//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1"
{
	unsigned char i;
	unsigned char val=0;
	MAKE_SHT_DATA_PIN_OUTPUT(pin);
	asm volatile ("nop"::);
	SET_SHT_DATA(pin); //release DATA-line
	MAKE_SHT_DATA_PIN_INPUT(pin);
	asm volatile ("nop"::);
	
	for (i=0x80;i>0;i/=2) //shift bit for masking
	{
		SET_SHT_SCK; //clk for SENSI-BUS
		asm volatile ("nop"::);
		asm volatile ("nop"::);
		if (SHT_DATA(pin)) val=(val | i); //read bit
		CLEAR_SHT_SCK;
		asm volatile ("nop"::);
		asm volatile ("nop"::);
	}
	
	MAKE_SHT_DATA_PIN_OUTPUT(pin);
	asm volatile ("nop"::);
	if (ack) CLEAR_SHT_DATA(pin); 
	else SET_SHT_DATA(pin); // Sent ack
	
	SET_SHT_SCK; //clk #9 for ack
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	
	CLEAR_SHT_SCK;
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	SET_SHT_DATA(pin); //release DATA-line
	return val;
}



//----------------------------------------------------------------------------------
void sht_transstart(unsigned char pin)
//----------------------------------------------------------------------------------
// generates a transmission start
{
	MAKE_SHT_DATA_PIN_OUTPUT(pin);
	asm volatile ("nop"::);
	SET_SHT_DATA(pin); CLEAR_SHT_SCK; //Initial state
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	SET_SHT_SCK;
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	CLEAR_SHT_DATA(pin);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	CLEAR_SHT_SCK;
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	SET_SHT_SCK;
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	SET_SHT_DATA(pin);
	asm volatile ("nop"::);
	asm volatile ("nop"::);
	CLEAR_SHT_SCK;
}


//----------------------------------------------------------------------------------
void sht_connectionreset(unsigned char pin)
//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
{
	unsigned char i;
	MAKE_SHT_DATA_PIN_OUTPUT(pin);
	asm volatile ("nop"::);
	
	SET_SHT_DATA(pin); CLEAR_SHT_SCK; //Initial state
	
	for(i=0;i<9;i++) //9 SCK cycles
	{
		SET_SHT_SCK;
		asm volatile ("nop"::);
		asm volatile ("nop"::);
		
		CLEAR_SHT_SCK;
		asm volatile ("nop"::);
		asm volatile ("nop"::);
	}
	sht_transstart(pin); //transmission start
}



//----------------------------------------------------------------------------------
unsigned char sht_softreset(unsigned char pin)
//----------------------------------------------------------------------------------
// resets the sensor by a softreset
{
	// rockclimber
	MAKE_SHT_SCK_PIN_OUTPUT;
	asm volatile ("nop"::);

	unsigned char error=0;
	sht_connectionreset(pin); //reset communication
	error+=sht_write_byte(RESET_SHT, pin); //send RESET_SHT-command to sensor
	return error; //error=1 in case of no response form the sensor
}



//----------------------------------------------------------------------------------
unsigned char sht_read_statusreg(unsigned char *p_sht_value, unsigned char *p_checksum, unsigned char pin)
//----------------------------------------------------------------------------------
// reads the status register with checksum (8-bit)
{
	unsigned char error=0;
	
	sht_transstart(pin); //transmission start
	error=sht_write_byte(STATUS_REG_R, pin); //send command to sensor
	*p_sht_value=sht_read_byte(ACK, pin); //read status register (8-bit)
	*p_checksum=sht_read_byte(noACK, pin); //read checksum (8-bit)
	return error; //error=1 in case of no response form the sensor
}



//----------------------------------------------------------------------------------
unsigned char sht_write_statusreg(unsigned char *p_sht_value, unsigned char pin)
//----------------------------------------------------------------------------------
// writes the status register with checksum (8-bit)
{
	unsigned char error=0;
	sht_transstart(pin); //transmission start
	error+=sht_write_byte(STATUS_REG_W, pin);//send command to sensor
	error+=sht_write_byte(*p_sht_value, pin); //send value of status register
	return error; //error>=1 in case of no response form the sensor
}



//----------------------------------------------------------------------------------
unsigned char sht_measure(sht_value *p_sht_value, unsigned char *p_checksum, unsigned char mode, unsigned char pin)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{
	unsigned char error=0;
	unsigned char i;

	sht_value this_sht_value;
	/* sht_transstart(pin); //transmission start */
	sht_connectionreset(pin);
	switch(mode)
	{ //send command to sensor
		case TEMP : error+=sht_write_byte(MEASURE_TEMP, pin); /*usart_write("TEMP: ");*/ break;
		case HUMI : error+=sht_write_byte(MEASURE_HUMI, pin); /*usart_write("HUMI: ");*/ break;
		default : break;
	}

	for (i=0;i<500;i++) {
		if(SHT_DATA(pin)==0) break; //wait until sensor has finished the measurement
		_delay_ms(1);  	// neccesary since the AVR is so much faster than the old 8051
							// and the 210ms for 14bit measuremnt are only a rough estimate
	}

	if(SHT_DATA(pin)) error+=1; // or timeout is reached 

	this_sht_value.i = 256*sht_read_byte(ACK, pin); //read the first byte (MSB)
	this_sht_value.i +=sht_read_byte(ACK, pin); //read the second byte (LSB)
		
	*p_checksum =sht_read_byte(noACK, pin); //read checksum
	*(p_sht_value)= this_sht_value;
	return error;
}




//----------------------------------------------------------------------------------------
void sht_raw_to_physical(sht_value *p_humidity ,sht_value *p_temperature)
//----------------------------------------------------------------------------------------
// calculates temperature [C] and humidity [%RH]
// input : humi [Ticks] (12 bit)
// temp [Ticks] (14 bit)
// output: humi [%RH]
// temp [C]
{ 

	// Const's for 12/14 bit
	const float C1=-4.0; // for 12 Bit
	const float C2= 0.0405; // for 12 Bit
	const float C3=-0.0000028; // for 12 Bit
	const float T1=0.01; // for 14 Bit @ 5V
	const float T2=0.00008; // for 14 Bit @ 5V
	const float RES_TEMP = 0.01; // for 14 Bit @ 5V

	
	float rh_lin; // rh_lin: Humidity linear
	float rh_true; // rh_true: Temperature compensated humidity
	float t_C; // t_C : Temperature [C]

	t_C = RES_TEMP*(*p_temperature).i +(SHT_TEMP_OFFSET); //calc. Temperature from ticks to [C]
	
	rh_lin=C3*(*p_humidity).i*(*p_humidity).i + C2*(*p_humidity).i + C1; //calc. Humidity from ticks to [%RH]
	rh_true=(t_C-25)*(T1+T2*(*p_humidity).i)+rh_lin; //calc. Temperature compensated humidity [%RH]
	
	if(rh_true>100)rh_true=100; //cut if the value is outside of
	if(rh_true<0.1)rh_true=0.1; //the physical possible range

	(*p_temperature).f=t_C;  //return temperature [C]
	(*p_humidity).f=rh_true; //return humidity[%RH]
}


//--------------------------------------------------------------------
float calc_dewpoint(float h,float t)
//--------------------------------------------------------------------
// calculates dew point
// input: humidity [%RH], temperature [C]
// output: dew point [C]
{ float k,dew_point ;
k = (log10(h)-2)/0.4343 + (17.62*t)/(243.12+t);
dew_point = 243.12*k/(17.62-k);
return dew_point;
}

//--------------------------------------------------------------------
void sht_switch_heating_element(unsigned char onoff, unsigned char pin)
//--------------------------------------------------------------------
// shwitches the internal heating element
// input onoff 0:off 1:on 
{
	unsigned char status;
	unsigned char checksum;
	sht_read_statusreg(&status, &checksum, pin);
	if (onoff==0) status &= ~(HEATER_BIT);
	else status |= (HEATER_BIT);
	sht_write_statusreg(&status, pin);
}

void delay_ms(unsigned short ms)
/* delay for a minimum of <ms> */
/* with a 4Mhz crystal, the resolution is 1 ms */
{
   unsigned short outer1, outer2;
      outer1 = 200;

      while (outer1) {
      outer2 = 1000;
      while (outer2) {
         while ( ms ) ms--;
         outer2--;
      }
      outer1--;
   }
}
