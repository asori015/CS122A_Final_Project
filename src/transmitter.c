/*	
 *	Lab Section: 023
 *	Assignment: Final Project
 *	Exercise Description: Programs micro controller to receive input from
 *		joystick and send data to reciever microcontroller through bluetooth.
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <scheduler.h>
#include "bit.h"
#include "usart_1284.h"

unsigned char direction1 = 0; //0 for forward, 1 for reverse
unsigned char direction2 = 0; //0 for left, 1 for right
unsigned char speed1 = 0;
unsigned char speed2 = 0;
unsigned char send = 0;
unsigned char j = 0;
unsigned int x = 0;
unsigned int y = 0;

void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	//        in Free Running Mode, a new conversion will trigger whenever
	//        the previous conversion completes.
}

void Set_A2D_Pin(unsigned char pinNum) {
	ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
	// Allow channel to stabilize
	static unsigned char i = 0;
	for ( i=0; i<15; i++ ) { asm("nop"); }
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
		if(USART_IsSendReady(0)){
			USART_Send(send,0);
 		}
		break;
		
		default:
		break;
	}
	
	return state;
}
 
enum SM2_States { SM2_s1 };

int TickFct_SM2(int state) {
	switch(state) { //Transitions
		case -1:
		state = SM2_s1;
		break;
		
		case SM2_s1:
		state = SM2_s1;
		break;
		
		default:
		state = -1;
		break;
	}
	switch(state) { //State Actions
		case SM2_s1:
		if(x > 0x0240){
			if(x > 0x03C0){
				speed2 = 3;
			}
			else if(x > 0x0340){
				speed2 = 2;
			}
			else if(x > 0x02C0){
				speed2 = 1;
			}
			else{
				speed2 = 0;
			}
			direction2 = 1;
		}
		else if(x < 0x01F0){
			if(x < 0x0020){
				speed2 = 3;
			}
			else if(x < 0x0100){
				speed2 = 2;
			}
			else if(x < 0x0180){
				speed2 = 1;
			}
			else{
				speed2 = 0;
			}
			direction2 = 0;
		}
		
		if(y > 0x022F){
			if(y > 0x03C0){
				speed1 = 3;
			}
			else if(y > 0x0310){
				speed1 = 2;
			}
			else if(y > 0x0280){
				speed1 = 1;
			}
			else{
				speed1 = 0;
			}
			direction1 = 0;
		}
		else if(y <  0x01F0){
			if(y <  0x0040){
				speed1 = 3;
			}
			else if(y <  0x0100){
				speed1 = 2;
			}
			else if(y <  0x0188){
				speed1 = 1;
			}
			else{
				speed1 = 0;
			}
			direction1 = 1;
		}
		break;
		
		default:
		break;
	}
	send = (speed2 << 4) + speed1;
	if(direction1){
		send = SetBit(send,3,1);
	}
	else{
		send = SetBit(send,3,0);
	}
	if(direction2){
		send = SetBit(send,7,1);
	}
	else{
		send = SetBit(send,7,0);
	}
	PORTB = send;
	
	return state;
}

enum SM3_States { SM3_s1 };

int TickFct_SM3(int state){
	switch(state){
		case -1:
		state = SM3_s1;
		break;
		
		case SM3_s1:
		if(j){
			x = ADC;
			Set_A2D_Pin(0x01);
			j = 0;
		}
		else{
			y = ADC;
			Set_A2D_Pin(0x00);
			j = 1;
		}
		break;
	}
	
	return state;
}
 
int main(void) {
	// initialize ports
	PORTA = 0xFF; DDRA = 0x00;
	PORTB = 0x00; DDRB = 0xFF;
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
  	tasks[i].period = 20;
  	tasks[i].elapsedTime = tasks[i].period;
  	tasks[i].TickFct = &TickFct_SM2;
  	
    i++;
    tasks[i].state = -1;
    tasks[i].period = 5;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFct_SM3;
	
	TimerSet(5); // value set should be GCD of all tasks
	TimerOn();
	
	ADC_init();
	
	initUSART(0);
	while(1) {} // task scheduler will be called by the hardware interrupt
	
}
