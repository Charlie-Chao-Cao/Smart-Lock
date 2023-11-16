//4x4矩阵键盘，函数不支持连续按， 扫描某个按键，该按键按下之后必须要松开，
//才能第二次触发，否则不会再响应这个按键，这样的好处就是可以防止按一次多次

#include "key.h"
#include "delay.h"

static u8 key_up=0;//按键按松开标志 0为松开
								    
//按键初始化函数
void KEY_Init(void)
{
	
	RCC->APB2ENR|=1<<6;     //使能PORTE时钟	//手册P71
	RCC->APB2ENR|=1<<7;     //使能PORTF时钟
	GPIOE->CRL&=0XFFF0000F;	//PE1234设置成输入，默认上拉
	GPIOE->CRL|=0X00088880; 
	GPIOE->ODR|=0xF<<1;	   	//PE1~4 上拉

	GPIOF->CRL&=0XFFF0000F;	//PF1~4设置成推挽输出
	GPIOF->CRL|=0X00033330; 		
	GPIOF->ODR|=0xF<<1;	
	//横行初始高电平，检测的那一行给低电平
	
} 



int keyboard_scan(void)
{
	
	/*************Scan  1st Line************************/

    GPIOF->ODR=0xF<<1;	//4行都给初始高电平

    GPIOF->ODR&=~(1<<1);	//R1给低电平
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//去抖动 
		key_up=1;
		if(KEY_C1==0)return 1;
		else if(KEY_C2==0)return 2;
		else if(KEY_C3==0)return 3;
		else if(KEY_C4==0)return 11;
	}
	else if(key_up==1&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	    
 	//return 0;// 无按键按下
	
	
		/*************Scan  2st Line************************/
//	key_up=1;//按键按松开标志	
	
    GPIOF->ODR=0xF<<1;	//4行都给初始高电平

    GPIOF->ODR&=~(1<<2);	//R2给低电平
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//去抖动 
		key_up=2;
		if(KEY_C1==0)return 4;
		else if(KEY_C2==0)return 5;
		else if(KEY_C3==0)return 6;
		else if(KEY_C4==0)return 12;
	}
	else if(key_up==2&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	    


		/*************Scan  3st Line************************/
//	key_up=1;//按键按松开标志
	
    GPIOF->ODR=0xF<<1;	//4行都给初始高电平

    GPIOF->ODR&=~(1<<3);	//R3给低电平
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//去抖动 
		key_up=3;
		if(KEY_C1==0)return 7;
		else if(KEY_C2==0)return 8;
		else if(KEY_C3==0)return 9;
		else if(KEY_C4==0)return 13;
	}
	else if(key_up==3&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	 


		/*************Scan  4st Line************************/
//	key_up=1;//按键按松开标志
	
    GPIOF->ODR=0xF<<1;	//4行都给初始高电平

    GPIOF->ODR&=~(1<<4);	//R3给低电平
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//去抖动 
		key_up=4;
		if(KEY_C1==0)return 16;
		else if(KEY_C2==0)return 0;
		else if(KEY_C3==0)return 15;
		else if(KEY_C4==0)return 14;
	}
	else if(key_up==4&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	


    return -1;

}
















