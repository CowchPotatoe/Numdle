#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "keypad.h"

void initKeypadIO() {
	KEY_RDDR |= 0x0F;	// PB3:0 = outputs (drive rows)
	KEY_CDDR &= 0xF0;	// PC3:0 = inputs (read columns)
	KEY_RPORT = 0x0F;	// Drive rows to 1
	KEY_CPORT = 0x0F;	// Set pull-up resistors on columns (PC3:0)
}

void checkAnyKeyPressed() {
	unsigned char anyKeyPressed;
	
	// Loop until a key is pressed
	do {
		// Ground all rows
		KEY_RPORT = 0xF0; // PB0-PB3 = 0, PB4-PB7 = 1 
		anyKeyPressed = KEY_COLSREAD & 0x0F;
	} while (anyKeyPressed == 0x0F);
}

void debounce() {
	// Wait and loop until get 2nd sample of key pressed
	do {
		_delay_ms(20);
	} while ( (KEY_COLSREAD & 0x0F) == 0x0F );
}

unsigned char identifyPressedKey() {
	unsigned char column, row; // Record column and row of pressed key
	unsigned char found = 0;   // Set to 1 when pressed key found
	
	// Array to hold keypad values
	unsigned char keypad[4][4] = {	
	{'7', '8', '9', '_'},
	{'4', '5', '6', '-'},
	{'1', '2', '3', '+'},
	{'/', '0', '*', '='}};

	// Check each row till find pressed key
	if (!found) {
		// Ground row 0
		KEY_RPORT = 0x0E;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		
		if (column != 0xF) {
			row = 0;
			found = 1;
		}		
	}
	
	if (!found) {
		// Ground row 1
		KEY_RPORT = 0x0D;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		
		if (column != 0xF) {
			row = 1;
			found = 1;
		}
	}

	if (!found) {
		// Ground row 2
		KEY_RPORT = 0x0B;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		if (column != 0xF) {
			row = 2;
			found = 1;
		}
	}

	if (!found) {
		// Ground row 3
		KEY_RPORT = 0x07;
		_delay_ms(20);
		column = KEY_COLSREAD & 0xF;
		if (column != 0xF) {
			row = 3;
			found = 1;
		}
	}

	// Decode column
	switch (column) {
		case 0x0E:		column = 0; break;
		case 0x0D:		column = 1; break;
		case 0x0B:		column = 2; break;
		case 0x07:		column = 3; break;
		default:		column = 0;
	}
	// Return value of key pressed
	return keypad[row][column];
}

