

/* http://zq281598585.taobao.com/  
启光电子：启光灵创科技有限公司 电商平台 Qi-Sun
STM32 开发板 库函数部分	  */

/*
  代码真对开发板：STM32F103 VC VE

   DHT11  温湿度采集 TFT显示
   PA0 io 连接DHT11数据口 

   蓝牙物联网版本  前提要有蓝牙模块连接到主板上，并且供电焊点已经连接 这样程序才能检测到蓝牙模块有无 否则会认为有蓝牙模块在线

TFT目前支持 9320 9325 9328 9341这几款93系列
TFT接口定义
数据口PB0-15
TFT-CS  PE0
TFT-BL PE2   背光控制
TFT-RST PE4
TFT-RS  PC4
TFT-WR  PC2
TFT-RD  PC0


*/		

				
#include  "mcusys.h"
#include  "delay.h"
#include  "led.h"
#include  "tft.h"
#include  "key.h"
#include  "flash.h"
#include  "gui.h"
#include  "dht11.h"
#include  "rtc.h"
#include  "BleEsp.h"
#include  "spi.h"



int main(void)
{
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
   	delay_init(168);	   //延时函数初始化	  
	  LED_Init();		  	   //初始化与LED连接的硬件接口       PA1、PE14
    KEY_Init();          //初始化按键                      PA4、PA15
	  SPI1_Init();			   //初始化SPI1总线                  PA5、PA6、PA7、PB3、PB4、PB5 
	  SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//设置为18M时钟,高速模式			 
    Lcd_Init();			     //LCD  初始化                      PCBE
	  SPI_Flash_Init();  	 //SPI FLASH 初始化
	 
	  GUI_Clear(White);		   //白色清屏 

	  My_RTC_Init();
    RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,0);		//配置WAKE UP中断,1秒钟中断一次
	
	  DHT11_gpio_Init();		//DHT11 io初始化            PA0

	  //蓝牙模块 ESPwifi模块 测试主函数
	  ble_esp_test(0);	//蓝牙模块模式
	  while(1); 
}



