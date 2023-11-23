#include "spi.h"
 
//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������W25Q128/NRF24L01
//SPI�ڳ�ʼ��
//�������Ƕ�SPI2�ĳ�ʼ��
void SPI2_Init(void)
{	 
	RCC->APB2ENR|=1<<3;  	//PORTBʱ��ʹ�� 	 
	RCC->APB1ENR|=1<<14;   	//SPI2ʱ��ʹ�� 
	//����ֻ���SPI�ڳ�ʼ��
	GPIOB->CRH&=0X000FFFFF; 
	GPIOB->CRH|=0XBBB00000;	//PB13/14/15���� 	    
	GPIOB->ODR|=0X7<<13;   	//PB13/14/15����
	SPI2->CR1|=0<<10;		//ȫ˫��ģʽ	
	SPI2->CR1|=1<<9; 		//���nss����
	SPI2->CR1|=1<<8;  

	SPI2->CR1|=1<<2; 		//SPI����
	SPI2->CR1|=0<<11;		//8bit���ݸ�ʽ	
	SPI2->CR1|=1<<1; 		//����ģʽ��SCKΪ1 CPOL=1
	SPI2->CR1|=1<<0; 		//���ݲ����ӵڶ���ʱ����ؿ�ʼ,CPHA=1  
	//��SPI2����APB1������.ʱ��Ƶ�����Ϊ36M.
	SPI2->CR1|=3<<3; 		//Fsck=Fpclk1/256
	SPI2->CR1|=0<<7; 		//MSBfirst   
	SPI2->CR1|=1<<6; 		//SPI�豸ʹ��
	SPI2_ReadWriteByte(0xff);//�������䣨ά�� MOSIΪ�ߵ�ƽ������ȥ����		 
}   
//SPI2�ٶ����ú���
//SpeedSet:0~7
//SPI�ٶ�=fAPB1/2^(SpeedSet+1)
//APB1ʱ��һ��Ϊ36Mhz
void SPI2_SetSpeed(u8 SpeedSet)
{
	SpeedSet&=0X07;			//���Ʒ�Χ
	SPI2->CR1&=0XFFC7; 
	SPI2->CR1|=SpeedSet<<3;	//����SPI2�ٶ�  
	SPI2->CR1|=1<<6; 		//SPI�豸ʹ��	  
} 
//SPI2 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u16 retry=0;				 
	while((SPI2->SR&1<<1)==0)		//�ȴ���������	
	{
		retry++;
		if(retry>=0XFFFE)return 0; 	//��ʱ�˳�
	}			  
	SPI2->DR=TxData;	 	  		//����һ��byte 
	retry=0;
	while((SPI2->SR&1<<0)==0) 		//�ȴ�������һ��byte  
	{
		retry++;
		if(retry>=0XFFFE)return 0;	//��ʱ�˳�
	}	  						    
	return SPI2->DR;          		//�����յ�������				    
}


/*
//������SPIģ��ĳ�ʼ�����룬���ó�����ģʽ������W25Q128/NRF24L01
//SPI�ڳ�ʼ��
//�������Ƕ�SPI2�ĳ�ʼ��
void SPI1_Init(void)
{
	//GPIO��ʼ��
	RCC->APB2ENR|=1<<2;  	//PORTAʱ��ʹ�� 	 
	RCC->APB2ENR|=1<<12;   	//SPI1ʱ��ʹ�� 
	GPIOA->CRL&=0X0000FFF0; 
	GPIOA->CRH|=0XBBBB000B;	//PA0/4/5/6/7�����������	    
	GPIOA->ODR|=0XF1;   		//PA0/4/5/6/7����
	
	//SPI��ʼ��
	SPI1->CR1|=0<<10;		//ȫ˫��ģʽ	
	SPI1->CR1|=0<<15;		//˫��˫��ģʽ
	
	SPI1->CR1|=1<<2; 		//����Ϊ���豸
	SPI1->CR1|=0<<11;		//8bit���ݸ�ʽ	
	SPI1->CR1|=1<<1; 		//����״̬ʱ��SCK���ָߵ�ƽ������ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI1->CR1|=1<<0; 		//���ݲ����ӵڶ���ʱ����ؿ�ʼ,CPHA=1  
	
	SPI1->CR1|=1<<9; 		//���nss����
	SPI1->CR1|=1<<8;  
	
	//��SPI1����APB1������.ʱ��Ƶ�����Ϊ36M.
	SPI1->CR1|=3<<3; 		//Fsck=Fpclk1/256
	SPI1->CR1|=0<<7; 		//MSBfirst   
	SPI1->CR1|=1<<6; 		//SPI�豸ʹ��
	SPI1_ReadWriteByte(0xff);//�������䣨ά�� MOSIΪ�ߵ�ƽ������ȥ����	
}   

//SPI2 ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData)
{		
	u16 retry=0;				 
	while((SPI1->SR&1<<1)==0)		//�ȴ���������	
	{
		retry++;
		if(retry>=0XFFFE)return 0; 	//��ʱ�˳�
	}			  
	SPI1->DR=TxData;	 	  		//����һ��byte 
	retry=0;
	while((SPI1->SR&1<<0)==0) 		//�ȴ�������һ��byte  
	{
		retry++;
		if(retry>=0XFFFE)return 0;	//��ʱ�˳�
	}	  						    
	return SPI1->DR;          		//�����յ�������				    
}

//SPI1�ٶ����ú���
//SpeedSet:0~7
//SPI�ٶ�=fAPB1/2^(SpeedSet+1)
//APB1ʱ��һ��Ϊ36Mhz
void SPI1_SetSpeed(u8 SpeedSet)
{
	SpeedSet&=0X07;			//���Ʒ�Χ
	SPI1->CR1&=0XFFC7; 
	SPI1->CR1|=SpeedSet<<3;	//����SPI1�ٶ�  
	SPI1->CR1|=1<<6; 		//SPI�豸ʹ��	  
} 

*/


