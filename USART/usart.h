#ifndef __USART_H
#define __USART_H
#include  "stdio.h"	
#include  "mcusys.h"

#define tbuf1 20

#define	esp_tb 10

extern u8 re_stbz;    //���յݼӱ���

extern u8 receive_end; //���ս�����־λ

extern u8 u2receive_str[tbuf1];	//Э����ջ�����

extern u8 esp_recive[esp_tb];		//10�ֽ� ESP��ʼ��ר�ý��ܻ���

extern u8 rx_num;  //���ռ�������

extern u8 re_data;	     //���ܻ����ֽ�




#define USART_REC_LEN  			200  	//�����������ֽ��� 200
//#define EN_USART1_RX 			1		//ʹ�ܣ�1��/��ֹ��0������1����
//#define EN_USART2_RX 			1		//ʹ�ܣ�1��/��ֹ��0������2����

#define tbuf0 50
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern u16 USART_RX_STA;         		//����״̬���	

extern u8  RX2_buffer[tbuf0];
extern u8 RX2_num;   //���ռ�������

//����봮���жϽ��գ��벻Ҫע�����º궨��
void uart_init(u32 bound);


void Uart1SendStr(u8 *str);
void Uart2SendStr(u8 *str);


u8 Data_compare(u8 *p);
u8 Data_compare2(u8 *p);//���esp��ʼ������
u8 Data_compare3(u8 *p);//���ble��ʼ������

#endif


