/*	
 *	Lab Section: 023
 *	Assignment: Final Project
 *	Exercise Description: Programs micro controller to receive input from
 *		from transmitter through bluetooth to set motor controller to driver motors.
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <scheduler.h>
#include "bit.h"
#include "usart_1284.h"

unsigned char low1 = 1;
unsigned char low2 = 1;
unsigned char high1 = 6;
unsigned char high2 = 6;
unsigned char counter1 = 0;
unsigned char counter2 = 0;
unsigned char x = 0;

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
	PORTB = 0x0E;
	low1 = 2;
	low2 = 2;
	return state;
}
 
 enum SM2_States { SM2_High, SM2_Low};
 
 int TickFct_SM2(int state){
	 switch(state){ //Actions and transitions
		 case -1:
		 state = SM2_High;
		 break;
		 
		 case SM2_High:
		 if(counter1 < low1){
			 state = SM2_High;
			 PORTB = SetBit(PORTB,5,1);
			 counter1++;
		 }
		 else{
			 state = SM2_Low;
		 }
		 break;
		 
		 case SM2_Low:
		 if(counter1 >= high1){
			 state = SM2_High;
			 counter1 = 0;
		 }
		 else{
			 state = SM2_Low;
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
 
 enum SM3_States { SM3_High, SM3_Low};
 
 int TickFct_SM3(int state){
	 switch(state){ //Actions and transitions
		 case -1:
		 state = SM3_High;
		 break;
		 
		 case SM3_High:
		 if(counter2 < low2){
			 state = SM3_High;
			 PORTB = SetBit(PORTB,6,1);
			 counter2++;
		 }
		 else{
			 state = SM3_Low;
		 }
		 break;
		 
		 case SM3_Low:
		 if(counter2 >= high1){
			 state = SM3_High;
			 counter2 = 0;
		 }
		 else{
			 state = SM3_Low;
			 PORTB = SetBit(PORTB,6,0);
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
  	tasks[i].period = 1;
  	tasks[i].elapsedTime = tasks[i].period;
  	tasks[i].TickFct = &TickFct_SM2;
  	
  	i++;
  	tasks[i].state = -1;
  	tasks[i].period = 1;
  	tasks[i].elapsedTime = tasks[i].period;
  	tasks[i].TickFct = &TickFct_SM3;
	
	TimerSet(1); // value set should be GCD of all tasks
	TimerOn();
	
	initUSART(0);
	
	PORTB = 0x0F;

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}
