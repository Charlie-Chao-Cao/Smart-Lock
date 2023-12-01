#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include "lcd.h"	
#include "led.h"
#include "key.h"
#include "spi.h"
#include "w25qxx.h"
#include "rc522.h"
#include "usart2.h"
#include "AS608.h" 
#include "malloc.h" 
#include "beep.h"



// ���幦�ܺ���
void unlockDoor(void) {
    printf("Door unlocked!\n");
}

void changePassword(void) {
    printf("Changing password...\n");
}

void viewLogs(void) {
    printf("Viewing access logs...\n");
}

// �����Ӳ˵�
typedef struct MenuItem {
    char name[25];
    void (*function)(void);
    struct MenuItem *submenu;
	int submenuSize;
} MenuItem;

MenuItem fingerprintUpdateorDelete[] = {
    {"1. Update Fingerprint" , &viewLogs, NULL,2},
    {"2. Delete Fingerprint", &viewLogs, NULL,2}
};

MenuItem fingerprintSettingsMenu[] = {
    {"1. Fingerprint 1" , NULL, fingerprintUpdateorDelete,2},
    {"2. Fingerprint 2", NULL, fingerprintUpdateorDelete,2},
	{"3. Fingerprint 3", NULL, fingerprintUpdateorDelete,2},
    {"4. Fingerprint 4" , NULL, fingerprintUpdateorDelete,2},
	{"5. Fingerprint 5", NULL, fingerprintUpdateorDelete,2}
};

MenuItem keyCardSettingsMenu[] = {
    {"1. Key card 1" , &viewLogs, NULL,2},
    {"2. Key card 2", &viewLogs, NULL,2}
};

// �������˵�
MenuItem mainMenu[] = {
    {"1. Change Password", &changePassword, NULL,NULL},
    {"2. Fingerprint Settings", NULL, fingerprintSettingsMenu,5},
	{"3. Key card Settings", NULL, keyCardSettingsMenu,2}
};
// ����һ��������ʾ�˵��Ĵ�С
#define MAIN_MENU_SIZE 3


// ��ǰ�˵�
MenuItem *currentMenu = mainMenu;
int menuSize = MAIN_MENU_SIZE;

// ���尴��ö��
typedef enum {
    UP=11,
    DOWN=12,
    CONFIRM=13,
    BACK=14
} Key;

// �˵���������
void navigateMenu(Key key);

// ��ǰѡ�в˵��������
int selectedIndex = 0;

// ջ�ṹ
typedef struct {
    MenuItem *menu;
    int index;
	int size;
} MenuState;

// ����˵�״̬ջ
MenuState menuStack[5];  // ����ջ��������Ϊ10
int stackTop = -1;  // ջ��������-1 ��ʾջΪ��

void refreshLCD(void);


//AS608���
#define usart2_baund  57600//����2�����ʣ�����ָ��ģ�鲨���ʸ���

SysPara AS608Para;//ָ��ģ��AS608����
u16 ValidN;//ģ������Чָ�Ƹ���
u8** kbd_tbl;
const  u8* kbd_menu[15]={"ɾָ��"," : ","¼ָ��","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//������
const  u8* kbd_delFR[15]={"����"," : ","���ָ��","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//������

void Add_FR(int num);	//¼ָ��
void Del_FR(void);	//ɾ��ָ��
void press_FR(void);//ˢָ��
void ShowErrMessage(u8 ensure);//��ʾȷ���������Ϣ
void AS608_load_keyboard(u16 x,u16 y,u8 **kbtbl);//�����������
u8  AS608_get_keynum(u16 x,u16 y);//��ȡ������
u16 GET_NUM(void);//��ȡ��ֵ


//Ҫд�뵽W25Q128���ַ�������
const u8 TEXT_Buffer[]={"123456"};
#define SIZE sizeof(TEXT_Buffer)	


void UpdateFingerprint(void)
{	
	Add_FR(selectedIndex);
}



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
	
	//AS608����
	u8 ensure;
	u8 key_num;
	char *str;	 
	
	
	Stm32_Clock_Init(9);	//ϵͳʱ������
	delay_init(72);	  		//��ʱ��ʼ��
	uart_init(72,115200); 	//���ڳ�ʼ��Ϊ115200
	
	//LCD��ʼ��
	LCD_Init();
	POINT_COLOR=WHITE;
	BACK_COLOR = BLACK; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//��LCD ID��ӡ��lcd_id���顣 
	LCD_Clear(BLACK);
	
	LED_Init();
	KEY_Init();
	BEEP_Init();					//��ʼ��������
	W25QXX_Init();			//W25QXX��ʼ��
	
	RC522_Init();	    //��ʼ����Ƶ��ģ��
	
	usart2_init(36,usart2_baund);		//��ʼ������2 
	
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
		
	while(1)
	{
		id = W25QXX_ReadID();
		if (id == W25Q128 || id == NM25Q128)
			break;
		LCD_ShowString(30,150,200,16,16,"W25Q128 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,70,200,16,16,"Please Check!");
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
    LCD_ShowString(30,70,200,16,16,"W25Q128 Ready!");
	FLASH_SIZE=128*1024*1024;	//FLASH ��СΪ16M�ֽ�
	
	//AS608��ʼ��
	LCD_ShowString(30,170,200,16,16,"��AS608ģ������....");
	//Show_Str_Mid(0,40,"��AS608ģ������....",16,240);
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		delay_ms(400);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,170,200,16,16,"δ��⵽ģ��!!!");
		//Show_Str_Mid(0,40,"δ��⵽ģ��!!!",16,240);
		delay_ms(800);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,170,200,16,16,"��������ģ��...");
		//Show_Str_Mid(0,40,"��������ģ��...",16,240);		  
	}
	LCD_ShowString(30,170,200,16,16,"ͨѶ�ɹ�!!!");
	//Show_Str_Mid(0,40,"ͨѶ�ɹ�!!!",16,240);
	str=mymalloc(SRAMIN,30);
	sprintf(str,"������:%d   ��ַ:%x",usart2_baund,AS608Addr);
	LCD_ShowString(30,190,200,16,16,(u8*)str);
	//Show_Str(0,60,240,16,(u8*)str,16,0);
	ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	if(ensure!=0x00)
		ShowErrMessage(ensure);//��ʾȷ���������Ϣ	
	ensure=PS_ReadSysPara(&AS608Para);  //������ 
	if(ensure==0x00)
	{
		mymemset(str,0,50);
		sprintf(str,"������:%d     �Աȵȼ�: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
		LCD_ShowString(30,210,200,16,16,(u8*)str);
		//Show_Str(0,80,240,16,(u8*)str,16,0);
	}
	else
		ShowErrMessage(ensure);	
	myfree(SRAMIN,str);
	
	
	
	
	
	delay_ms(1000);
	LCD_Clear(BLACK);
	
  	while(1)
	{
		refreshLCD();
		RC522_Handel();
		key=keyboard_scan();
		
		//printf("%d\t\n",sizeof(currentMenu) / sizeof(currentMenu[0]));
		
		// ��ѯ��ⰴ��
        if (key==11||key==12||key==13||key==14) {
            int keyPressed=key;
			printf("Press a key, key=%d\t\n",keyPressed);

            // ���ò˵���������
            navigateMenu((Key)keyPressed);

            // ������������
        }
		
		
		
		
//		if(key==1)	//KEY1����,д��W25QXX
//		{
//			LCD_Fill(0,170,239,319,WHITE);//�������    
// 			LCD_ShowString(30,170,200,16,16,"Start Write W25Q128...."); 
//			W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);			//�ӵ�����100����ַ����ʼ,д��SIZE���ȵ�����
//			LCD_ShowString(30,170,200,16,16,"W25Q128 Write Finished!");	//��ʾ�������
//		}
//		if(key==2)	//KEY0����,��ȡ�ַ�������ʾ
//		{
// 			LCD_ShowString(30,170,200,16,16,"Start Read W25Q128.... ");
//			W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//�ӵ�����100����ַ����ʼ,����SIZE���ֽ�
//			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:  ");	//��ʾ�������
//			LCD_ShowString(30,190,200,16,16,datatemp);//��ʾ�������ַ���
//		}
		
		
		//key_num=AS608_get_keynum(0,170);	
//		if(key_num)
//		{
//			if(key_num==1)Del_FR();		//ɾָ��
//			if(key_num==3)Add_FR();		//¼ָ��									
//		}
		if(PS_Sta)	 //���PS_Sta״̬���������ָ����
		{
			press_FR();//ˢָ��			
		}
	}	 
} 


// LCD ˢ�º���
void refreshLCD(void) {
    // �������дˢ�� LCD ��Ļ�Ĵ���
    // �����漰 LCD ���������ĵ��ã�������ʾ���ݵȲ���
	int x=35,y=70;
	//int menuSize = sizeof(currentMenu) / sizeof(currentMenu[0]);
	int i=0;
	//printf("%d\t\n",menuSize);
	
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	
	for(i=0;i<menuSize;i++)
	{
		LCD_ShowString(x,y,200,16,16,(u8*)currentMenu[i].name);
		//printf("%s\t\n",currentMenu[i].name);
		y+=20;
	}
	LCD_ShowString(10,70+(20*selectedIndex),200,16,16,"->");
	
}






////���ذ������棨�ߴ�x,yΪ240*150��
////x,y:������ʼ���꣨240*320�ֱ��ʵ�ʱ��x����Ϊ0��
//void AS608_load_keyboard(u16 x,u16 y,u8 **kbtbl)
//{
//	u16 i;
//	POINT_COLOR=RED;
//	kbd_tbl=kbtbl;
//	LCD_Fill(x,y,x+240,y+150,WHITE);
//	LCD_DrawRectangle(x,y,x+240,y+150);						   
//	LCD_DrawRectangle(x+80,y,x+160,y+150);	 
//	LCD_DrawRectangle(x,y+30,x+240,y+60);
//	LCD_DrawRectangle(x,y+90,x+240,y+120);
//	POINT_COLOR=BLUE;
//	for(i=0;i<15;i++)
//	{
//		if(i==1)//�������2����:������Ҫ�м���ʾ
//			Show_Str(x+(i%3)*80+2,y+7+30*(i/3),80,30,(u8*)kbd_tbl[i],16,0);	
//		else
//			Show_Str_Mid(x+(i%3)*80,y+7+30*(i/3),(u8*)kbd_tbl[i],16,80);
//	} 
//}
////����״̬����
////x,y:��������
////key:��ֵ(0~14)
////sta:״̬��0���ɿ���1�����£�
//void AS608_key_staset(u16 x,u16 y,u8 keyx,u8 sta)
//{		  
//	u16 i=keyx/3,j=keyx%3;
//	if(keyx>16)return;
//	if(sta &&keyx!=1)//�������2����:������Ҫ���
//		LCD_Fill(x+j*80+1,y+i*30+1,x+j*80+78,y+i*30+28,GREEN);
//	else if(keyx!=1)
//		LCD_Fill(x+j*80+1,y+i*30+1,x+j*80+78,y+i*30+28,WHITE);
//	if(keyx!=1)//���ǡ�����
//		Show_Str_Mid(x+j*80,y+7+30*i,(u8*)kbd_tbl[keyx],16,80);	
//}
////�õ�������������
////x,y:��������
////����ֵ:��1~15,��Ӧ������
//u8 AS608_get_keynum(u16 x,u16 y)
//{
//	u16 i,j;
//	static u8 key_x=0;//0,û���κΰ�������
//	u8 key=0;
//	tp_dev.scan(0); 		 
//	if(tp_dev.sta&TP_PRES_DOWN)//������������
//	{	
//		for(i=0;i<5;i++)
//		{
//			for(j=0;j<3;j++)
//			{
//			 	if(tp_dev.x[0]<(x+j*80+80)&&tp_dev.x[0]>(x+j*80)&&tp_dev.y[0]<(y+i*30+30)&&tp_dev.y[0]>(y+i*30))
//				{	
//					key=i*3+j+1;	 
//					break;	 		   
//				}
//			}
//			if(key)
//			{	   
//				if(key_x==key)key=0;
//				else 
//				{
//					AS608_key_staset(x,y,key_x-1,0);
//					key_x=key;
//					AS608_key_staset(x,y,key_x-1,1);
//				}
//				break;
//			}
//	  }  
//	}else if(key_x) 
//	{
//		AS608_key_staset(x,y,key_x-1,0);
//		key_x=0;
//	} 
//	return key; 
//}
////��ȡ������ֵ
//u16 GET_NUM(void)
//{
//	u8  key_num=0;
//	u16 num=0;
//	while(1)
//	{
//		key_num=AS608_get_keynum(0,170);	
//		if(key_num)
//		{
//			if(key_num==1)return 0xFFFF;//�����ء���
//			if(key_num==3)return 0xFF00;//		
//			if(key_num>3&&key_num<13&&num<99)//��1-9����(��������3λ��)
//				num =num*10+key_num-3;		
//			if(key_num==13)num =num/10;//��Del����			
//			if(key_num==14&&num<99)num =num*10;//��0����
//			if(key_num==15)return num;  //��Enter����
//		}
//		LCD_ShowNum(80+15,170+7,num,6,16);
//	}	
//}
//��ʾȷ���������Ϣ
void ShowErrMessage(u8 ensure)
{
	LCD_Fill(0,120,lcddev.width,160,WHITE);
	//Show_Str_Mid(0,120,(u8*)EnsureMessage(ensure),16,240);
	LCD_ShowString(30,230,200,16,16,(u8*)EnsureMessage(ensure));
}
//¼ָ��
void Add_FR(int num)
{
	u8 i,ensure ,processnum=0;
	u16 ID;
	while(1)
	{
		switch (processnum)
		{
			case 0:
				i++;
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"�밴ָ��",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					//BEEP=1;
					ensure=PS_GenChar(CharBuffer1);//��������
					//BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,120,lcddev.width,160,WHITE);
						Show_Str_Mid(0,120,"ָ������",16,240);
						i=0;
						processnum=1;//�����ڶ���						
					}else ShowErrMessage(ensure);				
				}else ShowErrMessage(ensure);						
				break;
			
			case 1:
				i++;
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"�밴�ٰ�һ��ָ��",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					//BEEP=1;
					ensure=PS_GenChar(CharBuffer2);//��������
					//BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,120,lcddev.width,160,WHITE);
						Show_Str_Mid(0,120,"ָ������",16,240);
						i=0;
						processnum=2;//����������
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;

			case 2:
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"�Ա�����ָ��",16,240);
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					LCD_Fill(0,120,lcddev.width,160,WHITE);
					Show_Str_Mid(0,120,"�Աȳɹ�,����ָ��һ��",16,240);
					processnum=3;//�������Ĳ�
				}
				else 
				{
					LCD_Fill(0,100,lcddev.width,160,WHITE);
					Show_Str_Mid(0,100,"�Ա�ʧ�ܣ�������¼��ָ��",16,240);
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//���ص�һ��		
				}
				delay_ms(1200);
				break;

			case 3:
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"����ָ��ģ��",16,240);
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					LCD_Fill(0,120,lcddev.width,160,WHITE);
					Show_Str_Mid(0,120,"����ָ��ģ��ɹ�",16,240);
					processnum=4;//�������岽
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"�����봢��ID,��Enter����",16,240);
				Show_Str_Mid(0,120,"0=< ID <=299",16,240);
				do
					//ID=GET_NUM();
					ID=num;
				while(!(ID<AS608Para.PS_max));//����ID����С�����������ֵ
				ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
				if(ensure==0x00) 
				{			
					LCD_Fill(0,100,lcddev.width,160,WHITE);					
					Show_Str_Mid(0,120,"¼��ָ�Ƴɹ�",16,240);
					PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
					LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
					delay_ms(1500);
					LCD_Fill(0,100,240,160,WHITE);
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}
		delay_ms(400);
		if(i==5)//����5��û�а���ָ���˳�
		{
			LCD_Fill(0,100,lcddev.width,160,WHITE);
			break;	
		}				
	}
}

//ˢָ��
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char *str;
	ensure=PS_GetImage();
	if(ensure==0x00)//��ȡͼ��ɹ� 
	{
		BEEP=1;//�򿪷�����	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //���������ɹ�
		{		
			BEEP=0;//�رշ�����	
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//�����ɹ�
			{				
				//LCD_Fill(0,100,lcddev.width,160,WHITE);
				LCD_Clear(BLACK);
				LCD_ShowString(60,20,210,24,24,"Smart Lock");
				LCD_ShowString(30,170,200,16,16,"ˢָ�Ƴɹ�");
				//Show_Str_Mid(0,100,"ˢָ�Ƴɹ�",16,240);				
				str=mymalloc(SRAMIN,50);
				sprintf(str,"ȷ�д���,ID:%d  ƥ��÷�:%d",seach.pageID,seach.mathscore);
				LCD_ShowString(30,170,200,16,16,(u8*)str);
				//Show_Str_Mid(0,140,(u8*)str,16,240);
				myfree(SRAMIN,str);
			}
			else 
				ShowErrMessage(ensure);					
	  }
		else
			ShowErrMessage(ensure);
		BEEP=0;//�رշ�����
		delay_ms(1000);
		LCD_Clear(BLACK);
		//LCD_Fill(0,100,lcddev.width,160,WHITE);
	}
		
}

////ɾ��ָ��
//void Del_FR(void)
//{
//	u8  ensure;
//	u16 ValidN,num;
//	LCD_Fill(0,100,lcddev.width,160,WHITE);
//	Show_Str_Mid(0,100,"ɾ��ָ��",16,240);
//	Show_Str_Mid(0,120,"������ָ��ID��Enter����",16,240);
//	Show_Str_Mid(0,140,"0=< ID <=299",16,240);
//	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_delFR);
//	num=GET_NUM();//��ȡ���ص���ֵ
//	if(num==0xFFFF)
//		goto MENU ; //������ҳ��
//	else if(num==0xFF00)
//		ensure=PS_Empty();//���ָ�ƿ�
//	else 
//		ensure=PS_DeletChar(num,1);//ɾ������ָ��
//	if(ensure==0)
//	{
//		LCD_Fill(0,120,lcddev.width,160,WHITE);
//		Show_Str_Mid(0,140,"ɾ��ָ�Ƴɹ�",16,240);		
//	}
//  else
//		ShowErrMessage(ensure);	
//	delay_ms(1200);
//	PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
//	LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
//MENU:	
//	LCD_Fill(0,100,lcddev.width,160,WHITE);
//	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_menu);
//}


// �˵���������
void navigateMenu(Key key) {
    switch (key) {
        case UP:
            // �����ϼ�������ѡ����һ���˵���
			POINT_COLOR=BLACK;
			LCD_ShowString(10,70+(20*selectedIndex),200,16,16,"->");	//�������
			POINT_COLOR=WHITE;
		
			if (selectedIndex > 0) {
				selectedIndex--;
			} else {
				selectedIndex = menuSize-1;
			}
            printf("Selected menu item: %s\n", currentMenu[selectedIndex].name);
            break;
        case DOWN:
            // �����¼�������ѡ����һ���˵���
			POINT_COLOR=BLACK;
			LCD_ShowString(10,70+(20*selectedIndex),200,16,16,"->");
			POINT_COLOR=WHITE;
		
			if (selectedIndex < menuSize-1) {
				selectedIndex++;
			} else {
				selectedIndex = 0;
			}
            printf("Selected menu item: %s\n", currentMenu[selectedIndex].name);
            break;
        case CONFIRM:
            // ִ��ȷ�ϼ�����������ִ�е�ǰ�˵���Ĺ��ܺ���
            if (currentMenu[selectedIndex].function != NULL) {
                currentMenu[selectedIndex].function();
            } 
			else if (currentMenu[selectedIndex].submenu != NULL) {
                // �����Ӳ˵������浱ǰ״̬��ջ
                stackTop++;
                menuStack[stackTop].menu = currentMenu;
                menuStack[stackTop].index = selectedIndex;
				menuStack[stackTop].size=menuSize;
				menuSize= currentMenu[selectedIndex].submenuSize;
                currentMenu = currentMenu[selectedIndex].submenu;
				
				selectedIndex=0;
				
				LCD_Clear(BLACK);
				refreshLCD();
                printf("Entered submenu\t\n");
            }
            break;
        case BACK:
            // �����ؼ�������������һ���˵�
            if (stackTop >= 0) {
                // ����ջ�е�״̬���ָ�֮ǰ��״̬
                currentMenu = menuStack[stackTop].menu;
				selectedIndex=menuStack[stackTop].index;
				menuSize=menuStack[stackTop].size;
                stackTop--;
                printf("Returned to previous menu\n");
				LCD_Clear(BLACK);
				refreshLCD();
            } 
			else {
                printf("Cannot go back, already at the top level\n");
            }
            break;
        default:
            // ������������
            break;
    }
}