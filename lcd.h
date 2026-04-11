#ifndef LCD_H
#define LCD_H

#include <stdint.h>

// I2C LCD address (PCF8574 chip)
#define LCD_ADDR 0x27

// LCD Control bits (PCF8574 pins)
#define LCD_RS 0
#define LCD_RW 1
#define LCD_EN 2
#define LCD_BL 3   // backlight

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

// LCD high-level
void lcd_commanda(unsigned char cmd);
void lcd_data(uint8_t data);
void lcd_init(void);
void lcd_print(unsigned char *str);

#endif
