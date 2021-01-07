/*	
 *	Lab Section: 023
 *	Assignment: Final Project
 *	Exercise Description: Programs micro controller to receive input from
 *		bluetooth module and send data to reciever microcontroller to drive motors.
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <scheduler.h>
#include <stdlib.h>
#include "bit.h"
#include "usart_1284.h"

unsigned char low1 = 0;
unsigned char low2 = 0;
unsigned char high1 = 6;
unsigned char high2 = 6;
unsigned char counter1 = 0;
unsigned char counter2 = 0;
unsigned char x = 0;

unsigned int pulse = 0;
unsigned char j = 0;
unsigned char k = 0;
int16_t distance = 10;

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
 			unsigned char direction1 = 0; //0 for forward, 1 for reverse
 			unsigned char direction2 = 0; //0 for left, 1 for right
 			x = USART_Receive(0);
 			low1 = x & 0x07;
 			direction1 = x & 0x08;
 			low2 = (x & 0x70) >> 4;
 			direction2 = x & 0x80;
 			if(direction1){
 				PORTB = SetBit(PORTB,0,0);
 				PORTB = SetBit(PORTB,1,1);
 			}
 			else{
 				PORTB = SetBit(PORTB,0,1);
 				PORTB = SetBit(PORTB,1,0);
 			}
 			if(direction2){
 				PORTB = SetBit(PORTB,3,1);
 				PORTB = SetBit(PORTB,4,0);
 			}
 			else{
 				PORTB = SetBit(PORTB,3,0);
 				PORTB = SetBit(PORTB,4,1);
 			}
 		}
		break;
		
		default:
		break;
	}
	return state;
}

enum SM2_States { SM2_s1 };

int TickFct_SM2(int state) {
	switch(state) { // Transitions and actions
		case -1: // Initial transition
		state = SM2_s1;
		break;
		
		case SM2_s1:
		if(j == 0){
			PORTD = SetBit(PORTD,3,1);
			j++;
		}
		else if(j == 1){
			PORTD = SetBit(PORTD,3,0);
			j++;
		}
		else if(j == 3){
			distance = pulse/580;
			j++;
		}
		else if(j > 8){
			j = 0;
		}
		else{
			j++;
		}
		break;
		
		default:
		state = -1;
	}
	
	return state;
}

enum SM3_States { SM3_High, SM3_Low};

int TickFct_SM3(int state){
	switch(state){ //Actions and transitions
		case -1:
		state = SM3_High;
		break;
		
		case SM3_High:
		if(counter1 < low1){
			state = SM3_High;
			if(distance > 5){
				PORTB = SetBit(PORTB,5,1);
			}
			else{
				PORTB = SetBit(PORTB,5,0);
			}
			counter1++;
		}
		else{
			state = SM3_Low;
		}
		break;
		
		case SM3_Low:
		if(counter1 >= high1){
			state = SM3_High;
			counter1 = 0;
		}
		else{
			state = SM3_Low;
			PORTB = SetBit(PORTB,5,0);
			counter1++;
		}
		break;
		
		default:
		state = -1;
		break;
	}
	return state;
}

enum SM4_States { SM4_High, SM4_Low};

int TickFct_SM4(int state){
	switch(state){ //Actions and transitions
		case -1:
		state = SM4_High;
		break;
		
		case SM4_High:
		if(counter2 < low2){
			state = SM4_High;
			if(distance > 5){
				PORTB = SetBit(PORTB,7,1);
			}
			else{
				PORTB = SetBit(PORTB,7,0);
			}
			counter2++;
		}
		else{
			state = SM4_Low;
		}
		break;
		
		case SM4_Low:
		if(counter2 >= high1){
			state = SM4_High;
			counter2 = 0;
		}
		else{
			state = SM4_Low;
			PORTB = SetBit(PORTB,7,0);
			counter2++;
		}
		break;
		
		default:
		state = -1;
		break;
	}
	return state;
}

int main(void) {
	// initialize ports
	DDRA = 0xFF;
	DDRB = 0xFF;
	DDRD = 0b11111011;
	_delay_ms(50);
	
	EIMSK|=(1<<INT0);
	EICRA|=(1<<ISC00);
	
	TCCR3A = 0;
	
	tasksNum = 4; // declare number of tasks
	task tsks[4]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i = 0; // task counter
 	tasks[i].state = -1;
 	tasks[i].period = 10;
 	tasks[i].elapsedTime = tasks[i].period;
 	tasks[i].TickFct = &TickFct_SM1;
 	
 	i++;
 	tasks[i].state = -1;
 	tasks[i].period = 10;
 	tasks[i].elapsedTime = tasks[i].period;
 	tasks[i].TickFct = &TickFct_SM2;
 	
 	i++;
	tasks[i].state = -1;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM3;
	
	i++;
	tasks[i].state = -1;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM4;
	
	TimerSet(1); // value set should be GCD of all tasks
	TimerOn();
	
	sei();
	initUSART(0);
	
	PORTB = 0x0E;

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}

 ISR(INT0_vect){
 	if (k==1){
 		TCCR3B=0;
 		pulse=TCNT3;
 		TCNT3=0;
 		k=0;
 	}
 	if (k==0){
 		TCCR3B|=(1<<CS10);
 		k=1;
 	}
 }