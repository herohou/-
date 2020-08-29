										  

/**********************************************************************************

**********************************************************************************/
#include  "mcusys.h"
#include  "usart.h"
#include  "stdio.h"
#include  "string.h"
#include  "BleEsp.h"

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
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
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);   
	return ch;
}
#endif 

/*使用microLib的方法*/
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



//#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  



//初始化IO 串口1 2
//bound:波特率
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;				   //IO口配置结构体
	USART_InitTypeDef USART_InitStructure;				 //串口配置结构体
	NVIC_InitTypeDef NVIC_InitStructure;				   //中断配置结构体

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟

	//串口1对应引脚复用映射 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1

	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
   
	//串口2对应引脚复用映射 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART1); //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART1); //GPIOA3复用为USART2

	//USART1端口配置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA2，PA3

  //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;                //波特率设置 一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;     //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;        //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	   //收发模式   这里可以配置仅发 或仅收

  USART_Init(USART1, &USART_InitStructure); //初始化串口
	USART_Init(USART2, &USART_InitStructure);

  USART_Cmd(USART1, ENABLE);  //使能串口1 
	USART_Cmd(USART2, ENABLE);  //使能串口2 
	
	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ClearFlag(USART2, USART_FLAG_TC);

//#if EN_USART1_RX		  //如果使能了接收  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断  接收到数据进入中断 
//#endif

//#if EN_USART2_RX		  //如果使能了接收  
   
	//Usart2 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
   
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断	 接收到数据进入中断
//#endif

}



//这里 定义了一个16位数据USART_RX_STA 来表示 采集的数据长度  数据状态等 相当于一个寄存器
//USART_RX_STA     15		    14	         13-0
//				 接收完成	接收到0x0d	  接收的数据长度  没接收加1 表示多了一个字节
//USART_RX_STA=0 则为接收数据做准备

//串口进入中断的前提是 数据的最后以回车为准  即  0x0d 0x0a  

void USART1_IRQHandler(void)                	//串口1中断响应程序		 其名字不能随便定义
	{
	u8 Res;													//当串口接收到数据  RXNE将被置1 
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
			{
			if(USART_RX_STA&0x4000)//接收到了0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 			  //接收到回车的后字节  置位状态寄存器 
				}
			else //还没收到0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;					 //接收到回车的前一字节  置位状态寄存器
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;			//将接收的数据 存入数组中
					USART_RX_STA++;									//长度+1 为下一次做准备
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
					}		 
				}
			}   		 
     } 

} 
//#endif	




 //串口1发送字符串
void Uart1SendStr(u8 *s)
{
  while (*s)                  //检测字符串结束标志
    {
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART1,*s++);

    }

}






u8  RX2_buffer[tbuf0];


//串口2发送字符串
void Uart2SendStr(u8 *s)
{
  while (*s)                  //检测字符串结束标志
    {
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);	  
		USART_SendData(USART2,*s++);

    }

}


//u8 RX_num=0;   //接收计数变量
//
//
//
//void USART2_IRQHandler(void)
//{	    
// 
// 	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收到数据
//	{	 
//	 			 
//		RX_buffer[RX_num] =USART_ReceiveData(USART2); 	//读取接收到的数据
//		RX_num++;               
//		if(RX_num>tbuf) RX_num = 0;   
//	}  											 
//} 
//







u8 u2receive_str[tbuf1];//定义20字节的接受缓存区

u8 esp_recive[esp_tb];		//10字节 ESP初始化专用接受缓存

u8 re_data,	     //接受缓存字节
   re_stbz=0;    //接收递加变量 
u8 s_QG[2];	     //首字符数组
u8 receive_end=0;//接收结束标志位


u8 RX2_num=0;   //接收计数变量


void USART2_IRQHandler(void)                	//串口2中断响应程序		 其名字不能随便定义
	{
 
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		re_data =USART_ReceiveData(USART2);//(USART1->DR);	//读取接收到的数据
		RX2_buffer[RX2_num] =re_data; 	//读取接收到的数据
		RX2_num++;               
		if(RX2_num>tbuf0) RX2_num = 0;  		

		   if(esp_init==0) //esp需要初始化
		   {
		   	esp_recive[re_stbz]=re_data;  //接收数据
			re_stbz++;
			if(re_stbz>esp_tb) re_stbz=0;
		   }
		
		   else if(esp_init==1)//esp模式下 esp初始化ok 或者  蓝牙模式
		   {
			   ////////////////////////////////////////////////////////////////////
			   //启光指令协议处理区域
			   if(re_data=='Q'&&re_stbz==0)  s_QG[0]='Q'; 	   //第一接收首字符 为Q  
			   if(re_data=='G'&&re_stbz==1)	 s_QG[1]='G';	   //第二接收首字符 为G
			
			   if((s_QG[0]=='Q')&&(s_QG[1]=='G')&&(re_stbz>1)&&(re_stbz<(tbuf1+2)))		 //已接收到首字符 并在递加变量范围内开始接收数据
			   {
				if(re_data=='/')							   //最后接收结束字符 为斜杠/
				{
				  receive_end=1;		 //接收结束标志位
				  s_QG[0]=s_QG[1]=0;	 //清 首字符数组
				}
			   	else u2receive_str[re_stbz-2]=re_data ;		   //如果没有收到结束符 将接收数据放入缓存数组中
			   }
			
			   re_stbz++;									   //接收递加变量增加
			
			   if((s_QG[0]!='Q')||(s_QG[1]!='G')&&(re_stbz>1)) //说明本组数据不对
			   {
			   	re_stbz=0; 			 //清 接收递加变量
				s_QG[0]=s_QG[1]=0;	 //清 首字符数组
			   }
			
			   if(receive_end==1) re_stbz=0;  //结束标志位置位 清 接收递加变量
			
			   if((s_QG[0]=='Q')&&(s_QG[1]=='G')&&(re_stbz>=(tbuf1+2))) //说明接收数据超过缓存区 或者 出错
			   {
			    re_stbz=0; 			 //清 接收递加变量
				s_QG[0]=s_QG[1]=0;	 //清 首字符数组   
				memset(u2receive_str, 0, tbuf1);//清缓存数组
			   }
			   ////////////////////////////////////////////////////////////////////
		   }
  		 
     } 

} 



//指定字符串与缓存数组数据进行数据比较
//*p 要比较的指定字符串指针数据
//返回：1 数据一致  0 数据不一致 
u8 Data_compare(u8 *p)
{ 
    if(strstr((const char*) u2receive_str,(const char*)p)!=NULL)
	    return 1;
	else
		return 0;
}

//真对esp初始化缓存
//指定字符串与缓存数组数据进行数据比较
//*p 要比较的指定字符串指针数据
//返回：1 数据一致  0 数据不一致 
u8 Data_compare2(u8 *p)
{ 
    if(strstr((const char*)esp_recive,(const char*)p)!=NULL)
	    return 1;
	else
		return 0;
}




//指定字符串与缓存数组数据进行数据比较
//*p 要比较的指定字符串指针数据
//返回：1 数据一致  0 数据不一致 
u8 Data_compare3(u8 *p)
{ 
    if(strstr((const char*)RX2_buffer,(const char*)p)!=NULL)
	    return 1;
	else
		return 0;
}






































