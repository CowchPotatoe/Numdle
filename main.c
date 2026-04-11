/*
 * CpE 301 - Numdle.c
 *
 * Created: 4/7/2026 7:47:00 PM
 * Author : ivanj
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h> //
#include "lcd.h"
#include "keypad.h"
#include "buzzer.h"

void initTimer();
//Number of equations to pull form
#define NUM_EQUATIONS 5
//Function prototype
void checkGuess(unsigned char *guess, unsigned char *equation);
// store user guess
unsigned char guess[6];
// store correct equation
unsigned char randEq[NUM_EQUATIONS][6] = {
    {'6','+','3','0','/','5'}, // 12
    {'5','*','3','-','1','0'}, // 5
    {'1','2','-','5','*','9'}, // -33
    {'9','/','3','*','2','4'}, // 72
    {'7','*','8','-','3','6'}  // 20
};
//store answers corresponding to the equations
int answers[NUM_EQUATIONS] = {12, 5, -33, 72, 20};
// number of attempts to guess equation
int numAttempts = 5;

int main(void) {
	unsigned char pressedKey;
	unsigned char start1[] = "Numdle:";
	unsigned char start2[] = "GuessTheEquation";
	unsigned char randNum[] = "Number:";
	unsigned char next[] = "Next Guess";
	
	// initialize components
	initKeypadIO();
	lcd_init();
	buzzer_init();
	initTimer();

	lcd_gotoxy(1,1);
	lcd_print(start1);
	lcd_gotoxy(1,2);
	lcd_print(start2);	
	_delay_ms(2500);
	lcdCommanda(0x01);
	_delay_ms(2);
	lcd_gotoxy(1,1);
	lcd_print(randNum);
	
	// index used for equation
	int eqIndex = 0;

	// use timer value as random number
	eqIndex = TCNT0 % NUM_EQUATIONS;

	while(1){
		// Collect exactly 6 inputs
        for(int i = 0; i < 6; i++){
			// takes input
            checkAnyKeyPressed();
            debounce();
            pressedKey = identifyPressedKey();
			// stores input
            guess[i] = pressedKey;
			// display input
            lcd_gotoxy(i+1,2);
            lcdData(pressedKey);
            _delay_ms(300);
        }
        // ignore every input after 6 inputs unless its '='
        do {
            checkAnyKeyPressed();
            debounce();
            pressedKey = identifyPressedKey();
        } while (pressedKey != '=');
        // Check the guess
		checkGuess(guess, randEq[eqIndex]);
        // Wait for button press before
        checkAnyKeyPressed();
        debounce();
        identifyPressedKey();
        // clear screen
        lcdCommanda(0x01);
        _delay_ms(2);
        lcd_gotoxy(1,1);
        lcd_print(next);
	}
	return 0;
}

// Checks user guess against the correct equation
void checkGuess(unsigned char *guess, unsigned char *equation) {
	unsigned char correct[] = "Correct";
	unsigned char incorrect[] = "Incorrect";
	// Returns 1 if the guess is correct, 0 otherwise
	int match = 1;
    for(int i = 0; i < 6; i++){
        if(guess[i] != equation[i]){
			match = 0;
			break;
        } 
    }
	// Tells user the inputted equation is incorrect
	if (match == 0){
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		lcd_print(incorrect);  
		
		// show correct equation (debug)
		lcd_gotoxy(1,2);
		for(int i = 0; i < 6; i++){
			lcdData(equation[i]);
		}
		_delay_ms(1500);

		buzzer_error();
	} else{ // Tells user inputted equation is correct
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		lcd_print(correct);
	}
    _delay_ms(300);
}

// Used for randomness
void initTimer(){
	// Initialize Timer0 as free running timer
	TCNT0 = 0; // load timer0 = 0
	TCCR0A = 0; // Timer0: normal mode, internal clock
	// TCCR0B = 1; // Timer0: enabled, no prescaler
	TCCR0B = (1 << CS02) | (1 << CS00); // prescaler = 1024
}