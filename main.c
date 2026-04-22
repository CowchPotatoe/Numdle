/*
 * CpE 301 - Numdle.c
 *
 * Created: 4/7/2026 7:47:00 PM
 * Author : ivanj
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "i2c.h"
#include "lcd.h"
#include "keypad.h"
#include "buzzer.h"

//Number of possible equations
#define NUM_EQUATIONS 5

//Function prototypes
void initTimer();
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

// main function
int main(void) {
	unsigned char start1[] = "Numdle1:";
	unsigned char start2[] = "GuessTheEquation";
	unsigned char randNum[] = "Number:";
	
	// initialize all hardware components
	initKeypadIO();
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
	// only receive 6 inputs
    while (i < 6) {
        checkAnyKeyPressed();
        debounce();
        pressedKey = identifyPressedKey();
		
		if (attempts == 0 && i == 0 && firstInput == 0) {
			// discard first input after intro screen
			firstInput = 1;
			continue;
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

        // store input
        guess[i] = pressedKey;

		// display input
        lcd_gotoxy(i + 4, 2);
        lcdData(pressedKey);
		// update index
        i++;
        _delay_ms(100);
    }
    // wait for '=' before continuing
    do {
        checkAnyKeyPressed();
        debounce();
        pressedKey = identifyPressedKey();
    } while (pressedKey != '=');
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
		checkAnyKeyPressed();
		debounce();
		pressedKey = identifyPressedKey();
		// clear screen and as for next guess
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		// show the number to quess
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
	checkAnyKeyPressed();
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
