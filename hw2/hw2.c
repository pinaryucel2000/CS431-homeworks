#include <REG51.H>
#include <inttypes.h>
#include "stdio.h"
#include <string.h>
#include "lcd.h"

#define SYS_CLOCK 29.4912
#define TR_CLOCK 2.4576 // SYS_CLOCK / 12 (MHz)
#define STEP 0.0004069 // 1 / TR_CLOCK(Hz) (ms)

// These variables are calculated before the delay function
// is called to have a more optimized delay function.
uint16_t TL0_value;
uint16_t TH0_value;

// Keypad setup
sbit R1 = P0^0;
sbit R2 = P0^1;
sbit R3 = P0^2;
sbit R4 = P0^3;
sbit C1 = P0^4;
sbit C2 = P0^5;
sbit C3 = P0^6;

// LCS setup
sbit DB7 = P1^7;
sbit DB6 = P1^6;
sbit DB5 = P1^5;
sbit DB4 = P1^4;
sbit RS = P1^3;
sbit E = P1^2;
sbit clear = P2^4;
sbit ret = P2^5;				  
sbit left = P2^6;
sbit right = P2^7;

// Below are the functions to use LCD.
// All of the code below is taken from: https://www.edsim51.com/examples.html#prog4
void returnHome(void);
void entryModeSet(bit id, bit s);
void displayOnOffControl(bit display, bit cursor, bit blinking);
void cursorOrDisplayShift(bit sc, bit rl);
void functionSet(void);
void setDdRamAddress(char address);
void sendChar(char c);
void sendString(char* str);
bit getBit(char c, char bitNumber);
void delay();

// My functions
char read_keypad();
void set_timer_values(uint16_t delay);
void delay(); // delay amount is determined by set_timer_values before this function is called
void print(char* first_line, char* second_line);

void main (void){
	uint8_t c;
	char text_input[2];
	char keypad_input[4];
	SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
	TMOD |= 0x21;                   /* TMOD: timer 1, mode 2, timer 0, mode 1   */
	TH1   = 0xfc;                   /* TH1:  252 																*/
	TR1   = 1;                      /* TR1:  timer 1 run                        */
	TI    = 1;                      /* TI:   set TI to send first char of UART  */

	// Delay function is initially set to create 20ms delays
	set_timer_values(20);

	// LCD settings
	functionSet();
	entryModeSet(1, 0); // increment and no shift
	displayOnOffControl(1, 1, 1); // display on, cursor on and blinking on
	
	// Input settings
	keypad_input[3] = '\0';
	text_input[1] = '\0';
	
	while(1){
		// 1. Receive a character and echo it back
    printf("Send a character. ");
	
		text_input[0] = _getkey();  
		
		// Tests: _getkey(), getchar(), getc()
		// scanf("%c", &text_input[0]);  
		// text_input[0] = getchar();
		
		// getc(); There is no file system in 8051?
		
		printf(" You sent '%c'.\n", text_input[0]); // Echo
		
		// 2. Wait for 20ms period (period can be changed with set_timer_values(delay) function)
		delay();
		
		// 3. print the char in first line
		print(text_input, "");

		// 4. Make an keypad reading
		set_timer_values(10);
		printf("Click 3 buttons on keypad.\n ");
		keypad_input[0] = read_keypad();
		keypad_input[1] = read_keypad();
		keypad_input[2] = read_keypad();
		
		printf("Keypad input: %s\n", keypad_input);
		print("", keypad_input);
	}
}

void set_timer_values(uint16_t delay){
	float value = 65536 - delay / STEP; // Should be greater than 0
	TL0_value = ((uint16_t)value & 0x00ff);
	TH0_value = ((uint16_t)value & 0xff00) >> 8;
}

// delay_time is in milliseconds
void delay(){
	TL0 = TL0_value;
	TH0 = TH0_value;
	TR0 = 1;
	while(TF0 == 0){};
	TR0 = 0;
	TF0 = 0;
}

// Prints to LCD.
void print(char* first_line, char* second_line){
	
	if(first_line != ""){
		setDdRamAddress(0x00); // set address to start of first line
		sendString(first_line);
	}

	if(second_line != ""){
		setDdRamAddress(0x40); // set address to start of second line
		sendString(second_line);
	}
	
		if (ret == 0) {
			returnHome();
		}
		else {	
			if (left == 0 && right == 1) {
				cursorOrDisplayShift(1, 0); // shift display left
			}
			else if (left == 1 && right == 0) {
				cursorOrDisplayShift(1, 1); // shift display right
			}	
		}	
}

char read_keypad()
{
	while(1){
		C1=1;
		C2=1;
		C3=1;
		
		R1=0;
		R2=1;
		R3=1;
		R4=1;
		
		if(C1==0){delay();while(C1==0);return '#';}
		if(C2==0){delay();while(C2==0);return '0';}
		if(C3==0){delay();while(C3==0);return '*';}
		
		R1=1;
		R2=0;
		R3=1;
		R4=1;

		if(C1==0){delay();while(C1==0);return '9';}
		if(C2==0){delay();while(C2==0);return '8';}
		if(C3==0){delay();while(C3==0);return '7';}
			
		R1=1;
		R2=1;
		R3=0;
		R4=1;

		if(C1==0){delay();while(C1==0);return '6';}
		if(C2==0){delay();while(C2==0);return '5';}
		if(C3==0){delay();while(C3==0);return '4';}
		
		R1=1;
		R2=1;
		R3=1;
		R4=0;

		if(C1==0){delay();while(C1==0);return '3';}
		if(C2==0){delay();while(C2==0);return '2';}
		if(C3==0){delay();while(C3==0);return '1';}	
	}
}

void returnHome(void) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB5 = 1;
	E = 1;
	E = 0;
	delay();
}	

void entryModeSet(bit id, bit s) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB6 = 1;
	DB5 = id;
	DB4 = s;
	E = 1;
	E = 0;
	delay();
}

void displayOnOffControl(bit display, bit cursor, bit blinking) {
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 0;
	E = 1;
	E = 0;
	DB7 = 1;
	DB6 = display;
	DB5 = cursor;
	DB4 = blinking;
	E = 1;
	E = 0;
	delay();
}

void cursorOrDisplayShift(bit sc, bit rl) {
	RS = 0;
	DB7 = 0;
	DB6 = 0;
	DB5 = 0;
	DB4 = 1;
	E = 1;
	E = 0;
	DB7 = sc;
	DB6 = rl;
	E = 1;
	E = 0;
	delay();
}

void functionSet(void) {
	DB7 = 0;
	DB6 = 0;
	DB5 = 1;
	DB4 = 0;
	RS = 0;
	E = 1;
	E = 0;
	delay();
	E = 1;
	E = 0;
	DB7 = 1;
	E = 1;
	E = 0;
	delay();
}

void setDdRamAddress(char address) {
	RS = 0;
	DB7 = 1;
	DB6 = getBit(address, 6);
	DB5 = getBit(address, 5);
	DB4 = getBit(address, 4);
	E = 1;
	E = 0;
	DB7 = getBit(address, 3);
	DB6 = getBit(address, 2);
	DB5 = getBit(address, 1);
	DB4 = getBit(address, 0);
	E = 1;
	E = 0;
	delay();
}

void sendChar(char c) {
	DB7 = getBit(c, 7);
	DB6 = getBit(c, 6);
	DB5 = getBit(c, 5);
	DB4 = getBit(c, 4);
	RS = 1;
	E = 1;
	E = 0;
	DB7 = getBit(c, 3);
	DB6 = getBit(c, 2);
	DB5 = getBit(c, 1);
	DB4 = getBit(c, 0);
	E = 1;
	E = 0;
	delay();
}

// -- End of LCD Module instructions
// --------------------------------------------------------------------

void sendString(char* str) {
	int index = 0;
	while (str[index] != 0) {
		sendChar(str[index]);
		index++;
	}
}

bit getBit(char c, char bitNumber) {
	return (c >> bitNumber) & 1;
}
