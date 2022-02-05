#ifndef		SYSTICK_H
#define		SYSTICK_H

/**********************************/
/*				Public DataType					*/
/**********************************/


/**********************************/
/*				Extern Variable					*/
/**********************************/


/**********************************/
/*				Public Function					*/
/**********************************/

void SysTick_Init(unsigned int ticks);
void SysTick_Handler(void);
void Delay_ms(unsigned int nTime);//nTime ms


#endif
