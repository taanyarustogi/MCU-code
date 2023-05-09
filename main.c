/*
 * main.c
 *
 * Author : Tanya Rustogi
 */ 

#define F_CPU 8000000 // Clock speed

#include <avr/io.h>
#include <util/delay.h> //for delay function
#include <stdio.h>
#include <math.h>

// pll
#include <stdbool.h>
#include <stdint.h>
#include "./twi.h"
#include "./si5351.h"

// encoder
#include "./encoder.h"

// lcd
#include "i2c.h"
#include "screen_cmds.h"
volatile uint8_t pin_value;

void screen_init(void)
{
	// TODO: Initialize screen
	I2Csendcmd(SCREEN_ADDR, COMMAND_8BIT_4LINES_NORMAL_RE1_IS0);
	I2Csendcmd(SCREEN_ADDR, COMMAND_NW);
	I2Csendcmd(SCREEN_ADDR, COMMAND_SEGMENT_BOTTOM_VIEW);
	I2Csendcmd(SCREEN_ADDR, COMMAND_BS1_1);
	I2Csendcmd(SCREEN_ADDR, COMMAND_8BIT_4LINES_RE0_IS1);
	I2Csendcmd(SCREEN_ADDR, COMMAND_BS0_1);
	I2Csendcmd(SCREEN_ADDR, COMMAND_FOLLOWER_CONTROL);
	I2Csendcmd(SCREEN_ADDR, COMMAND_POWER_BOOSTER_CONTRAST);
	I2Csendcmd(SCREEN_ADDR, COMMAND_SET_CONTRAST_1010);//0x7F
	I2Csendcmd(SCREEN_ADDR, COMMAND_8BIT_4LINES_RE0_IS0);
	I2Csendcmd(SCREEN_ADDR, COMMAND_DISPLAY_ON_CURSOR_ON_BLINK_ON);
	I2Csendcmd(SCREEN_ADDR, COMMAND_CLEAR_DISPLAY);
 }

void screen_write_string(char string_to_write[])
{
	int letter=0;
	
	I2Csendcmd(SCREEN_ADDR, COMMAND_CLEAR_DISPLAY);
	I2Csendcmd(SCREEN_ADDR, COMMAND_SET_CURSOR_LINE_1);
	int current_line = COMMAND_SET_CURSOR_LINE_1;
	
	while(string_to_write[letter]!='\0')
	{
		if ((letter != 0) && (letter % LINE_LENGTH == 0))
		{
			if (current_line == COMMAND_SET_CURSOR_LINE_4){
				// We've gone past the end of the screen, go back to top
				current_line = COMMAND_SET_CURSOR_LINE_1;
				// Clear the screen 
				I2Csendcmd(SCREEN_ADDR, COMMAND_CLEAR_DISPLAY);
			}
			else {
				current_line = current_line+0x20;
			}
			// We've gone past the end of the line, go to the next one
			I2Csendcmd(SCREEN_ADDR, current_line); 
		}
		
		I2Csenddatum(SCREEN_ADDR, string_to_write[letter]);
		letter++;
	}
}

/*unsigned char button_state(){
	if (pin_value == 1) {
		if ((PINA & (1<<PINA0)) == 0) {
			_delay_ms(5);
			if ((PINA & (1<<PINA0)) == 0) {
				pin_value = PINA;
				return 1;
			}
		}
	}
	pin_value = PINA;
	return 0;
}*/


int main(void)
{
	uint32_t FA = 9000000;
	bool enabled = true;
	
	// Set CLK to 8 MHz
	CLKPR = 1<<CLKPCE;
	CLKPR = 0;

	// from encoder
	volatile int16_t count = 0;
	
	//DDRA = DDRA | (1<<DDA6); //set portA6 as output and portA7 as input
	//PORTA = PORTA | (1<<PORTA6); // sets reset as high

	DDRD = 0xff; //PortD as output (only need PD6 for display)
	const int STR_LEN = 40;
	const float VREF = 3.3; // Measure this with a voltmeter
	volatile char string_to_write[STR_LEN];
	
	_delay_ms(5);
	PORTD = PORTD & (0<<PD6); // turn off
	_delay_ms(200);
	PORTD = PORTD | (1<<PD6); // turn on display
	_delay_ms(5);
	
	twi_init();
	//Initialize display
	screen_init(); 
	

	si5351_init();
	setup_PLL(SI5351_PLL_A, 28, 0, 1);
	set_LO_freq(FA);
	enable_clocks(enabled);

	if(encoder_init()==false)
		return;
	
	while (1)
	{
	/*	if(button_state()) {
			PORTD ^= (1<<PORTD7);
			
		} //txen signal*/
		
		count = getCount();
		char str[40];
		sprintf(str, "%d", count);
		screen_write_string(str);
		//Write some data to get started
		strncpy(string_to_write," M Hz ",STR_LEN);
		int letter = 0;
		while(string_to_write[letter]!='\0')
		{	I2Csenddatum(SCREEN_ADDR, string_to_write[letter]);
			letter++;
		}
		if (isPressed() == true) {
			FA = count * 1000000;
			set_LO_freq(FA);
		}
	}
}


