#include "sys.h"
#include "usart.h"		
#include "delay.h"	

#include <string.h>

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



// 定义功能函数
void LCD_ClearMiddle(void);
void UpdateFingerprint(void);
void DeleteFingerprint(void);
void changePassword(void);	
void addKeyCard(void);
void deleteKeyCard(void);

void unlockDoor(void) {
    printf("Door unlocked!\n");
}


// 定义子菜单
typedef struct MenuItem {
    char name[25];
    void (*function)(void);
    struct MenuItem *submenu;
	int submenuSize;
} MenuItem;

MenuItem fingerprintUpdateorDelete[] = {
    {"1. Update Fingerprint" , &UpdateFingerprint, NULL,NULL},
    {"2. Delete Fingerprint", &DeleteFingerprint, NULL,NULL}
};

MenuItem fingerprintSettingsMenu[] = {
    {"1. Fingerprint 1" , NULL, fingerprintUpdateorDelete,2},
    {"2. Fingerprint 2", NULL, fingerprintUpdateorDelete,2},
	{"3. Fingerprint 3", NULL, fingerprintUpdateorDelete,2},
    {"4. Fingerprint 4" , NULL, fingerprintUpdateorDelete,2},
	{"5. Fingerprint 5", NULL, fingerprintUpdateorDelete,2}
};

MenuItem keyCardSettingsMenu[] = {
    {"1. Add key card" , &addKeyCard, NULL,NULL},
    {"2. Delete key card", &deleteKeyCard, NULL,NULL}
};

// 定义主菜单
MenuItem mainMenu[] = {
    {"1. Change Password", &changePassword, NULL,NULL},
    {"2. Fingerprint Settings", NULL, fingerprintSettingsMenu,5},
	{"3. Key card Settings", NULL, keyCardSettingsMenu,2}
};
// 定义一个常量表示菜单的大小
#define MAIN_MENU_SIZE 3


// 当前菜单
MenuItem *currentMenu = mainMenu;
int menuSize = MAIN_MENU_SIZE;

// 定义按键枚举
typedef enum {
    UP=11,
    DOWN=12,
    CONFIRM=13,
    BACK=14
} Key;

// 菜单导航函数
void navigateMenu(Key key);

// 当前选中菜单项的索引
int selectedIndex = 0;

// 栈结构
typedef struct {
    MenuItem *menu;
    int index;
	int size;
} MenuState;

// 定义菜单状态栈
MenuState menuStack[5];  // 假设栈的最大深度为10
int stackTop = -1;  // 栈顶索引，-1 表示栈为空

void refreshLCD(void);

u32 FLASH_SIZE=128*1024*1024;	//FLASH 大小为16M字节


//AS608相关
#define usart2_baund  57600//串口2波特率，根据指纹模块波特率更改

SysPara AS608Para;//指纹模块AS608参数
u16 ValidN;//模块内有效指纹个数

void Add_FR(int num);	//录指纹
void Del_FR(u8 num);	//删除指纹
void press_FR(void);//刷指纹
void ShowErrMessage(u8 ensure);//显示确认码错误信息
//void AS608_load_keyboard(u16 x,u16 y,u8 **kbtbl);//加载虚拟键盘
//u8  AS608_get_keynum(u16 x,u16 y);//获取键盘数
//u16 GET_NUM(void);//获取数值


//要写入到W25Q128的密码
// Define the size of the password
#define PASSWORD_SIZE 6
// Global variable to store the password
int Password[PASSWORD_SIZE] ={1, 2, 3, 4, 5, 6};
//const u8 Password[]={"123456"};
#define SIZE sizeof(Password)	


void UpdateFingerprint(void)
{	
	LCD_Clear(BLACK);
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	LCD_ShowString(10,300,220,12,12,"S4:UP  S8:DOWN  S12:CONFIRM  S16:BACK");
	Add_FR(menuStack[stackTop].index);
}

void DeleteFingerprint(void)
{	
	LCD_Clear(BLACK);
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	LCD_ShowString(10,300,220,12,12,"S4:UP  S8:DOWN  S12:CONFIRM  S16:BACK");
	Del_FR(menuStack[stackTop].index);
}

void addKeyCard(void) {
	LCD_ClearMiddle();
	Show_Str_Mid(0,80,"Add key card",16,240);
	Show_Str_Mid(0,120,"Please place the key card",16,240);
    while(RC522_Handel(1)==0)	//mode 1
	{
		if(keyboard_scan()==14)	break;
	}	
	LCD_ClearMiddle();	
}
void deleteKeyCard(void) {
	LCD_ClearMiddle();
	Show_Str_Mid(0,80,"Delete key card",16,240);
	Show_Str_Mid(0,120,"Please place the key card",16,240);
	while(RC522_Handel(2)==0)	//mode 2 
	{
		if(keyboard_scan()==14)	break;
	}	
	LCD_ClearMiddle();
}

// Function to compare passwords
int comparePasswords(int userInput[PASSWORD_SIZE]) {
	int i;
    for (i = 0; i < PASSWORD_SIZE; i++) {
        if (userInput[i] != Password[i]) {
            return 0; // Passwords do not match
        }
    }
    return 1; // Passwords match
}

u8 getPassword(int *newPassword,u8 temp) {
	int i=0;
	int key1=-1;
	if(temp==0)
	{
		i=0;
	}		
	else 
	{
		LCD_ShowNum(16, 140, newPassword[0], sizeof(newPassword[0]), 16);	//第一个数字
		i=1;
	}
	LCD_ShowString(20,100,200,16,16,"Enter a 6-digit password:");
	while(1)
	{
		key1=keyboard_scan();
		if((key1>=0&&key1<=9)&&i<6)
		{
			newPassword[i]=key1;
			LCD_ShowNum(16+i*32, 140, newPassword[i], sizeof(newPassword[i]), 16);
			i++;
		}
		else if(key1==14)
		{
			return 0;
		}
		else if(key1==15&&i>0)
		{
			i--;
			LCD_Fill(16+i*32,140,16+(i+1)*32,160,BLACK);
		}
		else if (key1 == 13) {
            if (i == 6) {
                break;
            }
        }
	}
	return 1;
}

void changePassword(void) {
	int newPassword[6];
	
	LCD_Clear(BLACK);
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	LCD_ShowString(10,300,220,12,12,"S12: CONFIRM  S16: BACK  S15: DELETE");
	
	A1:
	Show_Str_Mid(0,60,"Change Password",16,240);
	Show_Str_Mid(0,80,"Check old password",16,240);
	if(getPassword(newPassword,0)==1)
	{
		if(comparePasswords(newPassword))
		{
			LCD_ClearMiddle();//清除半屏 
			Show_Str_Mid(0,100,"Password is correct!",16,240);
			delay_ms(1000);
			LCD_ClearMiddle();//清除半屏 
			Show_Str_Mid(0,80,"Set new password",16,240);
			if(getPassword(newPassword,0))
			{
				//printf("getPassword: %d%d%d%d%d%d\t\n", newPassword[0],newPassword[1],newPassword[2],newPassword[3],newPassword[4],newPassword[5]);
				LCD_ClearMiddle(); 
				LCD_ShowString(20,120,200,16,16,"Start writing password...."); 
				W25QXX_Write((u8*)newPassword,FLASH_SIZE-100,SIZE);			//从倒数第100个地址处开始,写入SIZE长度的数据
				
				W25QXX_Read((u8*)Password,FLASH_SIZE-100,SIZE);
				printf("密码修改成功！新密码为: %d%d%d%d%d%d\t\n", Password[0],Password[1],Password[2],Password[3],Password[4],Password[5]);
				LCD_ShowString(20,120,200,16,16,"Password change completed!");	//提示传送完成
				delay_ms(1000);
			}
		}
		else
		{
			LCD_ClearMiddle();//清除半屏 
			Show_Str_Mid(0,100,"Wrong password, please re-enter!",16,240);
			delay_ms(1000);
			LCD_ClearMiddle();//清除半屏 
			goto A1;
		}
	}
	LCD_ClearMiddle();
}

void checkPassword(int x) {
	int enteredPassword[6];
	enteredPassword[0]=x;
	
	LCD_Clear(BLACK);
	LCD_ShowString(60,20,210,24,24,"Smart Lock");
	LCD_ShowString(10,300,220,12,12,"S12: CONFIRM  S16: BACK  S15: DELETE");
	
	if(getPassword(enteredPassword,1))	
	{
		LCD_ClearMiddle();//清除半屏    
		if(comparePasswords(enteredPassword)==1)
		{
			Show_Str_Mid(0,120,"The door lock is unlocked!",16,240);
		}
		else
		{
			Show_Str_Mid(0,120,"Incorrect Password!",16,240);
		}
		delay_ms(1000);
	}
	
	LCD_ClearMiddle();
}

void LCD_ClearMiddle(void)
{
	LCD_Fill(0,50,240,290,BLACK);//清除半屏  
}


int main(void)
{				 
	//LCD变量
	u8 lcd_id[12]; //存放 LCD ID字符串
	
	//SPI & Flash变量
	int key;
	//u16 i=0;
//	u8 datatemp[SIZE];
//	u32 FLASH_SIZE; 
    u16 id = 0;
	
	//AS608变量
	u8 ensure;
//	u8 key_num;
	char *str;	 
	
	Stm32_Clock_Init(9);	//系统时钟设置
	delay_init(72);	  		//延时初始化
	uart_init(72,115200); 	//串口初始化为115200
	
	//LCD初始化
	LCD_Init();
	POINT_COLOR=WHITE;
	BACK_COLOR = BLACK; 
	sprintf((char*)lcd_id,"LCD ID:%04X",lcddev.id);//将LCD ID打印到lcd_id数组。 
	LCD_Clear(BLACK);
	
	LED_Init();
	KEY_Init();
	BEEP_Init();					//初始化蜂鸣器
	W25QXX_Init();			//W25QXX初始化
	
	RC522_Init();	    //初始化射频卡模块
	
	usart2_init(36,usart2_baund);		//初始化串口2 
	
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
		LED0=!LED0;//DS0闪烁
	}
    LCD_ShowString(30,70,200,16,16,"W25Q128 Ready!");
//	FLASH_SIZE=128*1024*1024;	//FLASH 大小为16M字节
	W25QXX_Read((u8*)Password,FLASH_SIZE-100,SIZE);		//读取密码
	
	//AS608初始化
	LCD_ShowString(30,170,200,16,16,"Handshake with AS608....");
	//Show_Str_Mid(0,40,"与AS608模块握手....",16,240);
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		delay_ms(400);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,110,200,16,16,"AS608 not detected!");
		//Show_Str_Mid(0,40,"未检测到模块!!!",16,240);
		delay_ms(800);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,110,200,16,16,"Try to connect module...");
		//Show_Str_Mid(0,40,"尝试连接模块...",16,240);		  
	}
	LCD_Fill(30,110,240,100,WHITE);
	LCD_ShowString(30,110,200,16,16,"AS608: Comm. Success!");
	//Show_Str_Mid(0,40,"通讯成功!!!",16,240);
	str=mymalloc(SRAMIN,30);
	sprintf(str,"Baudrate: %d",usart2_baund);
	LCD_ShowString(30,130,200,16,16,(u8*)str);
	sprintf(str,"Address: %x",AS608Addr);
	LCD_ShowString(30,150,200,16,16,(u8*)str);
	//Show_Str(0,60,240,16,(u8*)str,16,0);
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		ShowErrMessage(ensure);//显示确认码错误信息	
	ensure=PS_ReadSysPara(&AS608Para);  //读参数 
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
	
  	while(1)
	{
		refreshLCD();
		RC522_Handel(0);
		key=keyboard_scan();		
		//printf("%d\t\n",sizeof(currentMenu) / sizeof(currentMenu[0]));
		
		// 轮询检测按键
        if (key==11||key==12||key==13||key==14) {
            int keyPressed=key;
			printf("Press a key, key=%d\t\n",keyPressed);

            // 调用菜单导航函数
            navigateMenu((Key)keyPressed);

            // 处理其他任务
        }
		else if((key>=0&&key<=9))
		{
			checkPassword(key);
		}
		
		if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
		{
			press_FR();//刷指纹			
		}
	}	 
} 


// LCD 刷新函数
void refreshLCD(void) {
    // 在这里编写刷新 LCD 屏幕的代码
    // 可能涉及 LCD 驱动函数的调用，更新显示内容等操作
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
	LCD_ShowString(10,300,220,12,12,"S4:UP  S8:DOWN  S12:CONFIRM  S16:BACK");
}


//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	LCD_Fill(0,60,lcddev.width,110,BLACK);
	POINT_COLOR=YELLOW;
	Show_Str_Mid(0,70,(u8*)EnsureMessage(ensure),16,240);
	POINT_COLOR=WHITE;
	//LCD_ShowString(30,230,200,16,16,(u8*)EnsureMessage(ensure));
}
//录指纹
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
					ensure=PS_GenChar(CharBuffer1);//生成特征
					BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,130,lcddev.width,170,BLACK);
						Show_Str_Mid(0,130,"Fingerprint normal",16,240);
						i=0;
						processnum=1;//跳到第二步						
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
					ensure=PS_GenChar(CharBuffer2);//生成特征
					BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,130,lcddev.width,170,BLACK);
						Show_Str_Mid(0,130,"Fingerprint normal",16,240);
						i=0;
						processnum=2;//跳到第三步
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
					processnum=3;//跳到第四步
				}
				else 
				{
					LCD_Fill(0,110,lcddev.width,170,BLACK);
					Show_Str_Mid(0,130,"Comparison failed,",16,240);
					Show_Str_Mid(0,150,"please re-enter fingerprint!",16,240);
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//跳回第一步		
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
					processnum=4;//跳到第五步
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	
				LCD_Fill(0,110,lcddev.width,170,BLACK);
				//Show_Str_Mid(0,110,"请输入储存ID,按Enter保存",16,240);
				//Show_Str_Mid(0,120,"0=< ID <=299",16,240);
				do
					//ID=GET_NUM();
					ID=num;
				while(!(ID<AS608Para.PS_max));//输入ID必须小于容量的最大值
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{			
					LCD_Fill(0,110,lcddev.width,170,BLACK);					
					Show_Str_Mid(0,110,"Fingerprint entry successful!",16,240);
					//LCD_ShowNum(0,130, ID, sizeof(ID), 16);
					PS_ValidTempleteNum(&ValidN);//读库指纹个数
					//LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
					delay_ms(1500);
					LCD_Fill(0,110,240,170,BLACK);
					return ;
				}
				else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}
		delay_ms(400);
		if(i==5)//超过5次没有按手指则退出
		{
			LCD_Fill(0,100,lcddev.width,170,BLACK);
			break;	
		}				
	}
}

//刷指纹
void press_FR(void)
{
	SearchResult seach;
	u8 ensure;
	char *str;
	ensure=PS_GetImage();
	if(ensure==0x00)//获取图像成功 
	{
		BEEP=1;//打开蜂鸣器	
		ensure=PS_GenChar(CharBuffer1);
		if(ensure==0x00) //生成特征成功
		{		
			BEEP=0;//关闭蜂鸣器	
			ensure=PS_HighSpeedSearch(CharBuffer1,0,AS608Para.PS_max,&seach);
			if(ensure==0x00)//搜索成功
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
		BEEP=0;//关闭蜂鸣器
		delay_ms(1000);
		LCD_Clear(BLACK);
		//LCD_Fill(0,100,lcddev.width,160,WHITE);
	}
		
}

//删除指纹
void Del_FR(u8 num)
{
	u8  ensure;
	u16 ValidN;
	//LCD_Fill(0,100,lcddev.width,160,WHITE);
	//Show_Str_Mid(0,100,"删除指纹",16,240);
	//Show_Str_Mid(0,120,"请输入指纹ID按Enter发送",16,240);
	//Show_Str_Mid(0,140,"0=< ID <=299",16,240);
	//delay_ms(50);
	//AS608_load_keyboard(0,170,(u8**)kbd_delFR);
	//num=GET_NUM();//获取返回的数值
	
	if(num==0xFF)
		ensure=PS_Empty();//清空指纹库
	else 
		ensure=PS_DeletChar(num,1);//删除单个指纹
	if(ensure==0)
	{
		LCD_Fill(0,110,lcddev.width,160,BLACK);
		Show_Str_Mid(0,110,"Fingerprint deleted!",16,240);		
	}
	else
		ShowErrMessage(ensure);	
	delay_ms(1200);
	PS_ValidTempleteNum(&ValidN);//读库指纹个数
	LCD_Clear(BLACK);
	//LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
//MENU:	
//	LCD_Fill(0,100,lcddev.width,160,WHITE);
//	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_menu);
}


// 菜单导航函数
void navigateMenu(Key key) {
    switch (key) {
        case UP:
            // 处理上键操作，选择上一个菜单项
			POINT_COLOR=BLACK;
			LCD_ShowString(10,70+(20*selectedIndex),200,16,16,"->");	//擦除光标
			POINT_COLOR=WHITE;
		
			if (selectedIndex > 0) {
				selectedIndex--;
			} else {
				selectedIndex = menuSize-1;
			}
            printf("Selected menu item: %s\n", currentMenu[selectedIndex].name);
            break;
        case DOWN:
            // 处理下键操作，选择下一个菜单项
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
            // 执行确认键操作，例如执行当前菜单项的功能函数
            if (currentMenu[selectedIndex].function != NULL) {
                currentMenu[selectedIndex].function();
            } 
			else if (currentMenu[selectedIndex].submenu != NULL) {
                // 进入子菜单，保存当前状态到栈
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
            // 处理返回键操作，返回上一级菜单
            if (stackTop >= 0) {
                // 弹出栈中的状态，恢复之前的状态
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
            // 处理其他按键
            break;
    }
}
