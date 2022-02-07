#ifndef MATRIXDISPLAY_H
#define MATRIXDISPLAY_H

#include "gd32f10x.h"                   // Device header


/**********************************/
/*							Macro							*/
/**********************************/

/**********************************/
/*				Public DataType					*/
/**********************************/

/**********************************/
/*				Extern Variable					*/
/**********************************/

/**********************************/
/*				Public Function					*/
/**********************************/

int Matrix_Init(void);
void Display_OneLED(uint32_t matrix_idx, uint32_t value);

//Matrix: 1 ~ 6 rows
void Display_SOC(uint32_t soc);				//soc = 0 ~ 100
void Display_Kwh(uint32_t mKwh);			//Unit = 0.1Kwh
void Display_GunId(uint32_t GUN_X);
void Display_Err(uint32_t GUN_X);
//Matrix: 7 ~ 8 rows
void Display_RoundBar1(uint32_t period);												//Blocking Code
void Display_RoundBar2(uint32_t soc, uint32_t period);					//Blocking Code ( n * period ms)

void MatrixOff(void);

#endif
