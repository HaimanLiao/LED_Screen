#include "gd32f10x.h"                   // Device header

#include "gd32f10x_gpio.h"              // GigaDevice::Device:GD32F10x_StdPeripherals:GPIO
#include "gd32f10x_usart.h"             // GigaDevice::Device:GD32F10x_StdPeripherals:USART

#include "SysTick.h"
#include "Timer.h"
#include "DataPool.h"
#include "GunStaDisplay.h"
#include "KeyCtrl.h"

extern GUN_DATA GunA;
extern GUN_DATA GunB;
extern GUN_DATA GunC;

extern KEY_DATA Key1;
extern KEY_DATA Key2;
extern KEY_DATA Key3;

extern uint32_t GunDisplayNow;

//CK_AHB = 108MHz, 108MHz---->SysTimer, 54MHz(max)-->CK_APB1, 108MHz(max)-->CK_APB2
//CK_AHB = 72MHz,  72MHz----->SysTimer, 36MHz(max)-->CK_APB1, 72MHz(max)--->CK_APB2
/*CK_AHB = 48MHz,  48MHz----->SysTimer, 48MHz(max)-->CK_APB1, 48MHz(max)--->CK_APB2*/

int main(void)
{
	SysTick_Init(48000);							//CK_AHB = 48MHz
	ClockTime_Init();									//TimerTick Unit = 0.1s
	if(Gun_Display_Init() == -1)
	{
		return -1;
	}
	Delay_ms(10);

//	rcu_periph_clock_enable(RCU_GPIOB);
//	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_13);
//	gpio_bit_reset(GPIOB, GPIO_PIN_13);
//	gpio_init(GPIOB,GPIO_MODE_OUT_PP,GPIO_OSPEED_10MHZ,GPIO_PIN_12);
//	gpio_bit_set(GPIOB, GPIO_PIN_12);
	
	while(1)
	{
		Gun_Display();
		KeyLED_Display();
	}
}
