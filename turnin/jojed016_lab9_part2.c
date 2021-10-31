/*	Author: Jasmine Ojeda jojed016@ucr.edu
 *	Lab Section: 022
 *	Assignment: Lab 9  Exercise 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtube.com/shorts/2ec00C3XvsI?feature=share
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
    TCCR1B = 0x0B;

    OCR1A = 125;

    TIMSK1 = 0x02;

    TCNT1 = 0;

    _avr_timer_cntcurr = _avr_timer_M;

    SREG |= 0x80;
}

void TimerOff() {
    TCCR1B = 0x00;
}

void TimerISR() {
    TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
    _avr_timer_cntcurr--;
    if (_avr_timer_cntcurr == 0) {
        TimerISR();
	_avr_timer_cntcurr = _avr_timer_M;
    }
}

void TimerSet(unsigned long M) {
    _avr_timer_M = M;
    _avr_timer_cntcurr = _avr_timer_M;
}

void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if (!frequency) {
            TCCR3B &= 0x08;
	}
	else {
	    TCCR3B |= 0x03;
	}

	if (frequency < 0.954) {
	    OCR3A = 0xFFFF;
	}
        else if (frequency > 31250) {
            OCR3A = 0x0000;
	}
	else {
	    OCR3A = (short)(8000000 / (128 * frequency)) -1;
	}

	TCNT3 = 0;
	current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

enum SYSStates {SYS_START, OFF_RELEASE, OFF_PRESS, ON_RELEASE, ON_PRESS} SYS_State;

enum NOTEStates {NOTE_START, WAIT, INCREMENT, DECREMENT} NOTE_State; 

double frequency = 0x00;

void SYS_Tick() {
    unsigned char tmp_A = (~PINA) & 0x01;

    switch(SYS_State) {
        case SYS_START:
	    SYS_State = OFF_RELEASE;
	    break;
	case OFF_RELEASE:
            if (tmp_A) {
	         SYS_State = ON_PRESS; }
	    else {
	         SYS_State = OFF_RELEASE; }
	    break;

	case ON_PRESS:
	    if (tmp_A) {
	        SYS_State = ON_PRESS; }
	    else {
	        SYS_State = ON_RELEASE; }
	    break;

	case ON_RELEASE:
	    if (tmp_A) {
	        SYS_State = OFF_PRESS; }
	    else {
		SYS_State = ON_RELEASE; }
	    break;

	case OFF_PRESS:
	    if (tmp_A) {
		SYS_State = OFF_PRESS; }
	    else {
		SYS_State = OFF_RELEASE; }
	    break;
	default: break;
    }

    switch(SYS_State) {
        case SYS_START:
	    set_PWM(0); break;
	case OFF_RELEASE:
	    set_PWM(0); break;
	case ON_PRESS:
	    set_PWM(frequency); break;
	case ON_RELEASE:
	    set_PWM(frequency); break;
	case OFF_PRESS:
	    set_PWM(0); break;
	default: break;
    }
}

double note[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
unsigned char i;

void NOTE_Tick() {
    unsigned char tmp_A = (~PINA) & 0x06;

    switch(NOTE_State) {
        case NOTE_START:
            i = 4;
	    frequency = note[i]; 
	    NOTE_State = WAIT;
            break;

	case WAIT:
            if (tmp_A == 0x04) {
                NOTE_State = DECREMENT;
	    }
            else if (tmp_A == 0x02) {
                NOTE_State = INCREMENT;
	    }
            else {
                NOTE_State = WAIT;
	    }
            break;

	case INCREMENT:
            NOTE_State = WAIT;
	    break;
        case DECREMENT:
	    NOTE_State = WAIT;
	    break;
        default: break;
    }

    switch(NOTE_State) {
	case NOTE_START:
            break;
	case WAIT:
            frequency = note[i];
	    break;
        case INCREMENT:
            if (i < 7) {
                i++;
	    }
	    break;
        case DECREMENT:
            if (i > 0) {
		i--;
            }
            break;
	default: break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;

    DDRB = 0xFF; PORTB = 0x00;

    PWM_on();
    
    TimerSet(100);
    TimerOn();

    SYS_State = SYS_START;
    NOTE_State = NOTE_START;

    /* Insert your solution below */
    while (1) {
        SYS_Tick();
        NOTE_Tick();	

	while (!TimerFlag) { };
	TimerFlag = 0;
    }

    return 1;
}
