#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "display.h"

// Initialize the i2c protocol
void i2c_init(void){
	TWSR = 0x00;
	TWBR = 72;          // ~100kHz for 16MHz
	TWCR = (1 << TWEN);
}

// Start i2c communication
void i2c_start(void) {
	// Clear interrupt flag, enable TWI, enable start bit
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// Stop i2c communication
void i2c_stop() {
	// Clear interrupt flag, enable TWI, enable stop condition
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	_delay_us(10);
}

// Write
void i2c_write(unsigned char data) {
	// Set data register as data
	TWDR = data;
	// Clear interrupt flag, enable TWI
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// I2C and LCD interfacer, required for every function
void lcd_expander_write(uint8_t data) {
	i2c_start();
	i2c_write(LCD_ADDR << 1);    // write mode
	i2c_write(data | (1 << 3));  // keep backlight on, PIN3
	i2c_stop();
}

// Pulse enable signal for basically everything
void lcd_pulse(uint8_t data) {
	lcd_expander_write(data | (1 << E));
	_delay_us(1);
	lcd_expander_write(data & ~(1 << E));
	_delay_us(50);
}

// Send 4 bits since we are in 4 bit mode
void lcd_write4(uint8_t nibble, uint8_t mode) {
	uint8_t data = (nibble << 4);

	if (mode) {
		data |= (1 << RS);  // RS = data
		} else {
		data &= ~(1 << RS); // RS = command
	}

	lcd_pulse(data);
}

// Send full byte
void lcd_send(uint8_t value, uint8_t mode) {
	lcd_write4(value >> 4, mode);   // high nibble
	lcd_write4(value & 0x0F, mode); // low nibble
}

// Commands
void lcdCommanda(unsigned char cmd) {
	lcd_send(cmd, 0);
}

// Data (characters)
void lcdData(unsigned char data) {
	lcd_send(data, 1);
}

// Initialize LCD
void lcd_init() {
	_delay_ms(50);

	// Force 4-bit mode
	lcd_write4(0x03, 0);
	_delay_ms(5);
	lcd_write4(0x03, 0);
	_delay_us(150);
	lcd_write4(0x03, 0);
	lcd_write4(0x02, 0);

	// 4-bit, 2 lines, 5x8 font
	lcdCommanda(0x28);

	// Display ON, cursor OFF
	lcdCommanda(0x0C);

	// Clear display
	lcdCommanda(0x01);
	_delay_ms(2);

	// Entry mode
	lcdCommanda(0x06);
}

// Print string
void lcd_print(unsigned char * str) {
	unsigned char i = 0;
	while (str[i]!=0) {
		lcdData(str[i]); i++;
	}
}

// Function for changing position on the LCD
void lcd_gotoxy(unsigned char x, unsigned char y) {
	if (y < 1 || y > 4) return;

	unsigned char firstCharAdr[] = {0x80, 0xC0, 0x94, 0xD4};
	lcdCommanda(firstCharAdr[y - 1] + x - 1);
}