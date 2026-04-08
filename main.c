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

int inputNum = 0;

int main(void) {
	unsigned char pressedKey;
	unsigned char prompt1[] = "The Equation:";

	initKeypadIO();
	lcd_init();
	lcd_gotoxy(1,1);
	lcd_print(prompt1);
	
	while(1){
		checkAnyKeyPressed();
		debounce();
		pressedKey = identifyPressedKey();
		// displays the char onto the lcd
		lcd_gotoxy(inputNum+1,2);
		lcdData(pressedKey);
		// update where the cursor is next
		inputNum++;
		_delay_ms(300);
	}
	
	return 0;
}
