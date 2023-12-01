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



// 定义功能函数
void unlockDoor(void) {
    printf("Door unlocked!\n");
}

void changePassword(void) {
    printf("Changing password...\n");
}

void viewLogs(void) {
    printf("Viewing access logs...\n");
}

// 定义子菜单
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


//AS608相关
#define usart2_baund  57600//串口2波特率，根据指纹模块波特率更改

SysPara AS608Para;//指纹模块AS608参数
u16 ValidN;//模块内有效指纹个数
u8** kbd_tbl;
const  u8* kbd_menu[15]={"删指纹"," : ","录指纹","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//按键表
const  u8* kbd_delFR[15]={"返回"," : ","清空指纹","1","2","3","4","5","6","7","8","9","DEL","0","Enter",};//按键表

void Add_FR(int num);	//录指纹
void Del_FR(void);	//删除指纹
void press_FR(void);//刷指纹
void ShowErrMessage(u8 ensure);//显示确认码错误信息
void AS608_load_keyboard(u16 x,u16 y,u8 **kbtbl);//加载虚拟键盘
u8  AS608_get_keynum(u16 x,u16 y);//获取键盘数
u16 GET_NUM(void);//获取数值


//要写入到W25Q128的字符串数组
const u8 TEXT_Buffer[]={"123456"};
#define SIZE sizeof(TEXT_Buffer)	


void UpdateFingerprint(void)
{	
	Add_FR(selectedIndex);
}



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
	
	//AS608变量
	u8 ensure;
	u8 key_num;
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
		LCD_ShowString(30,150,200,16,16,"W25Q128 Check Failed!");
		delay_ms(500);
		LCD_ShowString(30,70,200,16,16,"Please Check!");
		delay_ms(500);
		LED0=!LED0;//DS0闪烁
	}
    LCD_ShowString(30,70,200,16,16,"W25Q128 Ready!");
	FLASH_SIZE=128*1024*1024;	//FLASH 大小为16M字节
	
	//AS608初始化
	LCD_ShowString(30,170,200,16,16,"与AS608模块握手....");
	//Show_Str_Mid(0,40,"与AS608模块握手....",16,240);
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		delay_ms(400);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,170,200,16,16,"未检测到模块!!!");
		//Show_Str_Mid(0,40,"未检测到模块!!!",16,240);
		delay_ms(800);
		LCD_Fill(0,40,240,80,WHITE);
		LCD_ShowString(30,170,200,16,16,"尝试连接模块...");
		//Show_Str_Mid(0,40,"尝试连接模块...",16,240);		  
	}
	LCD_ShowString(30,170,200,16,16,"通讯成功!!!");
	//Show_Str_Mid(0,40,"通讯成功!!!",16,240);
	str=mymalloc(SRAMIN,30);
	sprintf(str,"波特率:%d   地址:%x",usart2_baund,AS608Addr);
	LCD_ShowString(30,190,200,16,16,(u8*)str);
	//Show_Str(0,60,240,16,(u8*)str,16,0);
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		ShowErrMessage(ensure);//显示确认码错误信息	
	ensure=PS_ReadSysPara(&AS608Para);  //读参数 
	if(ensure==0x00)
	{
		mymemset(str,0,50);
		sprintf(str,"库容量:%d     对比等级: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
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
		
		// 轮询检测按键
        if (key==11||key==12||key==13||key==14) {
            int keyPressed=key;
			printf("Press a key, key=%d\t\n",keyPressed);

            // 调用菜单导航函数
            navigateMenu((Key)keyPressed);

            // 处理其他任务
        }
		
		
		
		
//		if(key==1)	//KEY1按下,写入W25QXX
//		{
//			LCD_Fill(0,170,239,319,WHITE);//清除半屏    
// 			LCD_ShowString(30,170,200,16,16,"Start Write W25Q128...."); 
//			W25QXX_Write((u8*)TEXT_Buffer,FLASH_SIZE-100,SIZE);			//从倒数第100个地址处开始,写入SIZE长度的数据
//			LCD_ShowString(30,170,200,16,16,"W25Q128 Write Finished!");	//提示传送完成
//		}
//		if(key==2)	//KEY0按下,读取字符串并显示
//		{
// 			LCD_ShowString(30,170,200,16,16,"Start Read W25Q128.... ");
//			W25QXX_Read(datatemp,FLASH_SIZE-100,SIZE);					//从倒数第100个地址处开始,读出SIZE个字节
//			LCD_ShowString(30,170,200,16,16,"The Data Readed Is:  ");	//提示传送完成
//			LCD_ShowString(30,190,200,16,16,datatemp);//显示读到的字符串
//		}
		
		
		//key_num=AS608_get_keynum(0,170);	
//		if(key_num)
//		{
//			if(key_num==1)Del_FR();		//删指纹
//			if(key_num==3)Add_FR();		//录指纹									
//		}
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
	
}






////加载按键界面（尺寸x,y为240*150）
////x,y:界面起始坐标（240*320分辨率的时候，x必须为0）
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
//		if(i==1)//按键表第2个‘:’不需要中间显示
//			Show_Str(x+(i%3)*80+2,y+7+30*(i/3),80,30,(u8*)kbd_tbl[i],16,0);	
//		else
//			Show_Str_Mid(x+(i%3)*80,y+7+30*(i/3),(u8*)kbd_tbl[i],16,80);
//	} 
//}
////按键状态设置
////x,y:键盘坐标
////key:键值(0~14)
////sta:状态，0，松开；1，按下；
//void AS608_key_staset(u16 x,u16 y,u8 keyx,u8 sta)
//{		  
//	u16 i=keyx/3,j=keyx%3;
//	if(keyx>16)return;
//	if(sta &&keyx!=1)//按键表第2个‘:’不需要清除
//		LCD_Fill(x+j*80+1,y+i*30+1,x+j*80+78,y+i*30+28,GREEN);
//	else if(keyx!=1)
//		LCD_Fill(x+j*80+1,y+i*30+1,x+j*80+78,y+i*30+28,WHITE);
//	if(keyx!=1)//不是‘：’
//		Show_Str_Mid(x+j*80,y+7+30*i,(u8*)kbd_tbl[keyx],16,80);	
//}
////得到触摸屏的输入
////x,y:键盘坐标
////返回值:（1~15,对应按键表）
//u8 AS608_get_keynum(u16 x,u16 y)
//{
//	u16 i,j;
//	static u8 key_x=0;//0,没有任何按键按下
//	u8 key=0;
//	tp_dev.scan(0); 		 
//	if(tp_dev.sta&TP_PRES_DOWN)//触摸屏被按下
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
////获取键盘数值
//u16 GET_NUM(void)
//{
//	u8  key_num=0;
//	u16 num=0;
//	while(1)
//	{
//		key_num=AS608_get_keynum(0,170);	
//		if(key_num)
//		{
//			if(key_num==1)return 0xFFFF;//‘返回’键
//			if(key_num==3)return 0xFF00;//		
//			if(key_num>3&&key_num<13&&num<99)//‘1-9’键(限制输入3位数)
//				num =num*10+key_num-3;		
//			if(key_num==13)num =num/10;//‘Del’键			
//			if(key_num==14&&num<99)num =num*10;//‘0’键
//			if(key_num==15)return num;  //‘Enter’键
//		}
//		LCD_ShowNum(80+15,170+7,num,6,16);
//	}	
//}
//显示确认码错误信息
void ShowErrMessage(u8 ensure)
{
	LCD_Fill(0,120,lcddev.width,160,WHITE);
	//Show_Str_Mid(0,120,(u8*)EnsureMessage(ensure),16,240);
	LCD_ShowString(30,230,200,16,16,(u8*)EnsureMessage(ensure));
}
//录指纹
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
				Show_Str_Mid(0,100,"请按指纹",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					//BEEP=1;
					ensure=PS_GenChar(CharBuffer1);//生成特征
					//BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,120,lcddev.width,160,WHITE);
						Show_Str_Mid(0,120,"指纹正常",16,240);
						i=0;
						processnum=1;//跳到第二步						
					}else ShowErrMessage(ensure);				
				}else ShowErrMessage(ensure);						
				break;
			
			case 1:
				i++;
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"请按再按一次指纹",16,240);
				ensure=PS_GetImage();
				if(ensure==0x00) 
				{
					//BEEP=1;
					ensure=PS_GenChar(CharBuffer2);//生成特征
					//BEEP=0;
					if(ensure==0x00)
					{
						LCD_Fill(0,120,lcddev.width,160,WHITE);
						Show_Str_Mid(0,120,"指纹正常",16,240);
						i=0;
						processnum=2;//跳到第三步
					}else ShowErrMessage(ensure);	
				}else ShowErrMessage(ensure);		
				break;

			case 2:
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"对比两次指纹",16,240);
				ensure=PS_Match();
				if(ensure==0x00) 
				{
					LCD_Fill(0,120,lcddev.width,160,WHITE);
					Show_Str_Mid(0,120,"对比成功,两次指纹一样",16,240);
					processnum=3;//跳到第四步
				}
				else 
				{
					LCD_Fill(0,100,lcddev.width,160,WHITE);
					Show_Str_Mid(0,100,"对比失败，请重新录入指纹",16,240);
					ShowErrMessage(ensure);
					i=0;
					processnum=0;//跳回第一步		
				}
				delay_ms(1200);
				break;

			case 3:
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"生成指纹模板",16,240);
				ensure=PS_RegModel();
				if(ensure==0x00) 
				{
					LCD_Fill(0,120,lcddev.width,160,WHITE);
					Show_Str_Mid(0,120,"生成指纹模板成功",16,240);
					processnum=4;//跳到第五步
				}else {processnum=0;ShowErrMessage(ensure);}
				delay_ms(1200);
				break;
				
			case 4:	
				LCD_Fill(0,100,lcddev.width,160,WHITE);
				Show_Str_Mid(0,100,"请输入储存ID,按Enter保存",16,240);
				Show_Str_Mid(0,120,"0=< ID <=299",16,240);
				do
					//ID=GET_NUM();
					ID=num;
				while(!(ID<AS608Para.PS_max));//输入ID必须小于容量的最大值
				ensure=PS_StoreChar(CharBuffer2,ID);//储存模板
				if(ensure==0x00) 
				{			
					LCD_Fill(0,100,lcddev.width,160,WHITE);					
					Show_Str_Mid(0,120,"录入指纹成功",16,240);
					PS_ValidTempleteNum(&ValidN);//读库指纹个数
					LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
					delay_ms(1500);
					LCD_Fill(0,100,240,160,WHITE);
					return ;
				}else {processnum=0;ShowErrMessage(ensure);}					
				break;				
		}
		delay_ms(400);
		if(i==5)//超过5次没有按手指则退出
		{
			LCD_Fill(0,100,lcddev.width,160,WHITE);
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
				LCD_ShowString(30,170,200,16,16,"刷指纹成功");
				//Show_Str_Mid(0,100,"刷指纹成功",16,240);				
				str=mymalloc(SRAMIN,50);
				sprintf(str,"确有此人,ID:%d  匹配得分:%d",seach.pageID,seach.mathscore);
				LCD_ShowString(30,170,200,16,16,(u8*)str);
				//Show_Str_Mid(0,140,(u8*)str,16,240);
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

////删除指纹
//void Del_FR(void)
//{
//	u8  ensure;
//	u16 ValidN,num;
//	LCD_Fill(0,100,lcddev.width,160,WHITE);
//	Show_Str_Mid(0,100,"删除指纹",16,240);
//	Show_Str_Mid(0,120,"请输入指纹ID按Enter发送",16,240);
//	Show_Str_Mid(0,140,"0=< ID <=299",16,240);
//	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_delFR);
//	num=GET_NUM();//获取返回的数值
//	if(num==0xFFFF)
//		goto MENU ; //返回主页面
//	else if(num==0xFF00)
//		ensure=PS_Empty();//清空指纹库
//	else 
//		ensure=PS_DeletChar(num,1);//删除单个指纹
//	if(ensure==0)
//	{
//		LCD_Fill(0,120,lcddev.width,160,WHITE);
//		Show_Str_Mid(0,140,"删除指纹成功",16,240);		
//	}
//  else
//		ShowErrMessage(ensure);	
//	delay_ms(1200);
//	PS_ValidTempleteNum(&ValidN);//读库指纹个数
//	LCD_ShowNum(56,80,AS608Para.PS_max-ValidN,3,16);
//MENU:	
//	LCD_Fill(0,100,lcddev.width,160,WHITE);
//	delay_ms(50);
//	AS608_load_keyboard(0,170,(u8**)kbd_menu);
//}


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