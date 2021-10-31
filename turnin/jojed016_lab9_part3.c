/*	Author: Jasmine Ojeda jojed016@ucr.edu
 *	Lab Section: 022
 *	Assignment: Lab 9  Exercise 3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtube.com/shorts/yYIccvIFsqM?feature=share 
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include <stdio.h>
#endif

#define SIZEOF(a) sizeof(a)/sizeof(*a)
#define B 246.94
#define C 261.63
#define D 293.66
#define E 329.63
#define F 349.23
#define G 392.00
#define G_F 369.99
#define A 440.00
#define B_UP 493.88
#define C_UP 523.25

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

enum States {START, WAIT, PLAY, WAIT_RELEASE} state; 

double frequency = 0x00;

double note[] = {G_F, 0, G_F, 0, F, 0, F, 0, B, 0, D, B, 0, B, 0, G_F, 0, G_F, 0, F, 0, F, 0, B, 0, 
                   G_F, 0, G_F, 0, F, 0, F, 0, B, 0, D, 0, B, 0, D, 0, E, 0, C, 0, D, 0, B};
unsigned char time[] = {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 3, 3, 1, 2, 1, 1, 1, 3, 1, 2, 1, 2, 1, 2, 9,
                        2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 4, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2};
unsigned char i;
unsigned char j;
unsigned char size = SIZEOF(note);

void Tick() {
    unsigned char tmp_A = (~PINA) & 0x01;

    switch(state) {
        case START:
            i = 0;
	    frequency = 0; 
	    state = WAIT;
            break;

	case WAIT:
            if (tmp_A) {
		i = 0;
		j = 0;
		state = PLAY;
	    }
	    else {
		state = WAIT;
	    }
            break;

	case PLAY:
            if (i < size) {
		state = PLAY;
	    }
            else {
                state = WAIT_RELEASE;
	    }	    
	    break;
	case WAIT_RELEASE:
	    if (tmp_A) {
                state = WAIT_RELEASE;
	    }
	    else {
                state = WAIT;
            }
        default: break;
    }

    switch(state) {
	case START:
            break;
	case WAIT:
            frequency = 0;
	    break;
	case WAIT_RELEASE:
	    frequency = 0;
	    break;
        case PLAY:
	    j++;

            if (j > time[i]) {
                j = 0;
                i++;
            }

            frequency = note[i];
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

    state = START;

    /* Insert your solution below */
    while (1) {
        Tick();
	set_PWM(frequency);

	while (!TimerFlag) { };
	TimerFlag = 0;
    }

    return 1;
}
