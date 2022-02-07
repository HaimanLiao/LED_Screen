#include "gd32f10x_misc.h"              // GigaDevice::Device:GD32F10x_StdPeripherals:MISC
#include "gd32f10x_timer.h"             // GigaDevice::Device:GD32F10x_StdPeripherals:TIMER
#include "Timer.h"

static void(*Timer1_task)(void) = 0;		//Only can be Changed in TIMERx_Task_Init, which enables TIMERx_IRQn
static void(*Timer2_task)(void) = 0;		//Only can be Changed in TIMERx_Task_Init, which enables TIMERx_IRQn

static uint32_t TimerTick = 0;					//TickUnit = 0.1s ---> Capacity = 13 years

int TIMERx_Task_Init(uint32_t TIMERx, void (*task)(void), uint32_t Run_Period)
{
	static uint32_t Timer1_BSY = 0;
	static uint32_t Timer2_BSY = 0;
	
	timer_parameter_struct timer_initpara;
	rcu_periph_enum RCU_TIMERx;
	uint8_t TIMERx_IRQn;
	
	if(task == 0)
	{
		return -1;
	}
	
	if(Run_Period != 0)
	{
		Run_Period--;
	}
	
	switch (TIMERx)
	{
		case TIMER1:
			if(Timer1_BSY == 1)
			{
				return -1;
			}
			
			Timer1_BSY = 1;
			RCU_TIMERx = RCU_TIMER1;
			TIMERx_IRQn = TIMER1_IRQn;
			Timer1_task = task;
			break;
		case TIMER2:
			if(Timer2_BSY == 1)
			{
				return -1;
			}
			
			Timer2_BSY = 1;
			RCU_TIMERx = RCU_TIMER2;
			TIMERx_IRQn = TIMER2_IRQn;
			Timer2_task = task;
			break;
		default:
			break;
	}

	rcu_periph_clock_enable(RCU_TIMERx);
	timer_deinit(TIMERx);
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler         = 4799;
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = Run_Period;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_init(TIMERx, &timer_initpara);

	timer_interrupt_flag_clear(TIMERx, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMERx, TIMER_INT_UP);
	timer_enable(TIMERx);
	
	nvic_irq_enable(TIMERx_IRQn, 1, 1);
	
	return 0;
}

void TIMER1_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP))
		{
			timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
			
			Timer1_task();
    }
}

void TIMER2_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER2, TIMER_INT_FLAG_UP))
		{
			timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
			
			Timer2_task();
    }
}

void ClockTime_Init(void)					//TickUnit = 0.1s
{
	timer_parameter_struct timer_initpara;
	rcu_periph_clock_enable(RCU_TIMER3);
	timer_deinit(TIMER3);
	timer_struct_para_init(&timer_initpara);

	timer_initpara.prescaler         = 4799;
	timer_initpara.alignedmode       = TIMER_COUNTER_EDGE;
	timer_initpara.counterdirection  = TIMER_COUNTER_UP;
	timer_initpara.period            = 999;
	timer_initpara.clockdivision     = TIMER_CKDIV_DIV1;
	timer_init(TIMER3, &timer_initpara);

	timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMER3, TIMER_INT_UP);
	timer_enable(TIMER3);
	
	nvic_irq_enable(TIMER3_IRQn, 2, 1);
}

void timer_set(timer_t *t, uint32_t interval)
{
	t->start = TimerTick;
	t->interval = interval;
}

uint32_t timer_expired(timer_t *t)
{
	return ((TimerTick - (t->start)) > (t->interval) ? 1 : 0);
}

uint32_t clock_tick(void)
{
	return TimerTick;
}

void TIMER3_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP))
		{
			timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
			
			TimerTick++;
    }
}
