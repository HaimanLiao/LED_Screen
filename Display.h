#ifndef DISPLAY_H
#define DISPLAY_H

#include "gd32f10x.h"                   // Device header


/**********************************/
/*							Macro							*/
/**********************************/

#define GUN_A					0
#define GUN_B					1
#define GUN_C					2

/**********************************/
/*				Public DataType					*/
/**********************************/

/**********************************/
/*				Extern Variable					*/
/**********************************/

/**********************************/
/*				Public Function					*/
/**********************************/

void Matrix_Init(void);
void Matrix_Flush(void);							//Key Function

//Matrix: 1 ~ 6
void Display_SOC(uint32_t soc);				//soc = 0 ~ 100
void Display_Kwh(uint32_t mKwh);			//Unit = 0.1Kwh
void Display_GunId(uint32_t GUN_X, uint8_t OnOff);

//Matrix: 7 ~ 8
void Display_ChgBar(uint32_t soc);		//soc = 0 ~ 100

#endif
