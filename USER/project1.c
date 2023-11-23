#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "lcd.h"	
#include "led.h"
#include "key.h"
#include "spi.h"
#include "w25qxx.h"
#include "rc522.h"

//Ҫд�뵽W25Q128���ַ�������
const u8 TEXT_Buffer[]={"123456"};
#define SIZE sizeof(TEXT_Buffer)	



int main(void)
{				 
	//LCD����
	u8 lcd_id[12]; //��� LCD ID�ַ���
	
	//SPI & Flash����
	u8 key;
	//u16 i=0;
	u8 datatemp[SIZE];
	u32 FLASH_SIZE; 
    u16 id = 0;
	
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
	W25QXX_Init();			//W25QXX��ʼ��
	
	RC522_Init();	    //��ʼ����Ƶ��ģ��
		
	while(1)
	{
		id = W25QXX_ReadID();
		if (id == W25Q128 || id == NM25Q128)
			break;
		LCD_ShowString(30,150,200,16,16,"W25Q128 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,150,200,16,16,"Please Check!        ");
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
	LCD_ShowString(30,150,200,16,16,"W25Q128 Ready!");    
	FLASH_SIZE=128*1024*1024;	//FLASH ��СΪ16M�ֽ�
	
  	while(1)
	{
		
		//keyboard_scan();
		LCD_ShowString(30,40,210,24,24,"WarShip STM32 ^_^"); 
		LCD_ShowString(30,70,200,16,16,"TFTLCD TEST");
		LCD_ShowString(30,90,200,16,16,"ATOM@ALIENTEK");
 		LCD_ShowString(30,110,200,16,16,lcd_id);		//��ʾLCD ID	      					 
		//LCD_ShowNum(30,130,keyboard_scan(),12,12);		

		RC522_Handel();
		key=keyboard_scan();
		if(key==1)	//KEY1����,д��W25QXX
		{
			LCD_Fill(0,170,239,319,WHITE);//�������    
 			LCD_ShowString(30,170,200,16,16,"Start Write W25Q128...."); 
			W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);			//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
			LCD_ShowString(30,170,200,16,16,"W25Q128 Write Finished!");	//��ʾ�������
		}
		if(key==2)	//KEY0����,��ȡ�ַ�������ʾ
		{
 			LCD_ShowString(30,170,200,16,16,"Start Read W25Q128.... ");
			W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:  ");	//��ʾ�������
			LCD_ShowString(30,190,200,16,16,datatemp);//��ʾ�������ַ���
		}
		
		
		
		

	}	 
} 




