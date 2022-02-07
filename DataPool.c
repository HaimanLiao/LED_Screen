#include "gd32f10x.h"                   // Device header
#include "DataPool.h"

GUN_DATA GunA = {0};
GUN_DATA GunB = {0};
GUN_DATA GunC = {0};

KEY_DATA Key1 = {0};
KEY_DATA Key2 = {0};
KEY_DATA Key3 = {0};

uint32_t GunDisplayNow = 0xffffffff;
