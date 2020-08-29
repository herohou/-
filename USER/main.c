

/* http://zq281598585.taobao.com/  
������ӣ������鴴�Ƽ����޹�˾ ����ƽ̨ Qi-Sun
STM32 ������ �⺯������	  */

/*
  ������Կ����壺STM32F103 VC VE

   DHT11  ��ʪ�Ȳɼ� TFT��ʾ
   PA0 io ����DHT11���ݿ� 

   �����������汾  ǰ��Ҫ������ģ�����ӵ������ϣ����ҹ��纸���Ѿ����� ����������ܼ�⵽����ģ������ �������Ϊ������ģ������

TFTĿǰ֧�� 9320 9325 9328 9341�⼸��93ϵ��
TFT�ӿڶ���
���ݿ�PB0-15
TFT-CS  PE0
TFT-BL PE2   �������
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
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
   	delay_init(168);	   //��ʱ������ʼ��	  
	  LED_Init();		  	   //��ʼ����LED���ӵ�Ӳ���ӿ�       PA1��PE14
    KEY_Init();          //��ʼ������                      PA4��PA15
	  SPI1_Init();			   //��ʼ��SPI1����                  PA5��PA6��PA7��PB3��PB4��PB5 
	  SPI1_SetSpeed(SPI_BaudRatePrescaler_2);//����Ϊ18Mʱ��,����ģʽ			 
    Lcd_Init();			     //LCD  ��ʼ��                      PCBE
	  SPI_Flash_Init();  	 //SPI FLASH ��ʼ��
	 
	  GUI_Clear(White);		   //��ɫ���� 

	  My_RTC_Init();
    RTC_Set_WakeUp(RTC_WakeUpClock_CK_SPRE_16bits,0);		//����WAKE UP�ж�,1�����ж�һ��
	
	  DHT11_gpio_Init();		//DHT11 io��ʼ��            PA0

	  //����ģ�� ESPwifiģ�� ����������
	  ble_esp_test(0);	//����ģ��ģʽ
	  while(1); 
}



