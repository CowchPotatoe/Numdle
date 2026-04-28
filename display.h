#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <stdint.h>

/*
https://www.mikrocontroller.net/attachment/521754/PCF8574_I2C_LCD_Cir_En.pdf
https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf
P0   -> RS
P1   -> RW
P2   -> EN
P3   -> BL-   # 1 for on, 0 for off
P4-7 -> DB7-4 # We must use 4-bit mode
*/

// I2C LCD address (PCF8574 chip)
#define LCD_ADDR 0x27

// LCD Control bits (PCF8574 pins)
#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2
#define LCD_BL 3   // backlight

#define RS 0 // LCD RS
#define RW 1 // LCD RW
#define E  2 // LCD EN

// I2C functions
void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_write(uint8_t data);

// LCD helper functions
void lcd_expander_write(uint8_t data);
void lcd_pulse(uint8_t data);
void lcd_write4(uint8_t nibble, uint8_t mode);
void lcd_send(uint8_t value, uint8_t mode);

// LCD high-level functions
void lcdCommanda(unsigned char cmd);
void lcdData(unsigned char data);
void lcd_init(void);
void lcd_print(unsigned char *str);
void lcd_gotoxy(unsigned char x, unsigned char y);

#endif