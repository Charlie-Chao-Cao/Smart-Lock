//4x4������̣�������֧���������� ɨ��ĳ���������ð�������֮�����Ҫ�ɿ���
//���ܵڶ��δ��������򲻻�����Ӧ��������������ĺô����ǿ��Է�ֹ��һ�ζ��

#include "key.h"
#include "delay.h"

static u8 key_up=0;//�������ɿ���־ 0Ϊ�ɿ�
								    
//������ʼ������
void KEY_Init(void)
{
	
	RCC->APB2ENR|=1<<6;     //ʹ��PORTEʱ��	//�ֲ�P71
	RCC->APB2ENR|=1<<7;     //ʹ��PORTFʱ��
	GPIOE->CRL&=0XFFF0000F;	//PE1234���ó����룬Ĭ������
	GPIOE->CRL|=0X00088880; 
	GPIOE->ODR|=0xF<<1;	   	//PE1~4 ����

	GPIOF->CRL&=0XFFF0000F;	//PF1~4���ó��������
	GPIOF->CRL|=0X00033330; 		
	GPIOF->ODR|=0xF<<1;	
	//���г�ʼ�ߵ�ƽ��������һ�и��͵�ƽ
	
} 



int keyboard_scan(void)
{
	
	/*************Scan  1st Line************************/

    GPIOF->ODR=0xF<<1;	//4�ж�����ʼ�ߵ�ƽ

    GPIOF->ODR&=~(1<<1);	//R1���͵�ƽ
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=1;
		if(KEY_C1==0)return 1;
		else if(KEY_C2==0)return 2;
		else if(KEY_C3==0)return 3;
		else if(KEY_C4==0)return 11;
	}
	else if(key_up==1&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	    
 	//return 0;// �ް�������
	
	
		/*************Scan  2st Line************************/
//	key_up=1;//�������ɿ���־	
	
    GPIOF->ODR=0xF<<1;	//4�ж�����ʼ�ߵ�ƽ

    GPIOF->ODR&=~(1<<2);	//R2���͵�ƽ
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=2;
		if(KEY_C1==0)return 4;
		else if(KEY_C2==0)return 5;
		else if(KEY_C3==0)return 6;
		else if(KEY_C4==0)return 12;
	}
	else if(key_up==2&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	    


		/*************Scan  3st Line************************/
//	key_up=1;//�������ɿ���־
	
    GPIOF->ODR=0xF<<1;	//4�ж�����ʼ�ߵ�ƽ

    GPIOF->ODR&=~(1<<3);	//R3���͵�ƽ
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=3;
		if(KEY_C1==0)return 7;
		else if(KEY_C2==0)return 8;
		else if(KEY_C3==0)return 9;
		else if(KEY_C4==0)return 13;
	}
	else if(key_up==3&&KEY_C1==1&&KEY_C2==1&&KEY_C3==1&&KEY_C4==1)
		key_up=0; 	 


		/*************Scan  4st Line************************/
//	key_up=1;//�������ɿ���־
	
    GPIOF->ODR=0xF<<1;	//4�ж�����ʼ�ߵ�ƽ

    GPIOF->ODR&=~(1<<4);	//R3���͵�ƽ
	if(key_up==0&&(KEY_C1==0||KEY_C2==0||KEY_C3==0||KEY_C4==0))
	{
		delay_ms(10);//ȥ���� 
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
















