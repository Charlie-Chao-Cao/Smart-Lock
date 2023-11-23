#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "lcd.h"	
#include "led.h"
#include "key.h"
#include "spi.h"
#include "w25qxx.h"
#include "rc522.h"

//要写入到W25Q128的字符串数组
const u8 TEXT_Buffer[]={"123456"};
#define SIZE sizeof(TEXT_Buffer)	



int main(void)
{				 
	//LCD变量
	u8 lcd_id[12]; //存放 LCD ID字符串
	
	//SPI & Flash变量
	u8 key;
	//u16 i=0;
	u8 datatemp[SIZE];
	u32 FLASH_SIZE; 
    u16 id = 0;
	
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
	W25QXX_Init();			//W25QXX初始化
	
	RC522_Init();	    //初始化射频卡模块
		
	while(1)
	{
		id = W25QXX_ReadID();
		if (id == W25Q128 || id == NM25Q128)
			break;
		LCD_ShowString(30,150,200,16,16,"W25Q128 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,150,200,16,16,"Please Check!        ");
		delay_ms(500);
		LED0=!LED0;//DS0闪烁
	}
	LCD_ShowString(30,150,200,16,16,"W25Q128 Ready!");    
	FLASH_SIZE=128*1024*1024;	//FLASH 大小为16M字节
	
  	while(1)
	{
		
		//keyboard_scan();
		LCD_ShowString(30,40,210,24,24,"WarShip STM32 ^_^"); 
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//显示LCD ID	      					 
		//LCD_ShowNum(30,130,keyboard_scan(),12,12);		

		RC522_Handel();
		key=keyboard_scan();
		if(key==1)	//KEY1按下,写入W25QXX
		{
			LCD_Fill(0,170,239,319,WHITE);//清除半屏    
 			LCD_ShowString(30,170,200,16,16,"Start Write W25Q128...."); 
			W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);			//从倒数第100个地址处开始,写入SIZE长度的数据
			LCD_ShowString(30,170,200,16,16,"W25Q128 Write Finished!");	//提示传送完成
		}
		if(key==2)	//KEY0按下,读取字符串并显示
		{
 			LCD_ShowString(30,170,200,16,16,"Start Read W25Q128.... ");
			W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//从倒数第100个地址处开始,读出SIZE个字节
			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:  ");	//提示传送完成
			LCD_ShowString(30,190,200,16,16,datatemp);//显示读到的字符串
		}
		
		
		
		

	}	 
} 




