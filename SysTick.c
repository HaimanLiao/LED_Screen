#include "gd32f10x.h"                   // Device header
#include "SysTick.h"

/**********************************/
/*							Macro							*/
/**********************************/


/**********************************/
/*				Private Variable				*/
/**********************************/

static volatile unsigned int TimeDelay = 0;

/**********************************/
/*				Public Function					*/
/**********************************/

void SysTick_Init(unsigned int ticks)
{
	SysTick->CTRL = 0;																						//Disable SysTick
	
	SysTick->LOAD = ticks - 1;																		//Set reload register
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);	//Least urgency (largest priority value)
	SysTick->VAL = 0;																							//Reset the SysTick counter value
	SysTick->CTRL |= 0x04;																				//Select Clock Source (AHB Clock}
	SysTick->CTRL |= 0x02;																				//SysTick Interrupt Enable
	
	SysTick->CTRL |= 0x01;																				//SysTick Enable
	
	NVIC_EnableIRQ(SysTick_IRQn);
}

void SysTick_Handler(void)
{
	if(TimeDelay > 0)TimeDelay--;
}

//     ticks
// |__________|
//         ^
//         |
//         |
//      Set Delay
void Delay_ms(unsigned int nTime)//Can't Delay Accurately (Bias = 0 ~ ticks)
{
	TimeDelay = nTime;
	while(TimeDelay != 0);
}
