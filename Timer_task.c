#include "gd32f10x_misc.h"              // GigaDevice::Device:GD32F10x_StdPeripherals:MISC
#include "gd32f10x_timer.h"             // GigaDevice::Device:GD32F10x_StdPeripherals:TIMER
#include "Timer_task.h"

static void(*Timer1_task)(void) = 0;		//Only can be Changed in TIMERx_Task_Init, which enables TIMERx_IRQn
static void(*Timer2_task)(void) = 0;		//Only can be Changed in TIMERx_Task_Init, which enables TIMERx_IRQn
static void(*Timer3_task)(void) = 0;		//Only can be Changed in TIMERx_Task_Init, which enables TIMERx_IRQn

void TIMERx_Task_Init(uint32_t TIMERx, void (*task)(void), uint32_t Run_Period)
{
	timer_parameter_struct timer_initpara;
	rcu_periph_enum RCU_TIMERx;
	uint8_t TIMERx_IRQn;
	
	if(task == 0)
	{
		return;
	}
	
	if(Run_Period != 0)
	{
		Run_Period--;
	}
	
	switch (TIMERx)
	{
		case TIMER1:
			RCU_TIMERx = RCU_TIMER1;
			TIMERx_IRQn = TIMER1_IRQn;
			Timer1_task = task;
			break;
		case TIMER2:
			RCU_TIMERx = RCU_TIMER2;
			TIMERx_IRQn = TIMER2_IRQn;
			Timer2_task = task;
			break;
		case TIMER3:
			RCU_TIMERx = RCU_TIMER3;
			TIMERx_IRQn = TIMER3_IRQn;
			Timer3_task = task;
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

void TIMER3_IRQHandler(void)
{
    if(SET == timer_interrupt_flag_get(TIMER3, TIMER_INT_FLAG_UP))
		{
			timer_interrupt_flag_clear(TIMER3, TIMER_INT_FLAG_UP);
			
			Timer3_task();
    }
}
