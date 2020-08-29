
#include "mcusys.h"
#include "delay.h"
#include "dht11.h"
	    
int DHT_Hpvalue;
int DHT_Tempvalue;





void DHT11_gpio_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;					 //定义GPIO结构体
 	
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//使能PC端口时钟
	
  //GPIOA1初始化设置
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//对应IO口
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIO	

}



/* Exported variables --------------------------------------------------------*/
u8 DHT_SensorData[5]={0x00, 0x00, 0x00, 0x00, 0x00};



u8 Sensor_AnswerFlag;  //收到起始标志位
u8 Sensor_ErrorFlag;   //读取传感器错误标志
u16 Sys_CNT;



u8 DHT11_DataCheck(void)
{
		u8 data_check;
		data_check = DHT_SensorData[0]+DHT_SensorData[1]+DHT_SensorData[2]+DHT_SensorData[3];	  //效验
		if(data_check!= DHT_SensorData[4])
		{
		  // UartSendByte(0x06);
			return 0;
		 }
		 DHT_Hpvalue = DHT_SensorData[0]*256 + DHT_SensorData[1];
		 DHT_Tempvalue = DHT_SensorData[2]*256 + DHT_SensorData[3];	

		if(DHT_Tempvalue&0x8000)
		{
				DHT_Tempvalue&=0x7fff;
				DHT_Tempvalue=-DHT_Tempvalue;
		}

		 DHT_Hpvalue/=10;
		 DHT_Tempvalue/=10;

		 return 1;
}



/********************************************\
|* 功能： 读传感器发送的单个字节	        *|
\********************************************/
u8 Read_SensorData(void)
  {
	u8 i;
	u16 cnt;
	u8 buffer1,tmp;
	buffer1 = 0;

	DHT11_IO_IN();
	for(i=0;i<8;i++)
	{
		cnt=0;
		while(!DHT11_IN);	//检测上次低电平是否结束
		{
		  if(++cnt >= 60000)
		   {
			  break;
		   }
		}
		//延时Min=26us Max50us 跳过数据"0" 的高电平
		delay_us(30);//延时30us
		
		//判断传感器发送数据位
		tmp =0;
		if(DHT11_IN)	 
		{
		  tmp = 1;
		}  
		cnt =0;
		while(DHT11_IN)		//等待高电平 结束
		{
		   	if(++cnt >= 60000)
			{
			  break;
			}
		}


		buffer1 <<=1;
		buffer1 |= tmp;	
	}
	return buffer1;
  }

/*
DHT11 读数据函数 
读取温度湿度值
返回0 读取失败 或 表示模块未连接
返回1 表示读取成功
*/
u8 DHT11_Read(void)
  {
	u8 i;
	DHT11_IO_OUT();

	DHT11_OUT = 1;
	delay_us(100);

	//主机拉低(Min=800US Max=20Ms) 
    DHT11_OUT = 0;
	delay_ms(15);  //延时20Ms  

	//释放总线 延时(Min=30us Max=50us)
	DHT11_OUT = 1; 	
    delay_us(40);//延时30us

	//主机设为输入 判断传感器响应信号 
	DHT11_OUT = 1; 
    
	 	  
	Sensor_AnswerFlag = 0;  // 传感器响应标志	 

	DHT11_IO_IN();
	//判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行	  
	if(DHT11_IN ==0)
	{
	   //UartSendByte(0x03);
	   Sensor_AnswerFlag = 1;//收到起始信号
	   Sys_CNT = 0;
	   //判断从机是否发出 80us 的低电平响应信号是否结束	 
	   while(!DHT11_IN)
	   {
	     if(++Sys_CNT>40000) //防止进入死循环
		 {
		   Sensor_ErrorFlag = 1;
		   return 0;
		  } 
	    }

	    Sys_CNT = 0;
	    //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
	    while(DHT11_IN)
	    {
	       if(++Sys_CNT>40000) //防止进入死循环
		   {
		     Sensor_ErrorFlag = 1;
		     return 0;
		   } 
	    } 		 
	    // 数据接收	传感器共发送40位数据 
	    // 即5个字节 高位先送  5个字节分别为湿度高位 湿度低位 温度高位 温度低位 校验和
	    // 校验和为：湿度高位+湿度低位+温度高位+温度低位
	    for(i=0;i<5;i++)
	    {
	      DHT_SensorData[i] = Read_SensorData();
	    }

		return DHT11_DataCheck();

	  }
	  else
	  {
	    Sensor_AnswerFlag = 0;	  // 未收到传感器响应	
		return 0;
	  }
}   














