#include "gd32f10x.h"                   // Device header
#include "gd32f10x_gpio.h"              // GigaDevice::Device:GD32F10x_StdPeripherals:GPIO

#include "GunStaDisplay.h"
#include "MatrixDisplay.h"
#include "DataPool.h"
#include "Timer.h"
#include "KeyCtrl.h"

#define CHG_SIGN_GREEN	0
#define CHG_SIGN_RED		1

#define GUN_A_DISPLAY		0
#define GUN_B_DISPLAY		1
#define GUN_C_DISPLAY		2

extern GUN_DATA GunA;
extern GUN_DATA GunB;
extern GUN_DATA GunC;

extern KEY_DATA Key1;
extern KEY_DATA Key2;
extern KEY_DATA Key3;

extern uint32_t GunDisplayNow;

static uint32_t GunX_Display = 0;

static uint32_t GunA_DisplayTick = 0;
static uint32_t GunB_DisplayTick = 0;
static uint32_t GunC_DisplayTick = 0;

typedef enum
{
	Bar0 = 48, Bar1 = 49, Bar2 = 50, Bar3 = 51, Bar4 = 52,
	Bar5 = 56, Bar6 = 57, Bar7 = 58, Bar8 = 59, Bar9 = 60
}CHG_BAR;

//uint8_t ChgBar[10] = {48, 49, 50, 51, 52,
//											56, 57, 58, 59, 60};

/**********************************/
/*				Private Function				*/
/**********************************/
static uint32_t abs(uint32_t x, uint32_t y)
{
	return ((x > y) ? (x - y) : (y - x));
}

static void Display_Others(void)			//Display the Square, '%' and "Energy Delivered"
{
	/**/
}

static void ChgSign_Ctrl(uint32_t color, uint32_t value)		//0---Off, Other---On
{
	/**/
}

//static void ChgFree_Display(GUN_DATA *Gun)
//{
//	MatrixOff();
//}

static void ChgPlug_Display(GUN_DATA *Gun)
{
	Display_Others();						//Display the Square, '%' and "Energy Delivered"
	Display_GunId(Gun->GunId);
	Display_SOC(0);
	Display_Kwh(0);
	
	Display_RoundBar1(400);			//0.4s = 400ms
}

static void ChgWait_Display(GUN_DATA *Gun)
{
	static uint32_t ChgSign_BlinkTick = 0;
	static uint32_t ChgSign_Val = 0;
	
	Display_Others();						//Display the Square, '%' and "Energy Delivered"
	Display_GunId(Gun->GunId);
	Display_SOC(0);
	Display_Kwh(0);
	
	if(abs(clock_tick(), ChgSign_BlinkTick) > 4)	//The First Flipp is Incorrect (it's OK): 0.4s = 4 * 0.1s
	{
		//ChgSign_Val = (ChgSign_Val == 0) ? 1 : 0;
		//ChgSign_Ctrl(ChgSign_Val);
		ChgSign_Ctrl(CHG_SIGN_GREEN, ~ChgSign_Val);
		
		ChgSign_BlinkTick = clock_tick();
	}
}

static void ChgStart_Display(GUN_DATA *Gun)
{
	Display_Others();						//Display the Square, '%' and "Energy Delivered"
	Display_GunId(Gun->GunId);
	Display_SOC(Gun->soc);
	Display_Kwh(Gun->kwh);
	
	ChgSign_Ctrl(CHG_SIGN_GREEN, 1);
	
	Display_RoundBar2(Gun->soc, 100);				//0.1s = 100ms + No blink + Go to next stage(CHG_STA_CHG)to blink
}

static void ChgChg_Display(GUN_DATA *Gun)
{
	static uint32_t ChgBar_BlinkTick = 0;
	static uint32_t ChgBar_Val = 0;
	uint32_t tmp, i;
	CHG_BAR ChgBar = Bar0;
	
	Display_Others();						//Display the Square, '%' and "Energy Delivered"
	Display_GunId(Gun->GunId);
	Display_SOC(Gun->soc);
	Display_Kwh(Gun->kwh);
	
	ChgSign_Ctrl(CHG_SIGN_GREEN, 1);
	
	tmp = (uint32_t)((Gun->soc) / 10);
	for(i = 0; i < tmp; i++)
	{
		Display_OneLED(ChgBar, 1);
		ChgBar++;
	}
	
	if(abs(clock_tick(), ChgBar_BlinkTick) > 4)	//The First Flipp is Incorrect (it's OK): 0.4s = 4 * 0.1s
	{
		//ChgBar_Val = (ChgBar_Val == 0) ? 1 : 0;
		//Display_OneLED(ChgBar, ChgBar_Val);
		Display_OneLED(ChgBar, ~ChgBar_Val);				//Quicker but without auto-correct Ability (Once flipp wrong, then wrong forever)
		
		ChgBar_BlinkTick = clock_tick();
	}
}

static void ChgErr_Display(GUN_DATA *Gun)
{
	ChgSign_Ctrl(CHG_SIGN_RED, 1);
	Display_Err(Gun->GunId);
}
/**********************************/
/*				Public Function					*/
/**********************************/

int Gun_Display_Init(void)
{
	if(Matrix_Init() == -1)
	{
		return -1;
	}
	
	GunA.GunId = GUN_A;
	GunB.GunId = GUN_B;
	GunC.GunId = GUN_C;
	
	GunA.ChgSta = CHG_STA_FREE;
	GunB.ChgSta = CHG_STA_FREE;
	GunC.ChgSta = CHG_STA_FREE;
	
	return 0;
}

void Gun_Display(void)
{
	switch(GunX_Display)
	{
		case GUN_A_DISPLAY:
		{
			switch(GunA.ChgSta)
			{
				case CHG_STA_FREE:
				{
					MatrixOff();
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					GunX_Display = GUN_B_DISPLAY;
					
					break;
				}
				case CHG_STA_PLUG:
				{
					KeyIRQ_Disable();
					ChgPlug_Display(&GunA);
					GunA.ChgSta = CHG_STA_WAIT;
					KeyIRQ_Enable();
					
					break;
				}
				case CHG_STA_WAIT:
				{
					ChgWait_Display(&GunA);
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_B)
						{
							GunX_Display = GUN_B_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_C)
						{
							GunX_Display = GUN_C_DISPLAY;
						}
					}
					
					break;
				}
				case CHG_STA_START:
				{
					KeyIRQ_Disable();
					ChgStart_Display(&GunA);
					GunA.ChgSta = CHG_STA_CHG;
					KeyIRQ_Enable();
					
					break;
				}
				case CHG_STA_CHG:
				{
					ChgChg_Display(&GunA);
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(GunA_DisplayTick == 0)
					{
						GunA_DisplayTick = clock_tick();
					}
					if(abs(GunA_DisplayTick, clock_tick()) > 60)//6s = 60 * 0.1s
					{
						GunA_DisplayTick = 0;
						GunX_Display = GUN_B_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_B)
						{
							GunX_Display = GUN_B_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_C)
						{
							GunX_Display = GUN_C_DISPLAY;
						}
					}
					
					break;
				}
				case CHG_STA_ERR:
				{
					ChgErr_Display(&GunA);
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(GunA_DisplayTick == 0)
					{
						GunA_DisplayTick = clock_tick();
					}
					if(abs(GunA_DisplayTick, clock_tick()) > 60)//6s = 60 * 0.1s
					{
						GunA_DisplayTick = 0;
						GunX_Display = GUN_B_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_B)
						{
							GunX_Display = GUN_B_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_C)
						{
							GunX_Display = GUN_C_DISPLAY;
						}
					}
					
					break;
				}
				default:
					break;
			}
		}
		case GUN_B_DISPLAY:
		{
			switch(GunB.ChgSta)
			{
				case CHG_STA_FREE:
				{
					MatrixOff();
					
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					GunX_Display = GUN_C_DISPLAY;
					
					break;
				}
				case CHG_STA_PLUG:
				{
					KeyIRQ_Disable();
					ChgPlug_Display(&GunB);
					GunB.ChgSta = CHG_STA_WAIT;
					KeyIRQ_Enable();
					
					break;
				}
				case CHG_STA_WAIT:
				{
					ChgWait_Display(&GunB);
					
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_A)
						{
							GunX_Display = GUN_A_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_C)
						{
							GunX_Display = GUN_C_DISPLAY;
						}
					}
					
					break;
				}
				case CHG_STA_START:
				{
					KeyIRQ_Disable();
					ChgStart_Display(&GunB);
					GunA.ChgSta = CHG_STA_CHG;
					KeyIRQ_Enable();
					
					break;
				}
				case CHG_STA_CHG:
				{
					ChgChg_Display(&GunB);
					
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(GunB_DisplayTick == 0)
					{
						GunB_DisplayTick = clock_tick();
					}
					if(abs(GunB_DisplayTick, clock_tick()) > 60)//6s = 60 * 0.1s
					{
						GunB_DisplayTick = 0;
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_A)
						{
							GunX_Display = GUN_A_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_C)
						{
							GunX_Display = GUN_C_DISPLAY;
						}
					}
					
					break;
				}
				case CHG_STA_ERR:
				{
					ChgErr_Display(&GunB);
					
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					if(GunC.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(GunB_DisplayTick == 0)
					{
						GunB_DisplayTick = clock_tick();
					}
					if(abs(GunB_DisplayTick, clock_tick()) > 60)//6s = 60 * 0.1s
					{
						GunB_DisplayTick = 0;
						GunX_Display = GUN_C_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_A)
						{
							GunX_Display = GUN_A_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_C)
						{
							GunX_Display = GUN_C_DISPLAY;
						}
					}
					
					break;
				}
				default:
					break;
			}
		}
		case GUN_C_DISPLAY:
		{
			switch(GunC.ChgSta)
			{
				case CHG_STA_FREE:
				{
					MatrixOff();
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					GunX_Display = GUN_A_DISPLAY;
					
					break;
				}
				case CHG_STA_PLUG:
				{
					KeyIRQ_Disable();
					ChgPlug_Display(&GunC);
					GunC.ChgSta = CHG_STA_WAIT;
					KeyIRQ_Enable();
					
					break;
				}
				case CHG_STA_WAIT:
				{
					ChgWait_Display(&GunC);
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_A)
						{
							GunX_Display = GUN_A_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_B)
						{
							GunX_Display = GUN_B_DISPLAY;
						}
					}
					
					break;
				}
				case CHG_STA_START:
				{
					KeyIRQ_Disable();
					ChgStart_Display(&GunC);
					GunC.ChgSta = CHG_STA_CHG;
					KeyIRQ_Enable();
					
					break;
				}
				case CHG_STA_CHG:
				{
					ChgChg_Display(&GunC);
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					
					if(GunC_DisplayTick == 0)
					{
						GunC_DisplayTick = clock_tick();
					}
					if(abs(GunC_DisplayTick, clock_tick()) > 60)//6s = 60 * 0.1s
					{
						GunC_DisplayTick = 0;
						GunX_Display = GUN_A_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_A)
						{
							GunX_Display = GUN_A_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_B)
						{
							GunX_Display = GUN_B_DISPLAY;
						}
					}
					
					break;
				}
				case CHG_STA_ERR:
				{
					ChgErr_Display(&GunC);
					
					if(GunB.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_B_DISPLAY;
					}
					if(GunA.ChgSta == CHG_STA_PLUG)
					{
						GunX_Display = GUN_A_DISPLAY;
					}
					
					if(GunC_DisplayTick == 0)
					{
						GunC_DisplayTick = clock_tick();
					}
					if(abs(GunC_DisplayTick, clock_tick()) > 60)//6s = 60 * 0.1s
					{
						GunC_DisplayTick = 0;
						GunX_Display = GUN_A_DISPLAY;
					}
					
					if(Key1.isPressed == 1)
					{
						if(Key1.PressCount == SELECT_A)
						{
							GunX_Display = GUN_A_DISPLAY;
						}
						else if(Key1.PressCount == SELECT_B)
						{
							GunX_Display = GUN_B_DISPLAY;
						}
					}
					
					break;
				}
				default:
					break;
			}
		}
		default:
			break;
	}
}
