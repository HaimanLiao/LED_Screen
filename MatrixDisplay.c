#include "gd32f10x_gpio.h"              // GigaDevice::Device:GD32F10x_StdPeripherals:GPIO
#include "SysTick.h"
#include "Timer.h"
#include "MatrixDisplay.h"
#include "GunStaDisplay.h"

/*1) Matrix_Flush task : Flush matirx display buffer with fixed period*/
/*2) BlinkLED task     : Blink one LED of the matrix with fixed period*/

/**********************************/
/*							Macro							*/
/**********************************/
//SOC and Gun Choice (DIG1~3_H)
#define DIG1					0
#define DIG2					1
#define DIG3					2
//Kwh
#define DIG4					3
#define DIG5					4
#define DIG6					5

/**********************************/
/*				Private Variable				*/
/**********************************/
static uint8_t Display_Data[8][8] = {0};			//X1 ~ X8
/*
(+5V)  A      B      C      D      E      F      G      H
PB10___ ______ ______ ______ ______ ______ ______ ______         X1(0 ~ 7)
			 |      |      |      |      |      |      |      |
PB2 ___|______|______|______|______|______|______|______|        X2(8 ~ 15)_____
			 |      |      |      |      |      |      |      |												|--->ErrGND
PB1 ___|______|______|______|______|______|______|______|        X3(16 ~ 23)____|
			 |      |      |      |      |      |      |      |
PB0 ___|______|______|______|______|______|______|______|        X4(24 ~ 31)
			 |      |      |      |      |      |      |      |
PA7 ___|______|______|______|______|______|______|______|        X5(32 ~ 39)
			 |      |      |      |      |      |      |      |
PA6 ___|______|______|______|______|______|______|______|        X6(40 ~ 47)
       |      |      |      |      |     PB4    PB3   PA15(GND)
PB15___|______|______|______|______|                             X7(48 ~ 52): Circle_Left
       |      |      |      |      |
PB11___|______|______|______|______|                             X8(56 ~ 60): Circle_Right
			PB9    PB8    PB7    PB6    PB5
		 
		 Set VCC and GND(PIN6 ~ PIN13) ----- Display One DIG(A ~ G) and One H
     PINx Set----- +5V or GND (LED On)
		 
Error GND:
		 PB2 and PB1 Set + PB14(OUT_STOP) Set
*/

static uint8_t Num_Segment[10][7] = {
// A  B  C  D  E  F  G
	{1, 1, 1, 1, 1, 1, 0},	//0
	{0, 1, 1, 0, 0, 0, 0},	//1
	{1, 1, 0, 1, 1, 0, 1},	//2
	{1, 1, 1, 1, 0, 0, 1},	//3
	{0, 1, 1, 0, 0, 1, 1},	//4
	{1, 0, 1, 1, 0, 1, 1},	//5
	{1, 0, 1, 1, 1, 1, 1},	//6
	{1, 1, 1, 0, 0, 0, 0},	//7
	{1, 1, 1, 1, 1, 1, 1},	//8
	{1, 1, 1, 1, 0, 1, 1},	//9
};

typedef struct
{
	uint32_t Port_Num;
	uint32_t Pin_num;
}PIN_CTRL;
PIN_CTRL VCC_Pin[8] = {
	{GPIOB, GPIO_PIN_10},
	{GPIOB, GPIO_PIN_2},
	{GPIOB, GPIO_PIN_1},
	{GPIOB, GPIO_PIN_0},
	{GPIOA, GPIO_PIN_7},
	{GPIOA, GPIO_PIN_6},
	{GPIOB, GPIO_PIN_15},
	{GPIOB, GPIO_PIN_11}
};
PIN_CTRL GND_Pin[8] = {
	{GPIOB, GPIO_PIN_9},
	{GPIOB, GPIO_PIN_8},
	{GPIOB, GPIO_PIN_7},
	{GPIOB, GPIO_PIN_6},
	{GPIOB, GPIO_PIN_5},
	{GPIOB, GPIO_PIN_4},
	{GPIOB, GPIO_PIN_3},
	{GPIOA, GPIO_PIN_15}
};

/**********************************/
/*				Private Function				*/
/**********************************/
static void Matrix_Flush(void)		//Display Matrix Elements One by One
{
	static uint32_t Last_VCC_PIN = 0;
	static uint32_t Current_VCC_PIN = 0;			//0 ~ 7 (X1 ~ X8)
	bit_status gpio_value;
	uint32_t i;
	
	Last_VCC_PIN = Current_VCC_PIN;
	Current_VCC_PIN = (Current_VCC_PIN == 7) ? 1 : (Last_VCC_PIN + 1);
	
	//VCC Pin Control
	gpio_bit_reset(VCC_Pin[Last_VCC_PIN].Port_Num, VCC_Pin[Last_VCC_PIN].Pin_num);
	gpio_bit_set(VCC_Pin[Current_VCC_PIN].Port_Num, VCC_Pin[Current_VCC_PIN].Pin_num);
	
	//GND Pin Control
	for(i = 0; i < 8; i++)
	{
		gpio_value = (Display_Data[Current_VCC_PIN][i] == 0) ? RESET : SET;
		gpio_bit_write(GND_Pin[i].Port_Num, GND_Pin[i].Pin_num, gpio_value);
	}
}

static void Display_DigNum(uint32_t DIG_Pos, uint32_t num)
{
	uint32_t i;
	
	if((DIG_Pos > 5) || (num > 7))return;
	
	for(i = 0; i < 7; i++)
	{
		Display_Data[DIG_Pos][i] = Num_Segment[num][i];
	}
}

static void Display_DigH(uint32_t DIG_Pos, uint32_t DigH_Value)
{
	if(DIG_Pos > 7)return;
	
	Display_Data[DIG_Pos][7] = ((DigH_Value == 0) ? 0 : 1);
}

static void ErrGND_Off(void)
{
	gpio_bit_reset(GPIOB, GPIO_PIN_14);
}

static void ErrGND_On(void)
{
	gpio_bit_set(GPIOB, GPIO_PIN_14);
}

/**********************************/
/*				Public Function					*/
/**********************************/
int Matrix_Init(void)			//GPIO Default Output Value = RESET
{
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOA);
	
	//+5V
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_10);		//Common +5V
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_2);			//Common +5V and "Error +5V"
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_1);			//Common +5V and "Error +5V"
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_0);			//Common +5V
	gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_7);			//Common +5V
	gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_6);			//Common +5V
	
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_15);		//Common +5V_Circle_L
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_11);		//Common +5V_Circle_R

	//GND
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_9);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_8);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_7);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_6);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_5);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_4);
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_3);
	gpio_init(GPIOA,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_15);
	
	//"Error GND"
	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_14);
	
	Delay_ms(10);
	
	if(TIMERx_Task_Init(TIMER1, Matrix_Flush, 10000) == -1)		//10000 * 0.1ms = 1s
	{
		return -1;
	}
	
	return 0;
}

void Display_SOC(uint32_t soc)				//Unit = 1 (soc = 0 ~ 100)
{
	uint32_t One;
	uint32_t Ten;
	uint32_t Hundred;
	
	if(soc > 100)return;
	ErrGND_Off();
	
	Hundred = (uint32_t)(soc / 100);
	Ten = (uint32_t)((soc - Hundred * 100) / 10);
	One = (uint32_t)(soc - Hundred * 100 - Ten * 10);
	
	Display_DigNum(DIG1, Hundred);
	Display_DigNum(DIG2, Ten);
	Display_DigNum(DIG3, One);
}

void Display_Kwh(uint32_t mKwh)			  //Unit = 0.1Kwh
{
	uint32_t One;
	uint32_t Ten;
	uint32_t Hundred;
	
	if(mKwh > 999)return;								//Max Display: 99.9Kwh
	ErrGND_Off();
	
	Hundred = (uint32_t)(mKwh / 100);
	Ten = (uint32_t)((mKwh - Hundred * 100) / 10);
	One = (uint32_t)(mKwh - Hundred * 100 - Ten * 10);
	
	Display_DigNum(DIG4, Hundred);
	Display_DigNum(DIG5, Ten);
	Display_DigH(DIG5, 1);		//Decimal Point
	Display_DigNum(DIG6, One);
	Display_DigH(DIG6, 1);		//Kwh
}

void Display_GunId(uint32_t GUN_X)
{
	ErrGND_Off();
	
	if(GUN_X == GUN_A)
	{
		Display_DigH(DIG1, 1);
		Display_DigH(DIG2, 0);
		Display_DigH(DIG3, 0);
	}
	else if(GUN_X == GUN_B)
	{
		Display_DigH(DIG1, 0);
		Display_DigH(DIG2, 1);
		Display_DigH(DIG3, 0);
	}
	else if(GUN_X == GUN_A)
	{
		Display_DigH(DIG1, 0);
		Display_DigH(DIG2, 0);
		Display_DigH(DIG3, 1);
	}
}

void Display_OneLED(uint32_t matrix_idx, uint32_t value)		//0---Off, Other---On
{
	uint32_t row;
	uint32_t column;
	
	ErrGND_Off();
	
	row = (uint32_t)(matrix_idx / 8);
	column = matrix_idx % 8;
	value = (value == 0) ? 0 : 1;
	
	Display_Data[row][column] = value;
}

void Display_RoundBar1(uint32_t period)
{
	uint32_t row;
	uint32_t column;

	ErrGND_Off();
	
	for(row = 6; row < 8; row++)
	{
		for(column = 0; column < 5; column++)
		{
			Display_Data[row][column] = 1;
			Delay_ms(period);
		}
	}

	//Reverse
	for(row = 7; row > 5; row--)								//7, 6
	{
		for(column = 5; column > 0; column--)
		{
			Display_Data[row][column - 1] = 0;			//4, 3, 2, 1, 0
			Delay_ms(period);
		}
	}
}

void Display_RoundBar2(uint32_t soc, uint32_t period)
{
	uint32_t row;
	uint32_t column;
	uint32_t tmp;
	
	ErrGND_Off();
	
	if(soc < 60)
	{
		row = 6;
		tmp = (uint32_t)(soc / 10);
		for(column = 0; column < tmp; column++)
		{
			Display_Data[row][column] = 1;
			Delay_ms(period);
		}
	}
	else if(soc < 100)
	{
		row = 6;
		for(column = 0; column < 5; column++)
		{
			Display_Data[row][column] = 1;
			Delay_ms(period);
		}
		
		row = 7;
		tmp = (uint32_t)((soc - 50) / 10);
		for(column = 0; column < tmp; column++)
		{
			Display_Data[row][column] = 1;
			Delay_ms(period);
		}
	}
	else	//soc = 100
	{
		for(row = 6; row < 8; row++)
		{
			for(column = 0; column < 5; column++)
			{
				Display_Data[row][column] = 1;
				Delay_ms(period);
			}
		}
	}
}

void MatrixOff(void)				//Matrix_Flush still Runs (VCC_Pins Still set in turns + All GND_Pins = 0---PIN Floating)
{
	uint32_t i, j;
	
	for(i = 0; i < 8; i++)
	{
		for(j = 0; j < 8; j++)
		{
			Display_Data[i][j] = 0;
		}
	}
}

void Display_Err(uint32_t GUN_X)			//Matrix_Flush still Runs
{
	MatrixOff();							//VCC_Pins Still set in turns
	
	Display_GunId(GUN_X);
	ErrGND_On();							//GND_Pins are Floating + ErrGND Pin Connect GND
}
