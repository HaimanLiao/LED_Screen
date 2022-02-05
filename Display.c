#include "gd32f10x_gpio.h"              // GigaDevice::Device:GD32F10x_StdPeripherals:GPIO
#include "Display.h"

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
PB10___ ______ ______ ______ ______ ______ ______ ______         X1
			 |      |      |      |      |      |      |      |
PB2 ___|______|______|______|______|______|______|______|        X2
			 |      |      |      |      |      |      |      |
PB1 ___|______|______|______|______|______|______|______|        X3
			 |      |      |      |      |      |      |      |
PB0 ___|______|______|______|______|______|______|______|        X4
			 |      |      |      |      |      |      |      |
PA7 ___|______|______|______|______|______|______|______|        X5
			 |      |      |      |      |      |      |      |
PA6 ___|______|______|______|______|______|______|______|        X6
       |      |      |      |      |     PB4    PB3   PA15(GND)
PB15___|______|______|______|______|                             X7 (Circle_Left)
       |      |      |      |      |
PB11___|______|______|______|______|                             X8 (Circle_Right)
			PB9    PB8    PB7    PB6    PB5
		 
		 Set VCC and GND(PIN6 ~ PIN13) ----- Display One DIG(A ~ G) and One H
     PINx Set----- +5V or GND (LED On)
		 
Error:
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
static void GPIO_Pin_Toggle(uint32_t gpio_periph, uint32_t pin)
{
	if(gpio_output_bit_get(gpio_periph, pin) == RESET)
	{
		gpio_bit_set(gpio_periph, pin);
	}
	else
	{
		gpio_bit_reset(gpio_periph, pin);
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
	
	Display_Data[DIG_Pos][7] = (DigH_Value == 0) ? 0 : 1;
}

/**********************************/
/*				Public Function					*/
/**********************************/
void Matrix_Flush(void)		//Display Matrix Elements One by One
{
	static uint32_t Last_VCC_PIN = 0;
	static uint32_t Current_VCC_PIN = 0;			//0 ~ 7 (X1 ~ X8)
	static uint32_t BlinkTick = 0;
	bit_status gpio_value;
	uint32_t i;
	
	Last_VCC_PIN = Current_VCC_PIN;
	Current_VCC_PIN = (Current_VCC_PIN == 7) ? 1 : (Last_VCC_PIN + 1);
	
	//VCC Pin Control
	gpio_bit_reset(VCC_Pin[Last_VCC_PIN].Port_Num, VCC_Pin[Last_VCC_PIN].Pin_num);
	gpio_bit_set(VCC_Pin[Current_VCC_PIN].Port_Num, VCC_Pin[Current_VCC_PIN].Pin_num);
	
	//GND Pin Control
	if(Current_VCC_PIN < 6)			//0 ~ 5 (X1 ~ X6)
	{
		for(i = 0; i < 8; i++)
		{
			gpio_value = (Display_Data[Current_VCC_PIN][i] == 0) ? RESET : SET;
			gpio_bit_write(GND_Pin[i].Port_Num, GND_Pin[i].Pin_num, gpio_value);
		}
	}
	else											 	//6 ~ 7 (X7 ~ X8)
	{
		for(i = 0; i < 5; i++)		//0---Off, 1---On, 2---Blink
		{
			if(Display_Data[Current_VCC_PIN][i] == 0)
			{
				gpio_bit_reset(GND_Pin[i].Port_Num, GND_Pin[i].Pin_num);
			}
			else if(Display_Data[Current_VCC_PIN][i] == 1)
			{
				gpio_bit_set(GND_Pin[i].Port_Num, GND_Pin[i].Pin_num);
			}
			else if(Display_Data[Current_VCC_PIN][i] == 2)
			{
				if(BlinkTick == 1000)
				{
					GPIO_Pin_Toggle(GND_Pin[i].Port_Num, GND_Pin[i].Pin_num);
					BlinkTick = 0;
				}
				else
				{
					BlinkTick++;
				}
			}
		}
	}
}

/*
We assume that their DIG numbers is different, which potentially makes different codes.
So there is no need to merge Display_SOC and Display_Kwh.
*/
void Display_SOC(uint32_t soc)				//Unit = 1 (soc = 0 ~ 100)
{
	uint32_t One;
	uint32_t Ten;
	uint32_t Hundred;
	
	if(soc > 100)return;
	
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
	
	Hundred = (uint32_t)(mKwh / 100);
	Ten = (uint32_t)((mKwh - Hundred * 100) / 10);
	One = (uint32_t)(mKwh - Hundred * 100 - Ten * 10);
	
	Display_DigNum(DIG4, Hundred);
	Display_DigNum(DIG5, Ten);
	Display_DigH(DIG5, 1);
	Display_DigNum(DIG6, One);
}

void Display_GunId(uint32_t GUN_X, uint8_t OnOff)
{
	OnOff = (OnOff == 0) ? 0 : 1;
	
	if(GUN_X == GUN_A)
	{
		Display_DigH(DIG1, OnOff);
	}
	else if(GUN_X == GUN_B)
	{
		Display_DigH(DIG2, OnOff);
	}
	else if(GUN_X == GUN_A)
	{
		Display_DigH(DIG3, OnOff);
	}
}

void Matrix_Init(void)
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
}
