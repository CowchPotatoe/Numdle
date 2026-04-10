#ifndef BUZZER_H
#define BUZZER_H

#include <avr/io.h>

// Buzzer Macros
#define BUZZER_DDR  DDRB
#define BUZZER_PORT PORTB
#define BUZZER_PIN  PB5

void buzzer_init();
void buzzer_on();
void buzzer_off();
void buzzer_error();

#endif