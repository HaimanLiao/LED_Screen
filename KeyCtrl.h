#ifndef KEYCTRL_H
#define KEYCTRL_H
#include "gd32f10x.h"                   // Device header

#define SELECT_A	0
#define SELECT_B	0
#define SELECT_C	0

void KeyIRQ_Disable(void);
void KeyIRQ_Enable(void);
void KeyLED_Display(void);

#endif
