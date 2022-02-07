#ifndef DATAPOOL_H
#define DATAPOOL_H

#include "gd32f10x.h"                   // Device header

typedef struct
{
	uint32_t GunId;
	uint32_t ChgSta;
	uint32_t soc;
	uint32_t kwh;
}GUN_DATA;

typedef struct
{
	uint32_t isPressed;
	uint32_t PressCount;									//Only Key "A/B/C" Use it
}KEY_DATA;

extern GUN_DATA GunA;
extern GUN_DATA GunB;
extern GUN_DATA GunC;

extern KEY_DATA Key1;										//"A/B/C"
extern KEY_DATA Key2;										//"START"
extern KEY_DATA Key3;										//"STOP"

extern uint32_t GunDisplayNow;

#endif
