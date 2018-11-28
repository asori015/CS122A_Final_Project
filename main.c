/*	
 *	Lab Section: 023
 *	Assignment: Final Project
 *	Exercise Description: Programs micro controller to receive input from
 *		joystick to drive LED matrix to display a dot and move it in 8 directions
 *		at varying frequencies.
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <scheduler.h>
#include "bit.h"
#include "usart_1284.h"

unsigned char motorArray[] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
unsigned char j = 0;
unsigned char angle = 0x40;
unsigned char curr_angle = 0x40;
unsigned char speed = 1;
unsigned char direction = 0; //0 for foward, 1 for reverse
unsigned char move = 0;
unsigned char x = 0;

void set_PWM(double frequency) {
	static double current_frequency = -1; // Keeps track of the currently set frequency
	
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		
		// prevents OCR0A from underflowing, using prescaler 64     // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		
		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }
		
		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

enum SM1_States { SM1_s1 };

int TickFct_SM1(int state) {
	switch(state) { //Transitions
		case -1:
		state = SM1_s1;
		break;
		
		case SM1_s1:
		state = SM1_s1;
		break;
		
		default:
		state = -1;
		break;
	}
	switch(state) { //State Actions
		case SM1_s1:
		if(USART_HasReceived(0)){
			x = USART_Receive(0);
 			if(x & 0x80){
	 			angle = x & 0x7F;
 			}
 			else{
	 			direction = (x & 0x40) >> 6;
	 			speed = x & 0x3F;
				set_PWM(speed * 100);
 			}
 		}
		break;
		
		default:
		break;
	}
	
	return state;
}

enum SM2_States { SM2_Forward, SM2_Reverse };

int TickFct_SM2(int state) {
	switch(state) { // Transitions
		case -1: // Initial transition
		if(direction){
			state = SM2_Reverse;
		}
		else{
			state = SM2_Forward;
		}
		break;
		
		case SM2_Forward:
		if(direction){
			state = SM2_Reverse;
		}
		else{
			state = SM2_Forward;
		}
		break;
		
		case SM2_Reverse:
		if(direction){
			state = SM2_Reverse;
		}
		else{
			state = SM2_Forward;
		}
		break;
		
		default:
		state = -1;
		break;
	}
	switch(state) { // State actions
		case SM2_Forward:
		PORTB = 0x0E; //0000 1110
		break;
		
		case SM2_Reverse:
		PORTB = 0x15; //0001 0101
		break;
		
		default:
		break;
	}
	return state;
}

 enum SM3_States { SM3_Wait, SM3_CW, SM3_CCW};
 
 int TickFct_SM3(int state) {
 	switch(state) { // Transitions
 		case -1: // Initial transition
 		state = SM3_Wait;
 		break;
 		
  		case SM3_Wait:
  		if(move == 1){
  			state = SM3_CW;
  			move = 0;
  		}
  		else if(move == 2){
  			state = SM3_CCW;
  			move = 0;
  		}
  		else{
  			state = SM3_Wait;
  		}
  		break;
  		
  		case SM3_CW:
  		if(j == 7){
  			state = SM3_Wait;
  			curr_angle++;
  		}
  		else{
  			state = SM3_CW;
  		}
  		break;
  		
  		case SM3_CCW:
  		if(j == 0){
  			state = SM3_Wait;
  			curr_angle--;
  		}
  		else{
  			state = SM3_CCW;
  		}
  		break;
 		
 		default:
 		state = -1;
 		break;
 	}
 	switch(state) { //State action
  		case SM3_Wait:
  		if(curr_angle != angle){
  			if(curr_angle < angle){
  				move = 1;
  				j = 0;
  			}
  			else{
  				move = 2;
  				j = 7;
  			}
  		}
  		break;
  		
  		case SM3_CW:
		PORTC = motorArray[j];
  		j++;
  		break;
  		
  		case SM3_CCW:
		PORTC = motorArray[j];
  		j--;
  		break;
 		
 		default:
 		break;
 	}
 	
 	return state;
 }

int main(void) {
	// initialize ports
	PORTA = 0xFF; DDRA = 0x00;
	PORTB = 0x00; DDRB = 0xFF;
	PORTC = 0x00; DDRC = 0xFF;
	PORTD = 0x00; DDRD = 0xFF;
	
	tasksNum = 3; // declare number of tasks
	task tsks[3]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i = 0; // task counter
	tasks[i].state = -1;
	tasks[i].period = 10;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM1;
	
	i++;
	tasks[i].state = -1;
	tasks[i].period = 100;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM2;
	
	i++;
	tasks[i].state = -1;
	tasks[i].period = 3;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM3;
	
	TimerSet(1); // value set should be GCD of all tasks
	TimerOn();
	
	initUSART(0);
	
	PWM_on();
	set_PWM(0);

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}
