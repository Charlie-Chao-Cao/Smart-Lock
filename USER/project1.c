#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "lcd.h"	
#include "led.h"
#include "key.h"



int main(void)
{				 
	//LCD变量
	u8 x=0; 
	u8 lcd_id[12]; //存放 LCD ID字符串
	
	Stm32_Clock_Init(9);	//系统时钟设置
	delay_init(72);	  		//延时初始化
	uart_init(72,115200); 	//串口初始化为115200
	
	
	//LCD初始化
	LCD_Init();
	POINT_COLOR=BLACK;
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。 
	LCD_Clear(WHITE);
	
	LED_Init();
	KEY_Init();
	
  	while(1)
	{
		
		//keyboard_scan();
		LCD_ShowString(30,40,210,24,24,"WarShip STM32 ^_^"); 
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//显示LCD ID	      					 
		//LCD_ShowNum(30,130,keyboard_scan(),12,12);		
	    x++;
		//if(x==12)x=0;			   		 
		//delay_ms(1000);	
		
		printf("%d\n\t",keyboard_scan());
		printf("\n");
		

	}	 
} 
