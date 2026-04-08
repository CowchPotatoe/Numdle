/*
 * CpE 301 - Numdle.c
 *
 * Created: 4/7/2026 7:47:00 PM
 * Author : ivanj
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include "keypad.h"

//Function prototype
void checkGuess(unsigned char *guess, unsigned char *equation);

// store user guess
unsigned char guess[6];
// store correct equation
unsigned char equation[6] = {'6','+','3','0','/','5'};
// number of attempts to guess equation
int numAttempts = 5;
// counts how many inputs
int inputNum = 0;

int main(void) {
	unsigned char pressedKey;
	unsigned char start1[] = "Numdle:";
	unsigned char start1[] = "Guess The Equation";
	unsigned char next[] = "Next Guess";
	
	initKeypadIO();
	lcd_init();
	lcd_gotoxy(1,1);
	lcd_print(start);
	
	while(1){
		//only allows 6 inputs
		if(inputNum<6){
			checkAnyKeyPressed();
			debounce();
			pressedKey = identifyPressedKey();
			// stores the key pressed
        	guess[inputNum] = pressedKey;
			// displays the char onto the lcd
			lcd_gotoxy(inputNum+1,2);
			lcdData(pressedKey);
			// update where the cursor is next
			inputNum++;
			_delay_ms(300);
			
			// if user has entered 6 characters, check guess
			if(inputNum == 6){
				checkGuess(guess, equation);
				_delay_ms(500);
				// reset inputNum for next guess
				inputNum = 0;
				lcdCommanda(0x01);
				_delay_ms(2);
				lcd_gotoxy(1,1);
				lcd_print(next);
			}
		}
	}
	return 0;
}

// Checks user guess against the correct equation
// Returns 1 if the guess is correct, 0 otherwise
void checkGuess(unsigned char *guess, unsigned char *equation) {
	unsigned char correct[] = "Correct";
	unsigned char incorrect[] = "Incorrect";
		
	int correct = 1;
    for(int i = 0; i < 6; i++){
        if(guess[i] != equation[i]){
			correct = 0;
			break;
        } 
    }
	if (correct == 0){
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		lcd_print(incorrect);  
	} else{
		lcdCommanda(0x01);
		_delay_ms(2);
		lcd_gotoxy(1,1);
		lcd_print(correct);  
	}
    
}