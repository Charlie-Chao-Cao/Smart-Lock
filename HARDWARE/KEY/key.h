#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

	

#define KEY_C1	PEin(1) //PE1
#define KEY_C2	PEin(2)	//PE2
#define KEY_C3 	PEin(3)	//PE3
#define KEY_C4 	PEin(4)	//PE4
#define KEY_R1	PFout(1) //PF1
#define KEY_R2	PFout(2)	//PF2 
#define KEY_R3 	PFout(3)	//PF3
#define KEY_R4 	PFout(4)	//PF4

//#define KEY0_PRES 	1	//KEY0按下
//#define KEY1_PRES	2	//KEY1按下
//#define KEY2_PRES	3	//KEY2按下
//#define WKUP_PRES   4	//KEY_UP按下(即WK_UP/KEY_UP)


void KEY_Init(void);//IO初始化
int keyboard_scan(void);
//u8 KEY_Scan(u8);  	//按键扫描函数					    
#endif
