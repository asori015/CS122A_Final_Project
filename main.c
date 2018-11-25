/*	
 *	Lab Section: 023
 *	Assignment: Final Project
 *	Exercise Description: N/A
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#include <scheduler.h>

unsigned char motorArray[] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09};
unsigned char j = 0;

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

void set_PWM2(double frequency) {
	static double current_frequency = -1; // Keeps track of the currently set frequency
	
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
		else { TCCR0B |= 0x03; } // resumes/continues timer/counter
		
		// prevents OCR0A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR0A = 0xFFFF; }
		
		// prevents OCR0A from underflowing, using prescaler 64     // 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR0A = 0x0000; }
		
		// set OCR0A based on desired frequency
		else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }
		
		TCNT0 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM2_on() {
	TCCR0A = (1 << COM0A0);
	
	// COM0A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS00: Set a prescaler of 64
	set_PWM2(0);
}

void PWM2_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

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

enum SM1_States { SM1_Wait, SM1_CW, SM1_CCW };

int TickFct_SM1(int state) {
	switch(state) { // Transitions
		case -1: // Initial transition
		state = SM1_Wait;
		break;
		
		case SM1_Wait:
		if(~PINA & 0x01){
			state = SM1_CW;
		}
		else if(~PINA & 0x02){
			state = SM1_CCW;
		}
		else{
			state = SM1_Wait;
		}
		break;
		
		case SM1_CW:
		if(~PINA & 0x01){
			state = SM1_CW;
		}
		else{
			state = SM1_Wait;
		}
		break;
		
		case SM1_CCW:
		if(~PINA & 0x02){
			state = SM1_CCW;
		}
		else{
			state = SM1_Wait;
		}
		break;
		
		default:
		state = -1;
	}

	switch(state) { // State actions
		case SM1_Wait:
		PORTC = 0x00;
		break;
		
		case SM1_CW:
		PORTC = motorArray[j];
		if(j == 0){
			j = 7;
		}
		else{
			j--;
		}
		break;
		
		case SM1_CCW:
		PORTC = motorArray[j];
		if(j > 6){
			j = 0;
		}
		else{
			j++;
		}
		break;
		
		default:
		break;
	}
	
	return state;
}

enum SM2_States { SM2_Wait, SM2_Forward, SM2_Backward };

int TickFct_SM2(int state) {
	switch(state) { // Transitions
		case -1: // Initial transition
		state = SM2_Wait;
		break;
		
		case SM2_Wait:
		if(~PINA & 0x04){
			state = SM2_Forward;
		}
		else if(~PINA & 0x08){
			state = SM2_Backward;
		}
		else{
			state = SM2_Wait;
		}
		break;
		
		case SM2_Forward:
		if(~PINA & 0x04){
			state = SM2_Forward;
		}
		else{
			state = SM2_Wait;
		}
		break;
		
		case SM2_Backward:
		if(~PINA & 0x08){
			state = SM2_Backward;
		}
		else{
			state = SM2_Wait;
		}
		break;
		
		default:
		state = -1;
	}
	
	switch(state) { //State action
		case SM2_Wait:
		PORTB = 0x00;
		PWM_off();
		PWM2_off();
		break;
		
		case SM2_Forward:
		PORTB = 0x16;
		PWM_on();
		PWM2_on();
		set_PWM(300);
		set_PWM2(300);
		break;
		
		case SM2_Backward:
		PORTB = 0x25;
		PWM_on();
		PWM2_on();
		set_PWM(300);
		set_PWM2(300);
		break;
	}
	
	return state;
}

int main(void) {
	// initialize ports
	PORTA = 0xFF; DDRA = 0x00;
	PORTB = 0x00; DDRB = 0xFF;
	PORTD = 0x00; DDRC = 0xFF;
	
	tasksNum = 2; // declare number of tasks
	task tsks[2]; // initialize the task array
	tasks = tsks; // set the task array
	
	// define tasks
	unsigned char i = 0; // task counter
	tasks[i].state = -1;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM1;
	
	i++;
	tasks[i].state = -1;
	tasks[i].period = 50;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct_SM2;
	
	TimerSet(1); // value set should be GCD of all tasks
	TimerOn();
	
	//ADC_init();

	while(1) {} // task scheduler will be called by the hardware interrupt
	
}
