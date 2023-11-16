#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "lcd.h"	
#include "led.h"
#include "key.h"



int main(void)
{				 
	//LCD����
	u8 x=0; 
	u8 lcd_id[12]; //��� LCD ID�ַ���
	
	Stm32_Clock_Init(9);	//ϵͳʱ������
	delay_init(72);	  		//��ʱ��ʼ��
	uart_init(72,115200); 	//���ڳ�ʼ��Ϊ115200
	
	
	//LCD��ʼ��
	LCD_Init();
	POINT_COLOR=BLACK;
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣 
	LCD_Clear(WHITE);
	
	LED_Init();
	KEY_Init();
	
  	while(1)
	{
		
		//keyboard_scan();
		LCD_ShowString(30,40,210,24,24,"WarShip STM32 ^_^"); 
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//��ʾLCD ID	      					 
		//LCD_ShowNum(30,130,keyboard_scan(),12,12);		
	    x++;
		//if(x==12)x=0;			   		 
		//delay_ms(1000);	
		
		printf("%d\n\t",keyboard_scan());
		printf("\n");
		

	}	 
} 
