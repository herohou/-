										  

/**********************************************************************************

**********************************************************************************/
#include  "mcusys.h"
#include  "usart.h"
#include  "stdio.h"
#include  "string.h"
#include  "BleEsp.h"

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout; 
//__use_no_semihosting was requested, but _ttywrch was 
_ttywrch(int ch) 
{ 
ch = ch; 
}       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);   
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/



//#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	  



//��ʼ��IO ����1 2
//bound:������
void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;				   //IO�����ýṹ��
	USART_InitTypeDef USART_InitStructure;				 //�������ýṹ��
	NVIC_InitTypeDef NVIC_InitStructure;				   //�ж����ýṹ��

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//ʹ��USART1ʱ��

	//����1��Ӧ���Ÿ���ӳ�� 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10����ΪUSART1

	//USART1�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
   
	//����2��Ӧ���Ÿ���ӳ�� 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART1); //GPIOA2����ΪUSART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART1); //GPIOA3����ΪUSART2

	//USART1�˿�����
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2��GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA2��PA3

  //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;                //���������� һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;     //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;        //����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	   //�շ�ģʽ   ����������ý��� �����

  USART_Init(USART1, &USART_InitStructure); //��ʼ������
	USART_Init(USART2, &USART_InitStructure);

  USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
	USART_Cmd(USART2, ENABLE);  //ʹ�ܴ���2 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ClearFlag(USART2, USART_FLAG_TC);

//#if EN_USART1_RX		  //���ʹ���˽���  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
   
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�  ���յ����ݽ����ж� 
//#endif

//#if EN_USART2_RX		  //���ʹ���˽���  
   
	//Usart2 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
   
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�	 ���յ����ݽ����ж�
//#endif

}



//���� ������һ��16λ����USART_RX_STA ����ʾ �ɼ������ݳ���  ����״̬�� �൱��һ���Ĵ���
//USART_RX_STA     15		    14	         13-0
//				 �������	���յ�0x0d	  ���յ����ݳ���  û���ռ�1 ��ʾ����һ���ֽ�
//USART_RX_STA=0 ��Ϊ����������׼��

//���ڽ����жϵ�ǰ���� ���ݵ�����Իس�Ϊ׼  ��  0x0d 0x0a  

void USART1_IRQHandler(void)                	//����1�ж���Ӧ����		 �����ֲ�����㶨��
	{
	u8 Res;													//�����ڽ��յ�����  RXNE������1 
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 			  //���յ��س��ĺ��ֽ�  ��λ״̬�Ĵ��� 
				}
			else //��û�յ�0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;					 //���յ��س���ǰһ�ֽ�  ��λ״̬�Ĵ���
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;			//�����յ����� ����������
					USART_RX_STA++;									//����+1 Ϊ��һ����׼��
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			}   		 
     } 

} 
//#endif	




 //����1�����ַ���
void Uart1SendStr(u8 *s)
{
  while (*s)                  //����ַ���������־
    {
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,*s++);

    }

}






u8  RX2_buffer[tbuf0];


//����2�����ַ���
void Uart2SendStr(u8 *s)
{
  while (*s)                  //����ַ���������־
    {
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,*s++);

    }

}


//u8 RX_num=0;   //���ռ�������
//
//
//
//void USART2_IRQHandler(void)
//{	    
// 
// 	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //���յ�����
//	{	 
//	 			 
//		RX_buffer[RX_num] =USART_ReceiveData(USART2); 	//��ȡ���յ�������
//		RX_num++;               
//		if(RX_num>tbuf) RX_num = 0;   
//	}  											 
//} 
//







u8 u2receive_str[tbuf1];//����20�ֽڵĽ��ܻ�����

u8 esp_recive[esp_tb];		//10�ֽ� ESP��ʼ��ר�ý��ܻ���

u8 re_data,	     //���ܻ����ֽ�
   re_stbz=0;    //���յݼӱ��� 
u8 s_QG[2];	     //���ַ�����
u8 receive_end=0;//���ս�����־λ


u8 RX2_num=0;   //���ռ�������


void USART2_IRQHandler(void)                	//����2�ж���Ӧ����		 �����ֲ�����㶨��
	{
 
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		re_data =USART_ReceiveData(USART2);//(USART1->DR);	//��ȡ���յ�������
		RX2_buffer[RX2_num] =re_data; 	//��ȡ���յ�������
		RX2_num++;               
		if(RX2_num>tbuf0) RX2_num = 0;  		

		   if(esp_init==0) //esp��Ҫ��ʼ��
		   {
		   	esp_recive[re_stbz]=re_data;  //��������
			re_stbz++;
			if(re_stbz>esp_tb) re_stbz=0;
		   }
		
		   else if(esp_init==1)//espģʽ�� esp��ʼ��ok ����  ����ģʽ
		   {
			   ////////////////////////////////////////////////////////////////////
			   //����ָ��Э�鴦������
			   if(re_data=='Q'&&re_stbz==0)  s_QG[0]='Q'; 	   //��һ�������ַ� ΪQ  
			   if(re_data=='G'&&re_stbz==1)	 s_QG[1]='G';	   //�ڶ��������ַ� ΪG
			
			   if((s_QG[0]=='Q')&&(s_QG[1]=='G')&&(re_stbz>1)&&(re_stbz<(tbuf1+2)))		 //�ѽ��յ����ַ� ���ڵݼӱ�����Χ�ڿ�ʼ��������
			   {
				if(re_data=='/')							   //�����ս����ַ� Ϊб��/
				{
				  receive_end=1;		 //���ս�����־λ
				  s_QG[0]=s_QG[1]=0;	 //�� ���ַ�����
				}
			   	else u2receive_str[re_stbz-2]=re_data ;		   //���û���յ������� ���������ݷ��뻺��������
			   }
			
			   re_stbz++;									   //���յݼӱ�������
			
			   if((s_QG[0]!='Q')||(s_QG[1]!='G')&&(re_stbz>1)) //˵���������ݲ���
			   {
			   	re_stbz=0; 			 //�� ���յݼӱ���
				s_QG[0]=s_QG[1]=0;	 //�� ���ַ�����
			   }
			
			   if(receive_end==1) re_stbz=0;  //������־λ��λ �� ���յݼӱ���
			
			   if((s_QG[0]=='Q')&&(s_QG[1]=='G')&&(re_stbz>=(tbuf1+2))) //˵���������ݳ��������� ���� ����
			   {
			    re_stbz=0; 			 //�� ���յݼӱ���
				s_QG[0]=s_QG[1]=0;	 //�� ���ַ�����   
				memset(u2receive_str, 0, tbuf1);//�建������
			   }
			   ////////////////////////////////////////////////////////////////////
		   }
  		 
     } 

} 



//ָ���ַ����뻺���������ݽ������ݱȽ�
//*p Ҫ�Ƚϵ�ָ���ַ���ָ������
//���أ�1 ����һ��  0 ���ݲ�һ�� 
u8 Data_compare(u8 *p)
{ 
    if(strstr((const char*) u2receive_str,(const char*)p)!=NULL)
	    return 1;
	else
		return 0;
}

//���esp��ʼ������
//ָ���ַ����뻺���������ݽ������ݱȽ�
//*p Ҫ�Ƚϵ�ָ���ַ���ָ������
//���أ�1 ����һ��  0 ���ݲ�һ�� 
u8 Data_compare2(u8 *p)
{ 
    if(strstr((const char*)esp_recive,(const char*)p)!=NULL)
	    return 1;
	else
		return 0;
}




//ָ���ַ����뻺���������ݽ������ݱȽ�
//*p Ҫ�Ƚϵ�ָ���ַ���ָ������
//���أ�1 ����һ��  0 ���ݲ�һ�� 
u8 Data_compare3(u8 *p)
{ 
    if(strstr((const char*)RX2_buffer,(const char*)p)!=NULL)
	    return 1;
	else
		return 0;
}






































