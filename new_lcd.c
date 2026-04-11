#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

void i2c_init(void){
	TWSR = 0x00;
	TWBR = 72;          // ~100kHz for 16MHz
	TWCR = (1 << TWEN);
}

void i2c_start(void) {
	// Clear interrupt flag, enable TWI, enable start bit
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

void i2c_stop() {
	// Clear interrupt flag, enable TWI, enable stop condition
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
	_delay_us(10);
}

void i2c_write(unsigned char data) {
	// Set data register as data
	TWDR = data;
	// Clear interrupt flag, enable TWI
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
}

// LCD with I2C, PCF8574 chip
#define LCD_ADDR 0x27

#define RS 0 // LCD RS
#define RW 1 // LCD RW
#define E  2 // LCD EN

/*
https://www.mikrocontroller.net/attachment/521754/PCF8574_I2C_LCD_Cir_En.pdf
https://cdn.sparkfun.com/assets/9/5/f/7/b/HD44780.pdf
P0   -> RS
P1   -> RW
P2   -> EN
P3   -> BL-   # 1 for on, 0 for off
P4-7 -> DB7-4 # We must use 4-bit mode
*/

// I2C and LCD interfacer, required for every function
void lcd_expander_write(uint8_t data) {
	i2c_start();
	i2c_write(LCD_ADDR << 1);    // write mode
	i2c_write(data | (1 << 3));  // keep backlight on, PIN3
	i2c_stop();
}

// Pulse enable signal for writing
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
void lcd_command(uint8_t cmd) {
	lcd_send(cmd, 0);
}

// Data (characters)
void lcd_data(uint8_t data) {
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
	lcd_command(0x28);

	// Display ON, cursor OFF
	lcd_command(0x0C);

	// Clear display
	lcd_command(0x01);
	_delay_ms(2);

	// Entry mode
	lcd_command(0x06);
}

// Print string
void lcd_print(unsigned char * str) { 
	unsigned char i = 0; 
	while (str[i]!=0) { 
		lcd_data(str[i]); i++; 
	} 
}

int main(void) {
	i2c_init();
	lcd_init();
	unsigned char start2[] = "GuessTheEquation";
	lcd_print(start2);

	while (1) {
		// loop forever
	}
}
