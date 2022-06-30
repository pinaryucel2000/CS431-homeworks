#include <REG51.H>
#include <inttypes.h>
#include "stdio.h"
#include <string.h>
#include <stdlib.h>

#define L 204
#define S 102
#define MAX_BUFFER_SIZE 4
#define SYS_CLOCK 29.4912
#define TR_CLOCK 2.4576 // SYS_CLOCK / 12 (MHz)
#define STEP 0.0004069 // 1 / TR_CLOCK(Hz) (ms) 

// These variables are calculated before the delay function
// is called to have a more optimized delay function.
uint16_t TL0_value;
uint16_t TH0_value;

char buffer[MAX_BUFFER_SIZE] = {'F', 'F', 'F', 'F'};
char flash_buffer[MAX_BUFFER_SIZE];
uint16_t buffer_size = 0;
uint16_t flash_buffer_size = 0;
uint16_t fi = 0;
char last_input = '\0';

uint8_t morse_lookup[10][4] =  {{'S', 'S', 'S', 'S'}, // 0
														    {'L', 'S', 'S', 'S'}, // 1
															  {'S', 'L', 'S', 'S'}, // 2
															  {'S', 'S', 'L', 'S'}, // 3
														    {'S', 'S', 'S', 'L'}, // 4
															  {'L', 'S', 'S', 'L'}, // 5
															  {'S', 'L', 'S', 'L'}, // 6
																{'S', 'S', 'L', 'L'}, // 7
																{'L', 'S', 'L', 'L'}, // 8
																{'S', 'L', 'L', 'L'}  // 9
															 };

void delay(uint16_t max); 
void append_buffer(char value);					
uint16_t is_valid(char value);						
void flash_char(char value);	
void flash_long();
void flash_short();		
void flash_string(char* value);					
void set_timer_values(uint16_t delay);
void delay_with_timer(); // delay amount is determined by set_timer_values before this function is called															 


void serial_isr() interrupt 4{
	if(RI == 1){
		last_input = SBUF;
		SBUF = last_input;  
		RI = 0;
	}
	else if(TI == 1){
		TI = 0;              
	}
}


void main (void){
	SCON  = 0x50;                   /* SCON: mode 1, 8-bit UART, enable rcvr    */
	TMOD |= 0x21;                   /* TMOD: timer 1, mode 2, timer 0, mode 1   */
	TH1   = 0xfc;                   /* TH1:  252 																*/
	TR1   = 1;                      /* TR1:  timer 1 run                        */
	TI    = 1;                      /* TI:   set TI to send first char of UART  */	
	P0 = 00000000;
	
	set_timer_values(20);

	ES  = 1;      // Enable Serial INterrupt
  EA  = 1;      // Enable Global Interrupt bit

	
	while(1){
			// 1. Demonstrate you are able to receive chars by echoing them back.
			// Do not need to do anything. ISR already does this.
		
			if(last_input != '\0' && last_input != '\n'){
				append_buffer(last_input);	
				last_input = '\0';	
			}
			
			// 2. Demonstrate you are able to save sequences by printing them back when enter is pressed. 
			if(last_input == '\n') {
				
				RI = 1;	
				printf("%c", buffer[0]); 
				RI = 0;
				printf("%c", buffer[1]); 	
				RI = 0;
				printf("%c", buffer[2]); 	
				RI = 0;
				printf("%c", buffer[3]); 	
			
				last_input = '\0';			
			}
		
	}
	
	while(1){
			// 3. Demonstrate you are able to flash certain hard-coded chars. 
			flash_char('1');
			delay(100);
			flash_char('2');
			delay(100);
			
			// 4. Demonstrate you are able to flash hard-coded strings.
			// 5. Demonstrate you can report back to serial when you finish flashing strings.
			flash_string("h9e2l3l4e");
			RI = 1;
			printf("Ended.");
	}

  // 6. Demonstrate you can report back to serial when you cancel flashing strings; i.e. when a new sequence is entered before flashing a new one is finished.
  // 7. Connect it all together, and demonstrate full functionality. i.e. demonstrate you can cancel flashing of sequences and handle other test cases.	
	while(1){
		if(last_input != '\0' && last_input != '\n'){
			append_buffer(last_input);	
			last_input = '\0';	
		}
		
		if(fi < flash_buffer_size){
			flash_char(flash_buffer[fi]);
			fi++;
			
			if(fi == flash_buffer_size){
				RI = 1;
				printf("Ended.\n");
				fi = 0;
				flash_buffer_size = 0;
			}
		}
		
		if(last_input == '\n') {
			if(flash_buffer_size != 0 && fi != flash_buffer_size){
				RI = 1;
				printf("Cancelled.\n");
				fi = 0;
				flash_buffer_size = 0;		
			}else{
				for(fi = 0; fi < buffer_size; fi++){
					flash_buffer[fi] = buffer[fi];
				}		
					
				flash_buffer_size = buffer_size;
				buffer_size = 0;
				fi = 0;					
			}
			last_input = '\0';			
		}
	}
}

void flash_string(char* value){
	uint16_t i;
	
	for(i = 0; i < strlen(value); i++){
		flash_char(value[i]);
		delay(50);
	}
}	


void flash_long(){
	P1 = L;
	delay(20);
	P1 = 0;
	delay(20);
}

void flash_short(){
	P1 = S;
	delay(20);
	P1 = 0;
	delay(20);
}	

void flash_char(char value){
	uint16_t index;
	uint16_t i; 
	
	if(is_valid(value) == 0){
		return;
	}
	
	index = atoi(&value);
	
	for(i = 0 ; i < 4; i++){
		if(morse_lookup[index][i] == 'S'){
			flash_short();
		}
		else{
			flash_long();
		}
	}
	
}

void append_buffer(char value){
	uint16_t i; 
	
	if(is_valid(value) == 0){
		return;
	}
	
	if(buffer_size + 1 <= MAX_BUFFER_SIZE){
		buffer[buffer_size] = value;
		buffer_size++;
	}
	else{
		for(i = 0; i < MAX_BUFFER_SIZE - 1; i++){
			buffer[i] = buffer[i+1];
		}
		
		buffer[buffer_size - 1] = value;
	}
		
}

uint16_t is_valid(char value){
	if(value != '0' && value != '1' && value != '2' && value != '3' && value != '4' && value != '5' && value != '6' && value != '7' && value != '8' && value != '9'){
		return 0;
	}
	else{
		return 1;
	}
}

void delay(uint16_t max){
	uint16_t i;
	
	for(i = 0; i < max; i++){}
}

void set_timer_values(uint16_t delay){
	float value = 65536 - delay / STEP; // Should be greater than 0
	TL0_value = ((uint16_t)value & 0x00ff);
	TH0_value = ((uint16_t)value & 0xff00) >> 8;
}

// delay_time is in milliseconds
void delay_with_timer(){
	TL0 = TL0_value;
	TH0 = TH0_value;
	TR0 = 1;
	while(TF0 == 0){};
	TR0 = 0;
	TF0 = 0;
}
