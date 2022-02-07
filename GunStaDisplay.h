#ifndef GUNSTADISPLAY_H
#define GUNSTADISPLAY_H

#define GUN_A					0
#define GUN_B					1
#define GUN_C					2

#define CHG_STA_FREE	0

#define CHG_STA_PLUG	1
#define CHG_STA_WAIT	2
#define CHG_STA_START	3

#define CHG_STA_CHG		4

#define CHG_STA_ERR		5

int Gun_Display_Init(void);
void Gun_Display(void);

#endif
