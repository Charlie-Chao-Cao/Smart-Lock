基于STM32的智能门锁  

硬件平台：STM32F103；开发模式：裸机（寄存器）；开发语言：C  

使用SPI协议实现了Flash的读写功能、RC-522 RFID模块对射频卡的读写功能；  
通过串口通信实现了AS608指纹模块指纹验证和录入删除功能；  
使用C语言编写了4x4矩阵按键的代码，设计了多级菜单系统；并实现了LCD屏幕的显示；  

实现了LCD屏幕的显示和多级菜单系统；  
实现了门禁卡的验证、添加和删除功能；  
实现了指纹的验证、添加和删除功能；  
实现了密码的输入、验证、修改功能  