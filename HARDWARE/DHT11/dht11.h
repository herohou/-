#ifndef __DHT11_H
#define __DHT11_H	    
#include "mcusys.h"


extern int DHT_Hpvalue;
extern int DHT_Tempvalue;


//IO方向设置
//#define DHT11_IO_IN()  {GPIOA->CRL&=0XFFFFFFF0;GPIOA->CRL|=0X00000008;}
//#define DHT11_IO_OUT() {GPIOA->CRL&=0XFFFFFFF0;GPIOA->CRL|=0X00000003;}


#define DHT11_IO_IN()  {GPIOA->MODER&=~3;GPIOA->MODER|=0;}	  //PA0 输入
#define DHT11_IO_OUT() {GPIOA->MODER&=~3;GPIOA->MODER|=1;}    //PA0 输出

#define	DHT11_IN   PAin(0)    //数据端口	PA0
#define	DHT11_OUT  PAout(0)  //数据端口	PA0


void DHT11_gpio_Init(void);
u8 DHT11_Read(void);	//读取温湿度值


#endif




