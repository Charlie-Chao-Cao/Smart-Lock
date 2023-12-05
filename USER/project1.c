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

void UpdateFingerprint(void);
void DeleteFingerprint(void);
	
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
    {"1. Update Fingerprint" , &UpdateFingerprint, NULL,2},
    {"2. Delete Fingerprint", &DeleteFingerprint, NULL,2}
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
void Del_FR(u8 num);	//ɾ��ָ��
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
	LCD_Clear(BLACK);
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	LCD_ShowString(10,300,220,12,12,"S4:UP  S8:DOWN  S12:CONFIRM  S16:DOWN");
	Add_FR(menuStack[stackTop].index);
}

void DeleteFingerprint(void)
{	
	LCD_Clear(BLACK);
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	LCD_ShowString(10,300,220,12,12,"S4:UP  S8:DOWN  S12:CONFIRM  S16:DOWN");
	Del_FR(menuStack[stackTop].index);
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
		LCD_ShowString(30,70,200,16,16,"W25Q128 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,70,200,16,16,"Please Check!");
		delay_ms(500);
		LED0=!LED0;//DS0��˸
	}
    LCD_ShowString(30,70,200,16,16,"W25Q128 Ready!");
	FLASH_SIZE=128*1024*1024;	//FLASH ��СΪ16M�ֽ�
	
	//AS608��ʼ��
	LCD_ShowString(30,170,200,16,16,"Handshake with AS608....");
	//Show_Str_Mid(0,40,"��AS608ģ������....",16,240);
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		delay_ms(400);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,110,200,16,16,"AS608 not detected!");
		//Show_Str_Mid(0,40,"δ��⵽ģ��!!!",16,240);
		delay_ms(800);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,110,200,16,16,"Try to connect module...");
		//Show_Str_Mid(0,40,"��������ģ��...",16,240);		  
	}
	LCD_Fill(30,110,240,100,WHITE);
	LCD_ShowString(30,110,200,16,16,"AS608: Comm. Success!");
	//Show_Str_Mid(0,40,"ͨѶ�ɹ�!!!",16,240);
	str=mymalloc(SRAMIN,30);
	sprintf(str,"Baudrate: %d",usart2_baund);
	LCD_ShowString(30,130,200,16,16,(u8*)str);
	sprintf(str,"Address: %x",AS608Addr);
	LCD_ShowString(30,150,200,16,16,(u8*)str);
	//Show_Str(0,60,240,16,(u8*)str,16,0);
	ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	if(ensure!=0x00)
		ShowErrMessage(ensure);//��ʾȷ���������Ϣ	
	ensure=PS_ReadSysPara(&AS608Para);  //������ 
	if(ensure==0x00)
	{
		mymemset(str,0,50);
		sprintf(str,"Lib. Cap. %d",AS608Para.PS_max-ValidN);
		LCD_ShowString(30,170,200,16,16,(u8*)str);
		sprintf(str,"Comp. Level: %d",AS608Para.PS_level);
		LCD_ShowString(30,190,200,16,16,(u8*)str);
		//Show_Str(0,80,240,16,(u8*)str,16,0);
	}
	else
		ShowErrMessage(ensure);	
	myfree(SRAMIN,str);
	
	
	
	delay_ms(1000);
	LCD_Clear(BLACK);
	
	
	
//	while(1)
//	{
//		LCD_Clear(BLACK);
//		key=keyboard_scan();
//		if (key==1) {
//            int keyPressed=key;
//			printf("Press a key, key=%d\t\n",keyPressed);
//			
//            // ���ò˵���������
//            Add_FR(1);

//            // ������������
//        }
//	}
	
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
	LCD_ShowString(10,300,220,12,12,"S4:UP  S8:DOWN  S12:CONFIRM  S16:DOWN");
}


//��ʾȷ���������Ϣ
void ShowErrMessage(u8 ensure)
{
	LCD_Fill(0,60,lcddev.width,110,BLACK);
	POINT_COLOR=YELLOW;
	Show_Str_Mid(0,70,(u8*)EnsureMessage(ensure),16,240);
	POINT_COLOR=WHITE;
	//LCD_ShowString(30,230,200,16,16,(u8*)EnsureMessage(ensure));
}
//¼ָ��
void Add_FR(int num)
{
	u8 i,ensure ,processnum=0;
	u16 ID;
	while(1)
	{
		printf("%d\t\n",processnum);
		switch (processnum)
		{
			case 0:
				i++;
				LCD_Fill(0,110,lcddev.width,170,BLACK);
				Show_Str_Mid(0,110,"Please press fingerprint",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP=1;
					ensure=PS_GenChar(CharBuffer1);//��������
					BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,130,lcddev.width,170,BLACK);
						Show_Str_Mid(0,130,"Fingerprint normal",16,240);
						i=0;
						processnum=1;//�����ڶ���						
					}else ShowErrMessage(ensure);				
				}else ShowErrMessage(ensure);						
				break;
			
			case 1:
				i++;
				LCD_Fill(0,110,lcddev.width,170,BLACK);
				Show_Str_Mid(0,110,"Press fingerprint again",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					BEEP=1;
					ensure=PS_GenChar(CharBuffer2);//��������
					BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,130,lcddev.width,170,BLACK);
						Show_Str_Mid(0,130,"Fingerprint normal",16,240);
						i=0;
						processnum=2;//����������
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;

			case 2:
				LCD_Fill(0,110,lcddev.width,170,BLACK);
				Show_Str_Mid(0,110,"Compare two fingerprints...",16,240);
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					LCD_Fill(0,130,lcddev.width,170,BLACK);
					Show_Str_Mid(0,130,"Comparison successful,",16,240);
					Show_Str_Mid(0,150,"fingerprints match!",16,240);
					processnum=3;//�������Ĳ�
				}
				else 
				{
					LCD_Fill(0,110,lcddev.width,170,BLACK);
					Show_Str_Mid(0,130,"Comparison failed,",16,240);
					Show_Str_Mid(0,150,"please re-enter fingerprint!",16,240);
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//���ص�һ��		
				}
				delay_ms(1200);
				break;

			case 3:
				LCD_Fill(0,110,lcddev.width,170,BLACK);
				Show_Str_Mid(0,110,"Generate fingerprint template...",16,240);
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					LCD_Fill(0,130,lcddev.width,170,BLACK);
					Show_Str_Mid(0,130,"Template generated!",16,240);
					processnum=4;//�������岽
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	
				LCD_Fill(0,110,lcddev.width,170,BLACK);
				//Show_Str_Mid(0,110,"�����봢��ID,��Enter����",16,240);
				//Show_Str_Mid(0,120,"0=< ID <=299",16,240);
				do
					//ID=GET_NUM();
					ID=num;
				while(!(ID<AS608Para.PS_max));//����ID����С�����������ֵ
				ensure=PS_StoreChar(CharBuffer2,ID);//����ģ��
				if(ensure==0x00) 
				{			
					LCD_Fill(0,110,lcddev.width,170,BLACK);					
					Show_Str_Mid(0,110,"Fingerprint entry successful!",16,240);
					//LCD_ShowNum(0,130, ID, sizeof(ID), 16);
					PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
					//LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
					delay_ms(1500);
					LCD_Fill(0,110,240,170,BLACK);
					return ;
				}
				else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}
		delay_ms(400);
		if(i==5)//����5��û�а���ָ���˳�
		{
			LCD_Fill(0,100,lcddev.width,170,BLACK);
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
				POINT_COLOR=GREEN;
				Show_Str_Mid(0,110,"Verification successful!",16,240);				
				str=mymalloc(SRAMIN,50);
				sprintf(str,"ID: %d  Score: %d",(seach.pageID)+1,seach.mathscore);
				Show_Str_Mid(0,130,(u8*)str,16,240);
				POINT_COLOR=WHITE;
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

//ɾ��ָ��
void Del_FR(u8 num)
{
	u8  ensure;
	u16 ValidN;
	//LCD_Fill(0,100,lcddev.width,160,WHITE);
	//Show_Str_Mid(0,100,"ɾ��ָ��",16,240);
	//Show_Str_Mid(0,120,"������ָ��ID��Enter����",16,240);
	//Show_Str_Mid(0,140,"0=< ID <=299",16,240);
	//delay_ms(50);
	//AS608_load_keyboard(0,170,(u8**)kbd_delFR);
	//num=GET_NUM();//��ȡ���ص���ֵ
	
	if(num==0xFF)
		ensure=PS_Empty();//���ָ�ƿ�
	else 
		ensure=PS_DeletChar(num,1);//ɾ������ָ��
	if(ensure==0)
	{
		LCD_Fill(0,110,lcddev.width,160,BLACK);
		Show_Str_Mid(0,110,"Fingerprint deleted!",16,240);		
	}
	else
		ShowErrMessage(ensure);	
	delay_ms(1200);
	PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	LCD_Clear(BLACK);
	//LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
//MENU:	
//	LCD_Fill(0,100,lcddev.width,160,WHITE);
//	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_menu);
}


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