#ifndef TIMER_H
#define TIMER_H

#include "gd32f10x.h"                   // Device header

typedef struct
{
	uint32_t start;
	uint32_t interval;
}timer_t;

//Run_Period Unit = 0.1ms
int TIMERx_Task_Init(uint32_t TIMERx, void (*task)(void), uint32_t Run_Period);

void TIMER1_IRQHandler(void);
void TIMER2_IRQHandler(void);

//TimerTick Unit = 0.1s
void ClockTime_Init(void);

void TIMER3_IRQHandler(void);

void timer_set(timer_t *t, uint32_t interval);
uint32_t timer_expired(timer_t *t);
uint32_t clock_tick(void);

#endif
