/*
 * CpE 301 - Numdle.c
 *
 * Created: 4/7/2026 7:47:00 PM
 * Author : ivanj
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "i2c.h"
#include "lcd.h"
#include "keypad.h"
#include "buzzer.h"

//Rotary encoder PINS
#define ENC_CLK PD6
#define ENC_DT PD7

//Number of possible equations
#define NUM_EQUATIONS 5

//Function prototypes
void initTimer();
void initEncoder();
void scrollHistory(int direction);
void restoreLCD();
void getUserInput();
void storeGuess();
void result();
void gamePlay();
void resetGame();
int checkGuess(unsigned char *guess, unsigned char *equation);

/* Global Variables*/

// store current user guess
unsigned char guess[6];

// predefined equations
unsigned char randEq[NUM_EQUATIONS][6] = {
    {'6','+','3','0','/','5'}, // 12
    {'5','*','3','-','1','0'}, // 5
    {'1','2','-','5','*','9'}, // -33
    {'9','/','3','*','2','4'}, // 72
    {'7','*','8','-','3','6'}  // 20
};

// used for string conversion
char answerStr[5];
// corresponding answers
int answers[NUM_EQUATIONS] = {12, 5, -33, 72, 20};
// store previous guesses
unsigned char guessHistory[NUM_EQUATIONS][6];
// number of guesses left
int attempts = 0;
// bool to indicate if the game is over
int gameOver = 0;
// bool to see result of comparison
int match = 0;
// index of selected equation
int eqIndex = 0;
// stores last key press
unsigned char pressedKey;

// Encoder variables
volatile unsigned char lastCLK = 1;
volatile int historyViewIndex = 0;
volatile int wasScrolling = 0;

// main function
int main(void) {
	unsigned char start1[] = "Numdle1:";
	unsigned char start2[] = "GuessTheEquation";
	unsigned char randNum[] = "Number:";
	
	// initialize all hardware components
	initKeypadIO();
	initEncoder();
	i2c_init();
	lcd_init();
	buzzer_init();
	initTimer();

	// display starting text
	lcd_gotoxy(1,1);
	lcd_print(start1);
	lcd_gotoxy(1,2);
	lcd_print(start2);	

	// continue after user input
	checkAnyKeyPressed();
	debounce();
	pressedKey = identifyPressedKey();

	// display game prompt
	lcdCommanda(0x01);
	_delay_ms(2);
	lcd_gotoxy(1,1);
	lcd_print(randNum);
	
    lcd_gotoxy(1,2);
    // Print attempt number
    lcdData((attempts + 1) + '0');
    lcdData('.');
    lcdData(' ');
	
	// Select random equation using timer
	eqIndex = TCNT0 % NUM_EQUATIONS;

	// convert answer to string
	itoa(answers[eqIndex], answerStr, 10);

	// display answer on LCD
	lcd_gotoxy(9,1); // position after "Number:"
	lcd_print((unsigned char*)answerStr);

	while(1){
		// get equation from keypad
		getUserInput();
		// save guess and check correctness
		storeGuess();
		// display result of guess
		result();
		// handle game progression
		gamePlay();
	}
	return 0;
}

// takes keypad as user input
void getUserInput(void) {
    int i = 0;
	int firstInput = 0;
	// only receive 6 inputs or 7 if it is a backspace
    while (i < 7) {
		// check is a blocking function that ret 1 if scrolling
        if (checkAnyKeyPressed() == -1) {
			// Restore the LCD after we are done scrolling
			restoreLCD();
			continue;
		}
        debounce();
        pressedKey = identifyPressedKey();
		// debounce, ignore input if fail
		if (pressedKey == 1) continue;
		
		if (attempts == 0 && i == 0 && firstInput == 0) {
			// discard first input after intro screen
			firstInput = 1;
			continue;
		}
		
		if (pressedKey == '=') {
			break;
		}
		
        // backspace handling
        if (pressedKey == '_') {
            if (i > 0) {
                i--; // move back in array
                // erase from LCD
                lcd_gotoxy(i + 4, 2);
                lcdData(' ');
                // move cursor back again
                lcd_gotoxy(i + 4, 2);
				_delay_ms(200);
            }
            continue;
        }
		
		// Don't store any inputs beyond 6 chars
		if (i < 6) {
			// store input
			guess[i] = pressedKey;

			// display input
			lcd_gotoxy(i + 4, 2);
			lcdData(pressedKey);
			// update index
			i++;
			_delay_ms(100);
		}

    }
}

// stores the guess for each attempt
void storeGuess() {
	// stores guess into history
	for (int i = 0; i < 6; i++) {
		guessHistory[attempts][i] = guess[i];
	}
	// compare guess with selected equation
	match = checkGuess(guess, randEq[eqIndex]);
}

// displays win/lose message after each guess
void result() {
	unsigned char num[] = "Number: ";
	unsigned char win[] = "YOU WIN!!!";
	unsigned char lose[] = "Incorrect";

	// if correct guess
	if (match == 1) {
		// display win message
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		lcd_print(win);
		buzzer_win();
		// game over
		gameOver = 1;
		return;
	}
	// Wrong guess
	if (attempts < 5) {
		// increment attempts
		attempts++;
	}
	// clear screen and display lose message
	lcdCommanda(0x01);
	_delay_ms(2);
	lcd_gotoxy(1,1);
	lcd_print(lose);

	// debug purpose, show current equation
	lcd_gotoxy(1,2);
	for (int i = 0; i < 6; i++) {
		lcdData(randEq[eqIndex][i]);
	}

	buzzer_error();
	// Show next prompt if player still has attempts left
	if (attempts < 5) {
		// wait for button press
		if (checkAnyKeyPressed() == -1) {
			restoreLCD();
		}
		debounce();
		pressedKey = identifyPressedKey();
		// clear screen and as for next guess
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		// show the number to guess
		lcd_print(num);
		// convert answer to string
		itoa(answers[eqIndex], answerStr, 10);
		// display answer on LCD
		lcd_gotoxy(9,1); // position after "Number:"
		lcd_print((unsigned char*)answerStr);
		_delay_ms(200);
		
		lcd_gotoxy(1,2);
		// Print attempt number
		lcdData((attempts + 1) + '0');
		lcdData('.');
		lcdData(' ');
	}
}

// handle losing condition and game reset
void gamePlay() {
	unsigned char correct[] = "Correct Eq:";
	// wait for user input
	if (checkAnyKeyPressed() == -1) restoreLCD();
	debounce();
	pressedKey = identifyPressedKey();
	// if player used all attempts and failed
	if (attempts >= 5 && match == 0) {
		// display the correct equation
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		lcd_print(correct);
		lcd_gotoxy(1,2);
		for (int i = 0; i < 6; i++) {
			lcdData(randEq[eqIndex][i]);
		}
		buzzer_error();
		gameOver = 1;
	}
	// if game ended, wait and reset
	if (gameOver) {
		checkAnyKeyPressed();
		debounce();
		pressedKey = identifyPressedKey();
		resetGame();
	}
}

// resets the game and select new equation
void resetGame() {
	unsigned char newNum[] = "Number:";
	attempts = 0;
	gameOver = 0;
	eqIndex = TCNT0 % NUM_EQUATIONS;
	lcdCommanda(0x01);
	lcd_gotoxy(1,1);
	lcd_print(newNum);

    // convert answer to string
	itoa(answers[eqIndex], answerStr, 10);

	// display answer on LCD
	lcd_gotoxy(9,1); // position after "Number:"
	lcd_print((unsigned char*)answerStr);

	_delay_ms(800);
	
	lcd_gotoxy(1,2);
	// Print attempt number
	lcdData((attempts + 1) + '0');
	lcdData('.');
	lcdData(' ');
	
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
	TCCR0B = (1 << CS00); // no prescaler
}

// Rotary interrupt, once we start scrolling, start showing history
ISR(PCINT2_vect) {
	// The two orthogonal signals
	unsigned char clk = (PIND >> ENC_CLK) & 1;
	unsigned char dt = (PIND >> ENC_DT) & 1;
	static int clicker = 0;
	
	// Only check when CLK changed from the last one
	if (clk != lastCLK) {
		// Check falling edge
		if (clk == 0) {
			// Clockwise, CLK is 0 and DT is 1
			if (dt != clk) {
				clicker++;
			} else {
				clicker--;
			}
			
			// Scroll on 5 rotations
			if (clicker >= 5) {
				scrollHistory(1);
				clicker = 0;
			} else if (clicker <= -5) {
				scrollHistory(-1);
				clicker = 0;
			}
		}
		// Save the last CLK for next ISR
		lastCLK = clk;
	}
}

// For actually scrolling through history
void scrollHistory(int dir) {
	// No attempt so far, exit
	if (attempts == 0) {
		return;
	}
	
	// Update index in accordance to scroll direction
	historyViewIndex += dir;
	// Cannot scroll past initial attempt
	if (historyViewIndex < 0) {
		historyViewIndex = 0;
	}
	// Cannot scroll past last attempt
	if (historyViewIndex >= attempts) {
		historyViewIndex = attempts - 1;
	}
	
	unsigned char histLabel[] = "History:";
	
	// Clear screen and show history:
	lcdCommanda(0x01);
	_delay_ms(2);
	lcd_gotoxy(1,1);
	lcd_print(histLabel);
	
	lcd_gotoxy(1,2);
	for (int i = 0; i < 6; i++) {
		// Print each char of prev guess
		lcdData(guessHistory[historyViewIndex][i]);
	}
	
	// Print current/total prev guesses
	char indexStr[4];
	itoa(historyViewIndex +1, indexStr, 10);
	lcd_gotoxy(11, 1);
	lcd_print((unsigned char*)indexStr);
	lcd_gotoxy(12, 1);
	lcdData('/');
	lcd_gotoxy(13, 1);
	itoa(attempts, indexStr, 10);
	lcd_print((unsigned char*)indexStr);
	
	// Set scrolling to 1 so we know to restore
	wasScrolling = 1;
}

void initEncoder() {
	// inputs on PORTD for encoder
    DDRD  &= ~((1 << ENC_CLK) | (1 << ENC_DT));
	// pullup resistors
    PORTD |=  ((1 << ENC_CLK) | (1 << ENC_DT));  

	// PCINT for PORTD pins
    PCICR  |= (1 << PCIE2);
	// Which pins to observe on PORTD (PD6 and PD7)
    PCMSK2 |= (1 << PCINT22) | (1 << PCINT23);

	// Enable interrupts
    sei();
}

void restoreLCD() {
	unsigned char num[] = "Number: ";
	lcdCommanda(0x01);
	_delay_ms(2);
	lcd_gotoxy(1,1);
	lcd_print(num);

	lcd_gotoxy(9, 1);
	// show the number to guess
	// convert answer to string
	itoa(answers[eqIndex], answerStr, 10);

	// display answer on LCD
	lcd_gotoxy(9,1); // position after "Number:"
	lcd_print((unsigned char*)answerStr);
	
	lcd_gotoxy(1,2);
	// Print attempt number
	lcdData((attempts + 1) + '0');
	lcdData('.');
	lcdData(' ');
	

}
