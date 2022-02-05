#ifndef TIMER_TASK
#define TIMER_TASK

#include "gd32f10x.h"                   // Device header

//Run_Period Unit = 0.1ms
void TIMERx_Task_Init(uint32_t TIMERx, void (*task)(void), uint32_t Run_Period);

void TIMER1_IRQHandler(void);
void TIMER2_IRQHandler(void);
void TIMER3_IRQHandler(void);

#endif
