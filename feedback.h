#ifndef FEEDBACK_H
#define FEEDBACK_H

#include <avr/io.h>
#include <stdint.h>

// LED pins
#define LED_RED  PD4
#define LED_BLUE PD5

void feedback_init();
void feedback_tone(unsigned int freq);
void feedback_off();
void feedback_error();
void feedback_win();

#endif