/*
 * CpE 301 - Numdle.c
 *
 * Created: 4/7/2026 7:47:00 PM
 * Author : ivanj
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "i2c.h"
#include "lcd.h"
#include "keypad.h"
#include "buzzer.h"

//Number of equations to pull form
#define NUM_EQUATIONS 5

//Function prototypes
void initTimer();
int checkGuess(unsigned char *guess, unsigned char *equation);

// Variables and arrays:
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
int attempts = 0;
// store guess history (5 attempts × 6 chars)
unsigned char guessHistory[5][6];
// boolean for game over status
int gameOver = 0;

int main(void) {
	unsigned char pressedKey;
	unsigned char start1[] = "Numdle:";
	unsigned char start2[] = "GuessTheEquation";
	unsigned char randNum[] = "Number:";
	unsigned char next[] = "Next Guess";
	unsigned char win[] = "YOU WIN!!!";
	unsigned char lose[] = "Incorrect";
	unsigned char correct[] = "Correct Eq:";
	int match = 0;

	
	// initialize components
	initKeypadIO();
	i2c_init();
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
		// clear input
		for (int i = 0; i < 6; i++) {
			guess[i] = 0;
		}
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
		
		// store guess in history
		for (int i = 0; i < 6; i++) {
			guessHistory[attempts][i] = guess[i];
		}
		// check guess
		match = checkGuess(guess, randEq[eqIndex]);
		// increments guess
		if (match==1) {
			lcdCommanda(0x01);
			_delay_ms(2);
			lcd_gotoxy(1,1);
			lcd_print(win);
			buzzer_win();
			gameOver = 1;
		} 
		else {
			lcdCommanda(0x01);
			_delay_ms(2);
			lcd_gotoxy(1,1);
			lcd_print(lose);
			// show correct equation (debug)
			lcd_gotoxy(1,2);
			for(int i = 0; i < 6; i++){
				lcdData(randEq[eqIndex][i]);
			}
			buzzer_error();
			attempts++;
			// wait for button click
			checkAnyKeyPressed();
			debounce();
			pressedKey = identifyPressedKey();
			// next guess
			lcdCommanda(0x01);
			_delay_ms(800);
			lcd_gotoxy(1, 1);
			lcd_print(next);
		}

		// out of attempts
		if (attempts >= 5 && !match){
			lcdCommanda(0x01);
			lcd_gotoxy(1, 1);
			lcd_print(correct);
			lcd_gotoxy(1, 2);
			for (int i = 0; i < 6; i++) {
				lcdData(randEq[eqIndex][i]);
			}
			buzzer_error();
			gameOver = 1;
		}
		// game over
		if (gameOver) {
			checkAnyKeyPressed();
			debounce();
			pressedKey = identifyPressedKey();
			// Reset GAME
			attempts = 0;
			gameOver = 0;
			// pick new equation
			eqIndex = (TCNT0^pressedKey) % NUM_EQUATIONS;
			lcdCommanda(0x01);
			_delay_ms(800);
		}
	}
	return 0;
}

// Checks user guess against the correct equation
int checkGuess(unsigned char *guess, unsigned char *equation) {
	// Returns 1 if the guess is correct, 0 otherwise
	int match = 1;
    for(int i = 0; i < 6; i++){
        if(guess[i] != equation[i]){
			match = 0;
			break;
        } 
    }
	return match;
}

// Used for randomness
void initTimer(){
	// Initialize Timer0 as free running timer
	TCNT0 = 0; // load timer0 = 0
	TCCR0A = 0; // Timer0: normal mode, internal clock
	// TCCR0B = 1; // Timer0: enabled, no prescaler
	TCCR0B = (1 << CS02) | (1 << CS00); // prescaler = 1024
}
