//------------------------------------------------------------------------------
//                                  _            _     
//                                 | |          | |    
//      ___ _ __ ___  ___ _   _ ___| |_ ___  ___| |__  
//     / _ \ '_ ` _ \/ __| | | / __| __/ _ \/ __| '_ \. 
//    |  __/ | | | | \__ \ |_| \__ \ ||  __/ (__| | | |
//     \___|_| |_| |_|___/\__, |___/\__\___|\___|_| |_|
//                         __/ |                       
//                        |___/    Engineering
//
// Filename:    main.c
// Description: LED-Paint
//				
//
// Copyright (c) Martin Steppuhn (www.emsystech.de)
//
// Nur für den privaten Gebrauch / NON-COMMERCIAL USE ONLY
//
// Die Nutzung (auch auszugsweise) ist für den privaten und nichtkommerziellen 
// Gebrauch erlaubt. Eine Veröffentlichung und Weiterverwendung des Quellcodes 
// ist möglich wenn diese Nutzungsbedingungen incl. Copyright beiliegen
// und die Quelle verlinkt ist. (www.emsystech.de)
//
// Bei kommerziellen Absichten nehmen Sie bitte Kontakt mit uns auf 
// (info@emsystech.de)
//
// Keine Gewähr auf Fehlerfreiheit, Vollständigkeit oder Funktion. Benutzung 
// auf eigene Gefahr. Es wird keinerlei Haftung für direkte oder indirekte 
// Personen- oder Sachschäden übernommen.
//              
// Author:      Martin Steppuhn
// History:     18.12.2009 Initial version
//
// Editor:		Lars Weimar
// History:		06.10.2016 
//				- Update to Atmel Studio 7
//				- Delete uart routine
// History:		07.10.2016 ver.0.0.1
//				- Moved Word lines to get Alarm1 and Alarm2 to bottom
//				- Added Alarm1 and Alarm2 Vars and Words
//				- Added routines to set alarm times
// History:		08.10.2016 ver.0.0.2
//				- Added ADC to get 2 more inputs on ADC6, ADC7 
//				- Added UP/DOWN buttons
// History:		09.10.2016 ver.0.0.3
//				- Added EEPROM to save alarm times
// History:		09.10.2016 ver.0.0.4
//				- Changed to 24h clock
//				- Bugfix alarm
// History:		03.11.2016 ver.0.0.5
//				- Changed pins to get the UART ports free
// History:		03.11.2016 ver.0.0.6
//				- add UART for NTP
// History:		06.11.2016 ver.0.0.7
//				- add NTP sync
//				- Bugfix time & alarm setting
//
//------------------------------------------------------------------------------

/**** Includes ****************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "std_c.h"
#include <avr/eeprom.h>

/**** Preprocessing directives (#define) **************************************/

//=== CPU ===
#define F_CPU       8000000UL

//=== Portpins ===

#define		DATA_SET		PORTB |=  (1<<PB4)
#define		DATA_CLR		PORTB &= ~(1<<PB4)

#define		CLK_SET		PORTB |=  (1<<PB3)
#define		CLK_CLR		PORTB &= ~(1<<PB3)

#define		STROBE_SET	PORTB |=  (1<<PB2)
#define		STROBE_CLR	PORTB &= ~(1<<PB2)

#define		BUTTON			(!(PIND & (1<<PD2)))
#define     UP				(!(PINC & (1<<PC4)))
#define     DOWN			(!(PINC & (1<<PC5)))

#define		BUZZER_TOGGLE	PORTD ^= (1<<PD3)
#define		BUZZER_OFF		PORTD &= ~(1<<PD3)
#define		BUZZER_ON		PORTD |= (1<<PD3)


//=== UART ===
#define BAUD        9600UL
#define UBRR_BAUD   ((F_CPU/(16UL*BAUD))-1)


//=== Pixels ===

#define		CLEAR_ALL			led[0]=0; led[1]=0; led[2]=0; led[3]=0;	led[4]=0; led[5]=0; led[6]=0; led[7]=0; led[8]=0;	led[9]=0; led[10]=0; led[11]=0

// Zeile 1
#define		WORD_ES_IST			led[0]|=1; led[1]|=1; led[4]|=1; led[5]|=1; led[6]|=1
#define		WORD_ZEHN			led[8]|=1; led[9]|=1; led[10]|=1; led[11]|=1
// Zeile 2
#define		WORD_FUENF			led[1]|=0x02; led[2]|=0x02; led[3]|=0x02; led[4]|=0x02
#define		WORD_VOR_1			led[6]|=0x02; led[7]|=0x02; led[8]|=0x02
// Zeile 3
#define		WORD_VIERTEL		led[5]|=0x04; led[6]|=0x04; led[7]|=0x04; led[8]|=0x04; led[9]|=0x04; led[10]|=0x04; led[11]|=0x04
// Zeile 4
#define		WORD_NACH			led[0]|=0x08; led[1]|=0x08; led[2]|=0x08; led[3]|=0x08
#define		WORD_VOR_2			led[4]|=0x08; led[5]|=0x08; led[6]|=0x08
#define		WORD_HALB			led[8]|=0x08; led[9]|=0x08; led[10]|=0x08; led[11]|=0x08
// Zeile 5
#define		WORD_5				led[0]|=0x10; led[1]|=0x10; led[2]|=0x10; led[3]|=0x10
#define		WORD_2				led[6]|=0x10; led[7]|=0x10; led[8]|=0x10; led[9]|=0x10
#define		WORD_1				led[8]|=0x10; led[9]|=0x10; led[10]|=0x10; led[11]|=0x10
// Zeile 6
#define		WORD_7				led[0]|=0x20; led[1]|=0x20; led[2]|=0x20; led[3]|=0x20; led[4]|=0x20; led[5]|=0x20
#define		WORD_6				led[7]|=0x20; led[8]|=0x20; led[9]|=0x20; led[10]|=0x20; led[11]|=0x20
// Zeile 7
#define		WORD_10				led[0]|=0x40; led[1]|=0x40; led[2]|=0x40; led[3]|=0x40
#define		WORD_9 				led[3]|=0x40; led[4]|=0x40; led[5]|=0x40; led[6]|=0x40
#define		WORD_4				led[8]|=0x40; led[9]|=0x40; led[10]|=0x40; led[11]|=0x40
// Zeile 8
#define		WORD_3				led[0]|=0x80; led[1]|=0x80; led[2]|=0x80; led[3]|=0x80
#define		WORD_11				led[4]|=0x80; led[5]|=0x80; led[6]|=0x80
#define		WORD_8				led[8]|=0x80; led[9]|=0x80; led[10]|=0x80; led[11]|=0x80
// Zeile 9
#define		WORD_12				led[1]|=0x100; led[2]|=0x100; led[3]|=0x100; led[4]|=0x100; led[5]|=0x100
#define		WORD_UHR			led[9]|=0x100; led[10]|=0x100; led[11]|=0x100
// Zeile 10
#define     WORD_ALARM1			led[1]|=0x200; led[2]|=0x200; led[3]|=0x200; led[4]|=0x200; led[5]|=0x200; led[6]|=0x200
#define     WORD_ALARM2			led[1]|=0x200; led[2]|=0x200; led[3]|=0x200; led[4]|=0x200; led[5]|=0x200; led[7]|=0x200
#define		WORD_NTP			led[9]|=0x200; led[10]|=0x200; led[11]|=0x200


/**** Type definitions (typedef) **********************************************/

/**** Global constants ********************************************************/

/**** Global variables ********************************************************/

/**** Local constants  ********************************************************/

const uint8  font[40] = 				// Digits
{
	0xFE,0x82,0x82,0xFE,		// 0
	0x08,0x04,0xFE,0x00,		// 1
	0xC4,0xA2,0x92,0x8C,		// 2
	0x82,0x92,0x92,0x7C,		// 3		
	0x1E,0x10,0xF8,0x10,		// 4
	0x9E,0x92,0x92,0x62,		// 5
	0x78,0x94,0x92,0x60,		// 6
	0x02,0xF2,0x0A,0x06,		// 7
	0xFE,0x92,0x92,0xFE, 		// 8
	0x9E,0x92,0x92,0xFE 		// 9
};

/**** Local variables *********************************************************/

uint8  			c;
uint16			led[12];
uint8			col_cnt;
uint16			col;
uint8 			button_mem,up_mem,down_mem,up_mem_al,down_mem_al;
uint8			hour,minute,second,ntp_hour,ntp_minute,ntp_sync,ntp_received;
uint8           alarm1_hour,alarm1_minute;
uint8			alarm2_hour,alarm2_minute;
uint8			alarm;
uint16			beep;
uint8			alarm1_enabled,alarm2_enabled;
uint8			time_setup;
uint8			time_setup_cnt0;
uint8			time_setup_cnt1;
uint8			key_cnt;
uint16			loop_cnt;
uint8			sec_flag,key_flag;
uint8			eeprom_written;
// define eeprom bytes
uint8_t			eeAlarm1_hour EEMEM = 0;
uint8_t			eeAlarm1_minute EEMEM = 0;
uint8_t			eeAlarm2_hour EEMEM = 0;
uint8_t			eeAlarm2_minute EEMEM = 0;

// define UART vars
#define				uart_maxstrlen 50
volatile uint8_t	uart_str_complete=0;
volatile uint8_t	uart_str_count=0;
volatile char uart_string[uart_maxstrlen+1]="X";
volatile char uart_target[uart_maxstrlen+1]="X";

/**** Local function prototypes ***********************************************/

//------------------------------------------------------------------------------
// Name:		uart_init
// Function:	UART initialisieren
//
// Parameter:
// Return:
//------------------------------------------------------------------------------
void uart_init(void)
{
	// Baudrate einstellen (Normaler Modus)
	UBRRH = (unsigned char) (UBRR_BAUD>>8);
	UBRRL = (unsigned char) (UBRR_BAUD & 0x0ff);
	
	// oder einfacher:
	// UBRR = UBRR_BAUD;

	// Aktivieren des Empfängers, des Senders und des "Daten empfangen"-Interrupts
	UCSRB = (1<<RXCIE)|(1<<RXEN)|(1<<TXEN);

	// Einstellen des Datenformats: 8 Datenbits, 1 Stoppbit
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}


//------------------------------------------------------------------------------
// Name:		draw_time
// Function:	Zeit entsprechend darstellen
//            
// Parameter: 
//				time_setup = 1  --> draw minutes
//				time_setup = 2 --> draw hours
//				time_setup = 3 --> draw alarm1_minutes
//				time_setup = 4 --> draw alarm1_hours
//				time_setup = 5 --> draw alarm2_minutes
//				time_setup = 6 --> draw alarm2_hours
// Return:    
//------------------------------------------------------------------------------
void draw_time(void)
{
	uint8	i,h;

	if(time_setup == 2)
	{
		i = (hour / 10) * 4;
		led[0] = font[i];
		led[1] = font[i+1];
		led[2] = font[i+2];
		led[3] = font[i+3];
		led[4] = 0;
		i = (hour % 10) * 4;
		led[5] = font[i];
		led[6] = font[i+1];
		led[7] = font[i+2];
		led[8] = font[i+3];
		led[9] = 0;

		if(second & 1)	{	led[10] = 0x44;	led[11] = 0;	}
			else					{	led[10] = 0;	led[11] = 0x44;	}
	}
	else if(time_setup == 1)
	{
		if(second & 1)	{	led[0] = 0x44;	led[1] = 0;	}
			else					{	led[0] = 0;	led[1] = 0x44;	}
		led[2] = 0;
		i = (minute / 10) * 4;
		led[3] = font[i];
		led[4] = font[i+1];
		led[5] = font[i+2];
		led[6] = font[i+3];
		led[7] = 0;
		i = (minute % 10) * 4;
		led[8] = font[i];
		led[9] = font[i+1];
		led[10] = font[i+2];
		led[11] = font[i+3];
	}
	else if(time_setup == 4)
	{
		i = (alarm1_hour / 10) * 4;
		led[0] = font[i];
		led[1] = font[i+1];
		led[2] = font[i+2];
		led[3] = font[i+3];
		led[4] = 0;
		i = (alarm1_hour % 10) * 4;
		led[5] = font[i];
		led[6] = font[i+1];
		led[7] = font[i+2];
		led[8] = font[i+3];
		led[9] = 0;

		if(second & 1)	{	led[10] = 0x44;	led[11] = 0;	}
		else					{	led[10] = 0;	led[11] = 0x44;	}
		WORD_ALARM1;
	}
	else if(time_setup == 3)
	{
		if(second & 1)	{	led[0] = 0x44;	led[1] = 0;	}
		else					{	led[0] = 0;	led[1] = 0x44;	}
		led[2] = 0;
		i = (alarm1_minute / 10) * 4;
		led[3] = font[i];
		led[4] = font[i+1];
		led[5] = font[i+2];
		led[6] = font[i+3];
		led[7] = 0;
		i = (alarm1_minute % 10) * 4;
		led[8] = font[i];
		led[9] = font[i+1];
		led[10] = font[i+2];
		led[11] = font[i+3];
		WORD_ALARM1;
	}
	else if(time_setup == 6)
	{
		i = (alarm2_hour / 10) * 4;
		led[0] = font[i];
		led[1] = font[i+1];
		led[2] = font[i+2];
		led[3] = font[i+3];
		led[4] = 0;
		i = (alarm2_hour % 10) * 4;
		led[5] = font[i];
		led[6] = font[i+1];
		led[7] = font[i+2];
		led[8] = font[i+3];
		led[9] = 0;

		if(second & 1)	{	led[10] = 0x44;	led[11] = 0;	}
		else					{	led[10] = 0;	led[11] = 0x44;	}
		WORD_ALARM2;
	}
	else if(time_setup == 5)
	{
		if(second & 1)	{	led[0] = 0x44;	led[1] = 0;	}
		else					{	led[0] = 0;	led[1] = 0x44;	}
		led[2] = 0;
		i = (alarm2_minute / 10) * 4;
		led[3] = font[i];
		led[4] = font[i+1];
		led[5] = font[i+2];
		led[6] = font[i+3];
		led[7] = 0;
		i = (alarm2_minute % 10) * 4;
		led[8] = font[i];
		led[9] = font[i+1];
		led[10] = font[i+2];
		led[11] = font[i+3];
		WORD_ALARM2;
	}
	else
	{
		CLEAR_ALL;
		WORD_ES_IST;
		WORD_UHR;

		h = 0;
		if(     minute <   5)	{	                            					h = hour; }
		else if(minute <  10)	{	WORD_FUENF;   WORD_NACH; 							h = hour; }
		else if(minute <  15)	{	WORD_ZEHN;    WORD_NACH; 							h = hour; }
		else if(minute <  20)	{	WORD_VIERTEL; WORD_NACH; 							h = hour; }
		else if(minute <  25)	{	WORD_ZEHN;  	WORD_VOR_1;	WORD_HALB;	h = hour + 1; }
		else if(minute <  30)	{	WORD_FUENF;   WORD_VOR_1; WORD_HALB;	h = hour + 1; }
		else if(minute <  35)	{	WORD_HALB;  													h = hour + 1; }
		else if(minute <  40)	{	WORD_FUENF; 	WORD_NACH; 	WORD_HALB;	h = hour + 1; }
		else if(minute <  45)	{	WORD_ZEHN; 		WORD_NACH; 	WORD_HALB;	h = hour + 1; }
		else if(minute <  50)	{	WORD_VIERTEL;	WORD_VOR_2;							h = hour + 1; }
		else if(minute <  55)	{	WORD_ZEHN; 		WORD_VOR_1;							h = hour + 1; }
		else if(minute <  60)	{	WORD_FUENF; 	WORD_VOR_1;							h = hour + 1; }

		if(     h ==  0) { WORD_12; }
		else if(h ==  1) { WORD_1;  }
		else if(h ==  2) { WORD_2;  }
		else if(h ==  3) { WORD_3;  }
		else if(h ==  4) { WORD_4;  }
		else if(h ==  5) { WORD_5;  }
		else if(h ==  6) { WORD_6;  }
		else if(h ==  7) { WORD_7;  }
		else if(h ==  8) { WORD_8;  }
		else if(h ==  9) { WORD_9;  }
		else if(h == 10) { WORD_10; }
		else if(h == 11) { WORD_11; }
		else if(h == 12) { WORD_12; }
		else if(h == 13) { WORD_1;  }
		else if(h == 14) { WORD_2;	}
		else if(h == 15) { WORD_3;	}
		else if(h == 16) { WORD_4;	}
		else if(h == 17) { WORD_5;	}
		else if(h == 18) { WORD_6;	}
		else if(h == 19) { WORD_7;	}
		else if(h == 20) { WORD_8;	}
		else if(h == 21) { WORD_9;	}
		else if(h == 22) { WORD_10;	}
		else if(h == 23) { WORD_11;	}
			
		if (alarm1_enabled == true)
		{
			WORD_ALARM1;
		}
		if (alarm2_enabled == true)
		{
			WORD_ALARM2;
		}
		if (ntp_sync == true)
		{
			WORD_NTP;
		}
		
	}
}

//------------------------------------------------------------------------------
// Name:      init
// Function:  Initalize defaults
//
// Parameter:
// Return:
//------------------------------------------------------------------------------
void init(void)
{
	// Configure Dataport as output
	DDRC |= ((1<<PC3) | (1<<PC2) | (1<<PC1) | (1<<PC0));
	DDRD |= ((1<<PD7) | (1<<PD6) | (1<<PD5) | (1<<PD4) | (1<<PD3));
	DDRB |= ((1<<PB0) | (1<<PB1) | (1<<PB2) | (1<<PB3) | (1<<PB4));
	
	// Configure Dataport as input
	DDRD &= ~((1<<PD0) | (1<<PD1) | (1<<PD2));
	DDRC &= ~((1<<PC5) | (1<<PC4));

	// enable internal pullup
	PORTD |= ((1<<PD0) | (1<<PD1) | (1<<PD2));
	PORTC |= ((1<<PC5) | (1<<PC4));
	SFIOR &= (1<<PUD);
	
	// Timer 2 mit externem 32kHz Quarz betreiben
	ASSR |= (1<<AS2);
	TCCR2	= (1<<CS22) + (1<<CS20);			// /128 für 1Hz Int

	// Timer 1 für LED INT mit ca. 1,2kHz
	TCCR1A = 0;
	TCCR1B = (1<<WGM12) + (1<<CS10);
	OCR1A = 6667;

	// Timer Interrupts

	TIMSK  = (1<<OCIE1A) + (1<<TOIE2);   // set interrupt mask
	

	// Set vars inital
	hour = 5;
	minute = 59;
	second = 0;
	time_setup = 0;
	ntp_sync = false;
	
	// read from EEPROM
	alarm1_hour = eeprom_read_byte (&eeAlarm1_hour);
	alarm1_minute = eeprom_read_byte (&eeAlarm1_minute);
	alarm2_hour = eeprom_read_byte (&eeAlarm2_hour);
	alarm2_minute = eeprom_read_byte (&eeAlarm2_minute);
	
	
	alarm = false;
	alarm1_enabled = true;
	alarm2_enabled = true;
}


//------------------------------------------------------------------------------
//		MAIN
//------------------------------------------------------------------------------
int main(void)
{
 	init();						// Initialize
	uart_init();				// USART initialisieren
	sei();						// Interrupt ein	

	while(1)
	{
		if(sec_flag)			//=== 1 Sekunde ===
		{
			sec_flag = false;
			if(BUTTON)
			{
				time_setup_cnt1++;
				if(time_setup_cnt1 > 3)
				{
					time_setup++;
					if(time_setup > 6) time_setup = 0;  // Leave settings after all menu steps 
				}
			}			
			else
			{
				time_setup_cnt0++;
				if(time_setup_cnt0 > 10) time_setup = 0;  // Set timeout to go back from settings to normal display (10 sec.)
			
			if ((!eeprom_written) && (time_setup == 0))
			{
			eeprom_written = true;
			eeprom_write_byte(&eeAlarm1_hour, alarm1_hour);
			eeprom_write_byte(&eeAlarm1_minute, alarm1_minute);
			eeprom_write_byte(&eeAlarm2_hour, alarm2_hour);
			eeprom_write_byte(&eeAlarm2_minute, alarm2_minute);
			}
			
			}
			draw_time();
		}
		
			
		if(key_flag)	//=== 10ms ===	
		{
			key_flag = false;
			
			if (!UP && up_mem && button_mem)
			{
				up_mem = false;
				time_setup_cnt0 = 0;
				eeprom_written = false;						// eeprom muss noch geschrieben werden!
				if(time_setup == 1)	minute = (minute<59) ?minute+1 : 0;
				if(time_setup == 2) hour = (hour < 23) ? hour+1 :	0;
				if(time_setup == 3)	alarm1_minute = (alarm1_minute<59) ?alarm1_minute+1 : 0;
				if(time_setup == 4) alarm1_hour = (alarm1_hour < 23) ? alarm1_hour+1 :	0;
				if(time_setup == 5)	alarm2_minute = (alarm2_minute<59) ?alarm2_minute+1 : 0;
				if(time_setup == 6) alarm2_hour = (alarm2_hour < 23) ? alarm2_hour+1 :	0;
			}
			if (!DOWN && down_mem && button_mem)
			{
				down_mem = false;
				time_setup_cnt0 = 0;
				eeprom_written = false;						// eeprom muss noch geschrieben werden!
				if(time_setup == 1)	minute = (minute>0) ?minute-1 : 59;
				if(time_setup == 2) hour = (hour > 0) ? hour-1 :	23;
				if(time_setup == 3)	alarm1_minute = (alarm1_minute>0) ?alarm1_minute-1 : 59;
				if(time_setup == 4) alarm1_hour = (alarm1_hour > 0) ? alarm1_hour-1 :	23;
				if(time_setup == 5)	alarm2_minute = (alarm2_minute>0) ?alarm2_minute-1 : 59;
				if(time_setup == 6) alarm2_hour = (alarm2_hour > 0) ? alarm2_hour-1 :	23;
			}
			if(BUTTON)
			{
				button_mem = true;
				time_setup_cnt0 = 0;
			}
			else
			{
				time_setup_cnt1 = 0; 
			}
			
			if (UP)
			{
				up_mem_al = true;
				up_mem = true;
			} 
			if (DOWN)
			{
				down_mem_al = true;
				down_mem = true;
			}
			
			if (!UP && up_mem_al)
			{
				up_mem_al = false;
				alarm1_enabled ^= true;
				BUZZER_OFF;
			}
			
			if (!DOWN && down_mem_al)
			{
				down_mem_al = false;
				alarm2_enabled ^= true;
				BUZZER_OFF;
			}
			
			draw_time();		
		}
		
		// check alarm time = now
		if ((alarm1_enabled == true) && (alarm1_hour == hour) && (alarm1_minute == minute))
		{
				alarm = true;
		}	
		else if ((alarm2_enabled == true) && (alarm2_hour == hour) && (alarm2_minute == minute))
		{
				alarm = true;
		}	
		 else
		 {
			 alarm = false;
		 }
		
		// UART get NTP time from ESP8266
		if (uart_str_complete == 1)
		{
			if ((uart_string[7] == 'S') && (uart_string[8] == 'Y')  && (uart_string[9] == 'S')  && (uart_string[10] == 'T')  && (uart_string[11] == 'I')  && (uart_string[12] == 'M')  && (uart_string[13] == 'E'))
			{
			ntp_hour = (uart_string[15] - '0') * 10 + (uart_string[16] - '0');
			ntp_minute = (uart_string[18] - '0') * 10 + (uart_string[19] - '0');
			ntp_received = true;
			}
			uart_str_complete=0;
		}
		
		// Check if NTP time in sync with local time
		if ((ntp_hour != hour) | (ntp_minute != minute))
		{
			if (ntp_received == true)
			{
			hour = ntp_hour;
			minute = ntp_minute;
			ntp_sync = true;
			ntp_received = false;
			} else {
				ntp_sync = false;
			}
		} 
		
		// Alarm tone
		if (alarm == true)
		{
			if (beep < 50)
			{
				BUZZER_ON;
			}
			else if (beep > 50)
				{
				BUZZER_OFF;
				//beep = 0;
			}
		}
		if (beep > 900)
		{
			beep = 0;
		}
		
	}
}


//------------------------------------------------------------------------------
// Name:      TIMER2_OVF_vect
// Function:  TIMER2 Overflow mit 1Hz (32kHz Uhrenquarz /128 /256
//            
// Parameter: 
// Return:    
//------------------------------------------------------------------------------
ISR(TIMER2_OVF_vect)    
{
	second++;
	if(second > 59)
	{
		second=0;
		minute++;
		if(minute > 59)
		{
			minute = 0;
			hour++;
			if(hour > 23) hour = 0;
		}
	}		
	sec_flag = true;
}


//------------------------------------------------------------------------------
// Name:      USART_RXC_vect
// Function:  Interrupt wird ausgelöst sobald neue Daten im USART-Empfangspuffer liegen
//
// Parameter:
// Return:
//------------------------------------------------------------------------------
ISR(USART_RXC_vect)
{
	unsigned char buffer;
	buffer = UDR;
	if ( uart_str_complete == 0 )
	{
		if  (buffer!='\n' && buffer!= '\r' && uart_str_count<uart_maxstrlen-1)
		{
			uart_string[uart_str_count] = buffer;
			uart_str_count++;
		}
		else
		{
			uart_string[uart_str_count] = '\0';
			uart_str_count = 0;
			uart_str_complete = 1;
		}
	}
	
	// Daten aus dem Puffer lesen ...
	//if (UDR == '\n')
	//{
	//	receive_UART[uart_count++] = '\0';
		//strcpy(copy_UART,receive_UART);
	//	uart_count = 0;
	//	uart_data = true;
	//} else {
	//receive_UART[uart_count++] = UDR;
	//}
	

	// ... warten bis der Sendepuffer leer ist ...
	//while ( !( UCSRA & (1<<UDRE)) );
}

//------------------------------------------------------------------------------
// Name:      TIMER1_COMPA_vect
// Function:  LED-Matrix Refresh mit 1,2kHz -> 100Hz Bildwiderholrate
//            
// Parameter: 
// Return:    
//------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)    
{
	col_cnt++;
	if(col_cnt > 11)
	{
		col_cnt=0;
		DATA_CLR;
	}
	else
	{
		DATA_SET;
	}
	CLK_SET;
	CLK_CLR;

	//PORTC &= ~0x0F;
	//PORTC &= ~0#b00001111;
	PORTC &= ~((1<<PC3) | (1<<PC2) | (1<<PC1) | (1<<PC0));
	//PORTD &= ~0xF0;
	//PORTD &= ~0b11110000;
	PORTD &= ~((1<<PD7) | (1<<PD6) | (1<<PD5) | (1<<PD4));
	//PORTB &= ~0x03;
	//PORTB &= ~0b00000011;
	PORTB &= ~((1<<PB1) | (1<<PB0));

	STROBE_SET;
	STROBE_CLR;

	col = led[col_cnt];

	//PORTC |= col & 0x0F;
	PORTC |= col & ((1<<PC3) | (1<<PC2) | (1<<PC1) | (1<<PC0));	
	PORTD |= col & ((1<<PD7) | (1<<PD6) | (1<<PD5) | (1<<PD4));
	PORTB |= (col >> 8) & ((1<<PB1) | (1<<PB0));
	
	// Entprellen der Taster
	key_cnt++;
	if(key_cnt > 111)
	{
		key_flag = true;			// zum sampeln/entprellen der Taste
		key_cnt=0;
	}
	
	// Zähler für Alarmton
	beep++;
	
	
		
}




