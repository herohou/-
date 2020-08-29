
#include "mcusys.h"
#include "delay.h"
#include "dht11.h"
	    
int DHT_Hpvalue;
int DHT_Tempvalue;





void DHT11_gpio_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;					 //����GPIO�ṹ��
 	
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);	//ʹ��PC�˿�ʱ��
	
  //GPIOA1��ʼ������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//��ӦIO��
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//��ͨ���ģʽ
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIO	

}



/* Exported variables --------------------------------------------------------*/
u8 DHT_SensorData[5]={0x00, 0x00, 0x00, 0x00, 0x00};



u8 Sensor_AnswerFlag;  //�յ���ʼ��־λ
u8 Sensor_ErrorFlag;   //��ȡ�����������־
u16 Sys_CNT;



u8 DHT11_DataCheck(void)
{
		u8 data_check;
		data_check = DHT_SensorData[0]+DHT_SensorData[1]+DHT_SensorData[2]+DHT_SensorData[3];	  //Ч��
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
|* ���ܣ� �����������͵ĵ����ֽ�	        *|
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
		while(!DHT11_IN);	//����ϴε͵�ƽ�Ƿ����
		{
		  if(++cnt >= 60000)
		   {
			  break;
		   }
		}
		//��ʱMin=26us Max50us ��������"0" �ĸߵ�ƽ
		delay_us(30);//��ʱ30us
		
		//�жϴ�������������λ
		tmp =0;
		if(DHT11_IN)	 
		{
		  tmp = 1;
		}  
		cnt =0;
		while(DHT11_IN)		//�ȴ��ߵ�ƽ ����
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
DHT11 �����ݺ��� 
��ȡ�¶�ʪ��ֵ
����0 ��ȡʧ�� �� ��ʾģ��δ����
����1 ��ʾ��ȡ�ɹ�
*/
u8 DHT11_Read(void)
  {
	u8 i;
	DHT11_IO_OUT();

	DHT11_OUT = 1;
	delay_us(100);

	//��������(Min=800US Max=20Ms) 
    DHT11_OUT = 0;
	delay_ms(15);  //��ʱ20Ms  

	//�ͷ����� ��ʱ(Min=30us Max=50us)
	DHT11_OUT = 1; 	
    delay_us(40);//��ʱ30us

	//������Ϊ���� �жϴ�������Ӧ�ź� 
	DHT11_OUT = 1; 
    
	 	  
	Sensor_AnswerFlag = 0;  // ��������Ӧ��־	 

	DHT11_IO_IN();
	//�жϴӻ��Ƿ��е͵�ƽ��Ӧ�ź� �粻��Ӧ����������Ӧ����������	  
	if(DHT11_IN ==0)
	{
	   //UartSendByte(0x03);
	   Sensor_AnswerFlag = 1;//�յ���ʼ�ź�
	   Sys_CNT = 0;
	   //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����	 
	   while(!DHT11_IN)
	   {
	     if(++Sys_CNT>40000) //��ֹ������ѭ��
		 {
		   Sensor_ErrorFlag = 1;
		   return 0;
		  } 
	    }

	    Sys_CNT = 0;
	    //�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
	    while(DHT11_IN)
	    {
	       if(++Sys_CNT>40000) //��ֹ������ѭ��
		   {
		     Sensor_ErrorFlag = 1;
		     return 0;
		   } 
	    } 		 
	    // ���ݽ���	������������40λ���� 
	    // ��5���ֽ� ��λ����  5���ֽڷֱ�Ϊʪ�ȸ�λ ʪ�ȵ�λ �¶ȸ�λ �¶ȵ�λ У���
	    // У���Ϊ��ʪ�ȸ�λ+ʪ�ȵ�λ+�¶ȸ�λ+�¶ȵ�λ
	    for(i=0;i<5;i++)
	    {
	      DHT_SensorData[i] = Read_SensorData();
	    }

		return DHT11_DataCheck();

	  }
	  else
	  {
	    Sensor_AnswerFlag = 0;	  // δ�յ���������Ӧ	
		return 0;
	  }
}   














