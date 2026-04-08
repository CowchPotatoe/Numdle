// LCD Interface:
// GND = R/W 
// PC5 = RS
// PC4 = E
// PD7:0 = D7:0

#ifndef LCD_H
#define LCD_H

#include <avr/io.h>

// LCD ports
#define LCD_DPRT PORTD
#define LCD_DDDR DDRD

#define LCD_CPRT PORTC
#define LCD_CDDR DDRC

#define LCD_RS 5
#define LCD_EN 4

void lcdCommanda(unsigned char cmnd);
void lcdData(unsigned char data);
void lcd_init();
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print(unsigned char * str);

#endif