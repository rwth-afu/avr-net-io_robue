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

#ifndef _HTTPD_H
  #define _HTTPD_H
  
  #include <avr/io.h>
  #include <avr/pgmspace.h>
  #include "stack.h"
  #include "base64.h"
  #include "analog.h"
  #include "wol.h"

  #define HTTPD_PORT            80
  
  #define CONVERSION_BUFFER_LEN 10
  
  //#define HTTP_DEBUG  usart_write 
  #define HTTP_DEBUG(...)  
    
  #define MAX_VAR_ARRAY 10
  unsigned int var_array[MAX_VAR_ARRAY];
  char dstr[24];
  
  typedef struct
  {
    const char *filename;        //Dateiname der Seite
    PGM_P      page_pointer;      //Zeiger auf Speicherinhalt
  } WEBPAGE_ITEM;
  
  struct http_table
  {
    unsigned char http_auth         : 1;
    unsigned char http_header_type  : 1;
    unsigned char first_switch      : 1;
    unsigned char post              : 1;
    unsigned char hdr_end           : 1;
    #if USE_CAM
    unsigned char cam               : 2;
    #endif //USE_CAM
    PGM_P         old_page_pointer;
    PGM_P         new_page_pointer;
  };
  
  //Prototypen
  void httpd (unsigned char);
  void httpd_init (void);
  void httpd_stack_clear (unsigned char);
  void httpd_header_check (unsigned char);
  void httpd_data_send (unsigned char);
 
#endif //_HTTPD_H




