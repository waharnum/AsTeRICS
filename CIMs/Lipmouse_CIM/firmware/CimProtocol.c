

/*
     AsTeRICS LipMouse CIM Firmware
	 using Teensy 2.0++ Controller board


	 file: CimProtocol.c
	 Version: 0.1
	 Author: Chris Veigl (FHTW), changes added by Alberto Ib��ez (07/01/2014)
	 Date: 04/09/2010



   AsTeRICS CIM Protocol Packet Frame:
   ===================================

 	Packet ID				2 bytes	"@T" (0x4054 )
 	ARE ID / CIM ID			2 bytes	
	Data Size				2 bytes	0x0000-0x0800
	Packet serial number	1 byte	0x00-0x7f (0x80-0xff for event-replies from CIM )
	CIM-Feature address		2 bytes	
	Request / Reply code	2 bytes	
	-------------------------------------------> 11 bytes = minimum frame length
	Optional data			0-2048 bytes	
	Optional CRC checksum	0 or 4 bytes CRC32
	-------------------------------------------> 2063 bytes = maximum frame length  

*/


#include "CimProtocol.h"
#include "Timer.h"
#include "usb_serial.h"
#include "Adc.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
//#include <string.h>
//#include "Leds.h"


#define ARE_MINIMAL_VERSION 1

//const uint32_t LIPMOUSE_CIM_UNIQUE_NUMBER = 0x12345678;  
const uint32_t LIPMOUSE_CIM_UNIQUE_NUMBER = 7;  

const char LIPMOUSE_CIM_FEATURELIST[]=
{
   0x00,0x00,  // unique number, data: 4 bytes 
   0x01,0x00,  // Activate periodic ADC Reports, data: 2 bytes: period in milliseconds (0=off)
   0x02,0x00,  // ADC value report, data: 10 bytes (chn low / high byte for chn1-5)

} ;



struct ARE_frame_t ARE_frame;
struct CIM_frame_t CIM_frame;

uint8_t buttonval;

unsigned char readstate=0;
unsigned int  datapos=0;
uint8_t first_packet=1;



//extern unsigned char old_PIND;
//extern unsigned char old_PINB;

//unsigned char PIND_Mask =0;
//unsigned char PINB_Mask =0;

void init_CIM_frame (void)
{
	CIM_frame.packet_id=CIM_FRAME_START;  // '@','T': Packet-ID/sync bytes
	CIM_frame.cim_id=CIM_ID_LIPMOUSE;    
}


uint8_t process_ARE_frame(uint8_t status_code)
{
    uint8_t ack_needed;
	uint8_t data_size=0;
	uint8_t command;


//	LEDs_ToggleLEDs(LED0);  // indicate correct frame

	command=(uint8_t)ARE_frame.request_code;
	CIM_frame.cim_feature=ARE_frame.cim_feature;
	CIM_frame.serial_number=ARE_frame.serial_number;
	CIM_frame.reply_code=(((uint16_t)status_code)<<8) + command;
	data_size=(uint8_t)ARE_frame.data_size;

	ack_needed=1;


//	if ((status_code & (CIM_ERROR_INVALID_ARE_VERSION | CIM_ERROR_CRC_MISMATCH)) == 0)
	if ((status_code & CIM_ERROR_INVALID_ARE_VERSION) == 0)
	{
	      // UART_Print(" feature ");  UART_Putchar(command);
		  // no serious packet error 
	      switch (command)   {  // process requested command

		  	case CMD_REQUEST_FEATURELIST:
			       if (data_size==0) {
				     reply_FeatureList();  // reply requested feature list
					 ack_needed=0;
					} else status_code |= CIM_ERROR_INVALID_FEATURE;
				  	break;

		  	case CMD_REQUEST_RESET_CIM:
					if (data_size!=0) status_code |= CIM_ERROR_INVALID_FEATURE;
					break;
		  	case CMD_REQUEST_START_CIM:
			        if (data_size==0) {
					  enable_timer_ISR();
					} else status_code |= CIM_ERROR_INVALID_FEATURE;
					break;
		  	case CMD_REQUEST_STOP_CIM:
			       if (data_size==0) {
			   		// LEDs_ToggleLEDs(LED4);  // indicate reset CIM
				     first_packet=1;  // reset first frame indicator etc.
					 disable_timer_ISR();
					} else status_code |= CIM_ERROR_INVALID_FEATURE;
				  break;

			case CMD_REQUEST_READ_FEATURE:  //  read feature from CIM

			  switch (ARE_frame.cim_feature) {
			     case LIPMOUSE_CIM_FEATURE_UNIQUENUMBER:   // read unique serial number
  		            if (data_size==0) {    
				     reply_UniqueNumber();
					 ack_needed=0;
					} else status_code |= CIM_ERROR_INVALID_FEATURE;
  			     	break;

			     case LIPMOUSE_CIM_FEATURE_ADCREPORT:
						generate_ADCFrame();
						reply_DataFrame();
					    ack_needed=0;
				     break;

			     case LIPMOUSE_CIM_FEATURE_BUTTONREPORT:
						generate_ButtonFrame();
						reply_DataFrame();
					    ack_needed=0;
				     break;


  			      default: 				// not a valid read  feature;		 
					status_code |= CIM_ERROR_INVALID_FEATURE;
					
			   }
			   break;
				   

			case CMD_REQUEST_WRITE_FEATURE:  //  write feature to CIM						

				switch (ARE_frame.cim_feature) {  // which feature address ?

			        case LIPMOUSE_CIM_FEATURE_SET_ADCPERIOD:
	  		            if (data_size==2) {    
							cli();
							ADC_updatetime=  (uint16_t)ARE_frame.data[0];
							ADC_updatetime+= ((uint16_t)ARE_frame.data[1])<<8;
							sei();
						}
				     break;
			        case LIPMOUSE_CIM_FEATURE_SET_LEDS:
	  		            if (data_size==1) {
							uint8_t actLeds=ARE_frame.data[0];    
							if (actLeds&1) PORTB&=~(1<<0); else PORTB|=(1<<0);
							if (actLeds&2) PORTE&=~(1<<7); else PORTE|=(1<<7);
							if (actLeds&4) PORTE&=~(1<<6); else PORTE|=(1<<6);
						}
				     break;
					default:         // not a valid write  feature;
		   				status_code |= CIM_ERROR_INVALID_FEATURE;
				}

			}
    }

	if (status_code & CIM_ERROR_INVALID_FEATURE)   {  // invalid data size or feature
	//	LEDs_ToggleLEDs(LED5);  // indicate wrong feature
	//	UART_Print(" invalid data size or no feature ");
	}
 

	if (ack_needed) {
	    reply_Acknowledge();
	}

    return(1);
}



void reply_FeatureList(void)
{
  
    if (UEINTX & (1<<RWAL)) { //wenn Buffer nicht voll ist 
	     CIM_frame.reply_code |= (CIM_ERROR_CIM_NOT_READY<<8) ;
//		 LEDs_TurnOnLEDs(LED6);  // indicate frame loss
    }
	if (!(UEINTX & (1<<RWAL))) { //wenn Buffer voll ist
		CIM_frame.data_size=sizeof(LIPMOUSE_CIM_FEATURELIST);     // feature list length
	    usb_serial_write ( (uint8_t *)&CIM_frame, CIM_HEADER_LEN);
    	usb_serial_write ( (uint8_t *)&LIPMOUSE_CIM_FEATURELIST, CIM_frame.data_size);
	} 
}

void reply_UniqueNumber(void)
{
		CIM_frame.data_size=4;    // lenght of unique number
    	usb_serial_write ((uint8_t *) &CIM_frame, CIM_HEADER_LEN);
    	usb_serial_write ((uint8_t *) &LIPMOUSE_CIM_UNIQUE_NUMBER, CIM_frame.data_size);

/*	if (UEINTX & (1<<RWAL)) { //wenn Buffer nicht voll ist
	     CIM_frame.reply_code |= (CIM_ERROR_CIM_NOT_READY<<8);
		// LEDs_TurnOnLEDs(LED6);  // indicate frame loss
    }
	if (!(UEINTX & (1<<RWAL))) { //wenn Buffer voll ist
		CIM_frame.data_size=4;    // lenght of unique number
    	usb_serial_write ((char *) &CIM_frame, CIM_HEADER_LEN);
    	usb_serial_write ((char *) &TEENSY_CIM_UNIQUE_NUMBER, CIM_frame.data_size);
	} */
}

void reply_Acknowledge(void)
{
	if ((UEINTX & (1<<RWAL))) { //wenn Buffer nicht voll ist 
	     CIM_frame.reply_code |= (CIM_ERROR_CIM_NOT_READY<<8) ;
    }

	if (!(UEINTX & (1<<RWAL))) {	//wenn Buffer voll ist
		CIM_frame.data_size=0;     // no data in ack frame
    	usb_serial_write ((uint8_t *) &CIM_frame, CIM_HEADER_LEN);
	} 
}

void reply_DataFrame(void)
{
	
	usb_serial_write ((uint8_t *) &CIM_frame, CIM_HEADER_LEN);
	usb_serial_write ((uint8_t *) CIM_frame.data, CIM_frame.data_size);
	
	/*if ((UEINTX & (1<<RWAL)) <  CIM_HEADER_LEN+CIM_frame.data_size) 
	{
	     CIM_frame.reply_code |= (CIM_ERROR_CIM_NOT_READY<<8) ;
	//	 LEDs_TurnOnLEDs(LED6);  // indicate frame loss
    }
    else
	{
	   	usb_serial_write ((char *) &CIM_frame, CIM_HEADER_LEN);
	   	usb_serial_write ((char *) CIM_frame.data, CIM_frame.data_size);
	}*/	    
}


#define LIPSY_V1_0


#ifdef LIPSY_BT
  #define AD_PRESSURE 0
  #define AD_UP 2 
  #define AD_DOWN 1  
  #define AD_LEFT  3 
  #define AD_RIGHT 4 
#endif

#ifdef LIPSY_V0
  #define AD_PRESSURE 0
  #define AD_UP 6 
  #define AD_DOWN 4 
  #define AD_LEFT  5 
  #define AD_RIGHT 7 
#endif

#ifdef LIPSY_V1_0
  #define AD_PRESSURE 0
  #define AD_UP 6 
  #define AD_DOWN 7 
  #define AD_LEFT  5 
  #define AD_RIGHT 4 
#endif


void generate_ADCFrame()
{
	uint16_t adcval;
	
	adcval=ADC_Read(AD_RIGHT); //right
	adcval=adcval-ADC_Read(AD_LEFT); //left -> value = right - left
	CIM_frame.data[0]=adcval&0xff;
	CIM_frame.data[1]=adcval>>8;
	
	adcval=ADC_Read(AD_DOWN); //down
	adcval=adcval-ADC_Read(AD_UP); //up -> value = down-up
	CIM_frame.data[2]=adcval&0xff;
	CIM_frame.data[3]=adcval>>8;
	
	adcval=ADC_Read(AD_PRESSURE); //pressure
	CIM_frame.data[4]=adcval&0xff;
	CIM_frame.data[5]=adcval>>8;
	CIM_frame.data_size=6; 
}



uint8_t update_Buttonval()
{
	uint8_t actval=0;

	if (!(PINC & (1<<3))) actval|=1; 
	if (!(PIND & (1<<2))) actval|=2; 
	if (!(PIND & (1<<3))) actval|=4; 

    if (actval != buttonval)
	{
	   buttonval=actval;
	   return(1);
    }
	else return (0);
}

void generate_ButtonFrame()
{
    update_Buttonval();

	CIM_frame.data[0]=buttonval;
	CIM_frame.data_size=1; 
}

#define FRAME_DONE 99


void parse_CIM_protocol(void)
{
    uint32_t checksum=0;
    static uint8_t transmission_mode;
	static uint8_t reply_status_code;
	static uint8_t last_serial;
	uint8_t actbyte;

	

    while (usb_serial_available()) 
	{ 
      actbyte=usb_serial_getchar();
		
      switch (readstate) 
	  {
		  case 0: // first sync byte
		  		  reply_status_code=0;
		  		  if (actbyte=='@') readstate++; 
				  break;
		  case 1: // second sync byte
		  		  if (actbyte=='T')  readstate++;
				  else readstate=0;
				  break;

 		  // packet in sync !

		  case 2: // ARE-ID: SW-version low byte
				  ARE_frame.are_id=actbyte;  
		  		  readstate++; 
				  break;
		  case 3: // ARE-ID: SW-version high byte
		  		  ARE_frame.are_id+=((uint16_t)actbyte)<<8;  
				  if (ARE_frame.are_id < ARE_MINIMAL_VERSION)    // outdated ARE ?
				     reply_status_code |= CIM_ERROR_INVALID_ARE_VERSION;
		  		  readstate++; 
				  break;
		  case 4: // data length low byte
		  		  ARE_frame.data_size=actbyte;
				  readstate++; 
				  break;
		  case 5: // data length high byte
		  		  ARE_frame.data_size+=((uint16_t)actbyte)<<8;
				  if (ARE_frame.data_size > DATABUF_SIZE)  { // dismiss packets of excessive length
				       readstate=0;
					   //ARE_frame.data_size=0;
  				       //reply_status_code |= CIM_ERROR_INVALID_FEATURE;
				  }
				  else readstate++; 
				 
				  break;
	      case 6: // serial_number 
		  		  ARE_frame.serial_number = actbyte;
				  if (first_packet)    // don't check first serial
				      first_packet=0;
				  else if (actbyte != (last_serial+1)%0x80) // check current serial number
				     reply_status_code |= CIM_ERROR_LOST_PACKETS;

				  last_serial=actbyte;
			      readstate++;
				  break;
		  case 7: // CIM-feature low byte
		  		  ARE_frame.cim_feature= actbyte; 
		  		  readstate++; 
				  
				  break;
		  case 8: // CIM-feature high byte
		  		  ARE_frame.cim_feature+=((int)actbyte)<<8; 
		  		  readstate++; 

				  break;
		  case 9: // Request code low byte ( command )
		  		  ARE_frame.request_code=actbyte;
		  		  readstate++; 
				  break;
		  case 10:// Request code high byte (transmission mode)
		  		  transmission_mode=actbyte;  // bit 0: CRC enable
				  // reply_status_code|=(actbyte & CIM_STATUS_CRC);   
				  // remember CRC state for reply
				  
		  		  if (ARE_frame.data_size>0) {
				  	readstate++;
					datapos=0;
				  }
				  else {  // no data in packet 
				    if (transmission_mode & CIM_STATUS_CRC) 
					   readstate+=2;      // proceed with CRC 
					else readstate=FRAME_DONE;  // frame is finished here !
				  }
				  break;

		  case 11: // read out data
		  		  ARE_frame.data[datapos]=actbyte;
				  datapos++;
				  if (datapos==ARE_frame.data_size)
				  {
				     if (transmission_mode & CIM_STATUS_CRC)  // with CRC: get checksum 
 				        readstate++;  					 
					 else  readstate=FRAME_DONE; // no CRC:  frame is finished here !
				  } 
				  break;
		  case 12: // checksum byte 1
		  		  checksum=actbyte;
				  readstate++;
				  break;
		  case 13: // checksum byte 2
		  		  checksum+=((long)actbyte)<<8;
				  readstate++;
				  break;
		  case 14: // checksum byte 3
		  		  checksum+=((long)actbyte)<<16;
				  readstate++;
				  break;
		  case 15: // checksum byte 4
		  		  checksum+=((long)actbyte)<<24;	  

				  // check CRC now (currently not used):
				  // if (checksum != crc32(ARE_frame.data, ARE_frame.data_size))
				  reply_status_code |= CIM_ERROR_CRC_MISMATCH;

				  // frame finished here !  
 				  readstate=FRAME_DONE;
				  break;
		  default: readstate=0; break;
	 }		

     if (readstate==FRAME_DONE)  {  // frame finished: store command in ringbuffer
			process_ARE_frame(reply_status_code);
			readstate=0;
		}
   }
}

