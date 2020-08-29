#ifndef __USART_H
#define __USART_H
#include  "stdio.h"	
#include  "mcusys.h"

#define tbuf1 20

#define	esp_tb 10

extern u8 re_stbz;    //接收递加变量

extern u8 receive_end; //接收结束标志位

extern u8 u2receive_str[tbuf1];	//协议接收缓存区

extern u8 esp_recive[esp_tb];		//10字节 ESP初始化专用接受缓存

extern u8 rx_num;  //接收计数变量

extern u8 re_data;	     //接受缓存字节




#define USART_REC_LEN  			200  	//定义最大接收字节数 200
//#define EN_USART1_RX 			1		//使能（1）/禁止（0）串口1接收
//#define EN_USART2_RX 			1		//使能（1）/禁止（0）串口2接收

#define tbuf0 50
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	

extern u8  RX2_buffer[tbuf0];
extern u8 RX2_num;   //接收计数变量

//如果想串口中断接收，请不要注释以下宏定义
void uart_init(u32 bound);


void Uart1SendStr(u8 *str);
void Uart2SendStr(u8 *str);


u8 Data_compare(u8 *p);
u8 Data_compare2(u8 *p);//真对esp初始化缓存
u8 Data_compare3(u8 *p);//真对ble初始化缓存

#endif


