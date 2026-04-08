// Board Row[1:4] = PB0:3 = Row 0:3
// Row[1] = PB0 = Row 0
// Row[2] = PB1 = Row 1
// Row[3] = PB2 = Row 2
// Row[4] = PB3 = Row 3
//
// Board Col[1:4] = PB0:3 = Col 0:3
// Col[4] = PC3 = Col 3
// Col[3] = PC2 = Col 2
// Col[2] = PC1 = Col 1
// Col[1] = PC0 = Col 0

#ifndef KEYPAD_H
#define KEYPAD_H

#include <avr/io.h>

// Keypad macros
#define KEY_CDDR DDRC
#define KEY_RDDR DDRB
#define KEY_CPORT PORTC
#define KEY_RPORT PORTB
#define KEY_COLSREAD PINC

void initKeypadIO();
void checkAnyKeyPressed();
void debounce();
unsigned char identifyPressedKey();

#endif