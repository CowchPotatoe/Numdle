// Ivan Chen and Isaac Chen
// 
//
// LCD Interface:
// GND = R/W 
// PC5 = RS
// PC4 = E
// PD7:0 = D7:0
// 
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

#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>

#define LCD_DPRT PORTD // LCD DATA PORT 
#define LCD_DDDR DDRD  // LCD DATA DDR 

#define LCD_CPRT PORTC // LCD CONTROL PORT 
#define LCD_CDDR DDRC  // LCD CONTOL DDR 

// LCD RW  (GND)
#define LCD_RS  5      // LCD RS on pC5
#define LCD_EN  4      // LCD EN on PC4

// Keypad access - allows changing the port easily
#define KEY_CDDR DDRC
#define KEY_RDDR DDRB
#define KEY_CPORT PORTC
#define KEY_RPORT PORTB
#define KEY_COLSREAD  PINC

// LCD functions
void lcdCommanda (unsigned char cmnd);
void lcdData(unsigned char data);
void lcd_init();
void lcd_gotoxy(unsigned char x, unsigned char y);  
void lcd_print(unsigned char * str);

// Keypad functions
void initKeypadIO();
void checkAnyKeyPressed();
void debounce();
unsigned char identifyPressedKey();
int inputNum = 0;

int main(void) {
	unsigned char pressedKey;
	unsigned char prompt1[] = "Enter Equation:";

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


/*********************************
 *********** FUNCTIONS ***********
 *********************************/

void lcdCommanda (unsigned char cmnd) {  
	LCD_DPRT = cmnd;				// Send cmnd to data port  
	
	LCD_CPRT &= ~(1 << LCD_RS);		// RS = 0 for command  
	//RW to GND
	//LCD_CPRT &= ~(1 << LCD_RW);   // RW = 0 for write   
	LCD_CPRT |=  (1 << LCD_EN);		// EN = 1 (E = EN) for H-to-L pulse  
	_delay_us(1);					// Wait to make enable wide  
	LCD_CPRT &= ~(1 << LCD_EN);		// EN = 0 for H-to_L pulse  
	_delay_us(100);					// Wait to make enable wide 
} 
 
void lcdData(unsigned char data) {  
	LCD_DPRT = data;				// Send data to data port 
	LCD_CPRT |=  (1 << LCD_RS);		// RS = 1 for data  
	//RW to GND
	//LCD_CPRT &= ~(1 << LCD_RW);		// RW = 0 for write   
	LCD_CPRT |=  (1 << LCD_EN);		// EN = 1 (E = EN) for H-to-L pulse  
	_delay_us(1);					// Wait to make enable wide  
	LCD_CPRT &= ~(1 << LCD_EN);		// EN = 0 for H-to_L pulse  
	_delay_us(100);					// Wait to make enable wide  
} 

void lcd_init() {  
	LCD_DDDR = 0xFF;  
	// LCD_CDDR = 0xFF;    
	// LCD_CPRT &= ~(1 << LCD_EN);		// LCD_EN = 0  
    LCD_CDDR |= (1<<LCD_RS)|(1<<LCD_EN); // RS and EN as output

	_delay_us(2000);    
	// Wait for init  
	lcdCommanda(0x38);				// Initialize LCD 2 line, 5x7 characters 
	lcdCommanda(0x0E);				// Display on, cursor on  
	lcdCommanda(0x01);				// Clear LCD  
	_delay_us(2000);				// Wait  
	lcdCommanda(0x06);				// Shift cursor right after display char.
} 
 
void lcd_gotoxy(unsigned char x, unsigned char y) {  
	unsigned char firstCharAdr[] = {0x80, 0xC0, 0x94, 0xD4};   
	
	lcdCommanda(firstCharAdr[y-1] + x -1);  
	_delay_us(100); 
} 
 
void lcd_print(unsigned char * str) {  
	unsigned char i = 0;  
	
	while (str[i]!=0)  {   
		lcdData(str[i]); 
		i++;  
	}
} 

//**************************************************************** 
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
	{'0', '1', '2', '3'},
	{'4', '5', '6', '7'},
	{'8', '9', 'A', 'B'},
	{'C', 'D', 'E', 'F'}};

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

