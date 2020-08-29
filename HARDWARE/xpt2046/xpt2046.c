
 
#include  "mcusys.h"
#include "math.h"
#include "xpt2046.h"
#include "delay.h"
#include "spi.h"
#include "gui.h"
#include "tft.h"
#include "flash.h"
#include "stdlib.h"
#include "math.h"
#include "led.h"
#include "key.h"

/*

  xpt2046
  AD_DCLK  PC5
  AD_CS    PC6
  AD_DIN   PC7
  AD_OUT   PC3			��ȡ������������
  AD_PEN   PC1


*/



u8 flash_tp[9];		//flash ����У׼���ݻ�����



//������У׼����
//���û��У׼��flash��  ʹ��Ĭ��ֵ
struct T_i T_flash=
{
 -657,		//�������� ��ֵҪ����10000������ʵֵ  ��ΪΪ�˷��㺯������ ����ʱ����10000 
 -901,		//�������� ��ֵҪ����10000������ʵֵ  ��ΪΪ�˷��㺯������ ����ʱ����10000 
  251,
  357,
};


//4KbytesΪһ��Sector
//16������Ϊ1��Block
//W25X16
//����Ϊ2M�ֽ�,����32��Block,512��Sector 

//��ȡFLASH�е�У׼�������� ����200 ��ַ0-7 �津������  8���־
//��ȡǰ  ����Ҫ�ж� ��ַ8  ��У׼��־λ �Ƿ�Ϊ0xf4
//sta ������ �Ƿ��flash�ж�ȡУ׼��������       1 ��ȡ   0 ����ȡ 
//����ֵ ���Ϊ0  ˵���Ѿ�У׼���������� Ȼ��ͨ��sta�ж��Ƿ���ȡflash�е�У׼���� ��Ϊ�����жϱ�׼
//����ֵ ���Ϊ1  ˵��û��У׼��   ֱ������У׼�����ṹ�� T_flash����ı������������ֵҲ��У׼��ֱ����д��������ģ�
u8 readflash_t(u8 sta)
{
SPI_Flash_Read(flash_tp,xpt_flash_sector*4096,9);


 if(sta==1&&flash_tp[8]==0xF4)    //��ȡУ׼���� 1 ��У׼�� ������У׼�� �ýṹ�屾����
 {
	T_flash.xi=((u16)flash_tp[1]<<8)+flash_tp[0];		  //����ȡ��У׼���ݸ��µ�������
	T_flash.yi=((u16)flash_tp[3]<<8)+flash_tp[2];
	T_flash.a=((u16)flash_tp[5]<<8)+flash_tp[4];
	T_flash.b=((u16)flash_tp[7]<<8)+flash_tp[6];
	return 0;              //��ȡ�ɹ�
 }
 else if(sta==0&&flash_tp[8]==0xF4) return 0;
 return 1;                 //û��У׼
}





//��ʼ�� ����оƬXPT2046 SPIƬѡIO ���ж�IO
void XPT2046_Init(void)
{	
  	GPIO_InitTypeDef GPIO_InitStructure;

		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//ʹ��GPIOB,C,Fʱ��

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1| GPIO_Pin_3;//PC1/PC3 ����Ϊ��������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//����ģʽ
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//�������
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
		GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_5|GPIO_Pin_7;//PC5 PC7����Ϊ�������
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//���ģʽ
		GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��
}


//Ĭ��Ϊtouchtype=0������.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;



//SPIд����
//������ICд��1byte����    
//num:Ҫд�������
void TP_Write_Byte(u8 num)    
{  
	u8 count=0;   
	for(count=0;count<8;count++)  
	{ 	  
		if(num&0x80)TP_DIN=1;  
		else TP_DIN=0;   
		num<<=1;    
		TP_CLK=0; 
		delay_us(1);
		TP_CLK=1;		//��������Ч	        
	}		 			    
} 




//SPI������ 
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������	   
u16 TP_Read_AD(u8 CMD)	  
{ 	 
	u8 count=0; 	  
	u16 Num=0; 
	TP_CLK=0;		//������ʱ�� 	 
	TP_DIN=0; 	//����������
	TP_CS=0; 		//ѡ�д�����IC
	TP_Write_Byte(CMD);//����������
	delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
	TP_CLK=0; 	     	    
	delay_us(1);    	   
	TP_CLK=1;		//��1��ʱ�ӣ����BUSY
	delay_us(1);    
	TP_CLK=0; 	     	    
	for(count=0;count<16;count++)//����8λ���� 
	{ 				  
		Num<<=1; 	 
		TP_CLK=0;	//�½�����Ч  	    	   
		delay_us(1);    
 		TP_CLK=1;
 		if(TP_OUT)Num++; 		 
	}  	
	Num>>=4;   	//ֻ�и�12λ��Ч.
	TP_CS=1;		//�ͷ�Ƭѡ	 
	return(Num);   
}



//��ȡһ������ֵ(x����y)
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ 
//xy:ָ�CMD_RDX/CMD_RDY��
//����ֵ:����������
#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
u16 TP_Read_XOY(u8 xy)
{
	u16 i, j;
	u16 buf[READ_TIMES];
	u16 sum=0;
	u16 temp;
	for(i=0;i<READ_TIMES;i++)buf[i]=TP_Read_AD(xy);		 		    
	for(i=0;i<READ_TIMES-1; i++)//����
	{
		for(j=i+1;j<READ_TIMES;j++)
		{
			if(buf[i]>buf[j])//��������
			{
				temp=buf[i];
				buf[i]=buf[j];
				buf[j]=temp;
			}
		}
	}	  
	sum=0;
	for(i=LOST_VAL;i<READ_TIMES-LOST_VAL;i++)sum+=buf[i];
	temp=sum/(READ_TIMES-2*LOST_VAL);
	return temp;   
} 




//��ȡx,y����
//��Сֵ��������100.
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
u8 TP_Read_XY(u16 *x,u16 *y)
{
	u16 xtemp,ytemp;			 	 		  
	xtemp=TP_Read_XOY(CMD_RDX);
	ytemp=TP_Read_XOY(CMD_RDY);	  												   
	//if(xtemp<100||ytemp<100)return 0;//����ʧ��
	*x=xtemp;
	*y=ytemp;
	return 1;//�����ɹ�
}







//SPI���߷� ��ȡXPT2046����
//�ɼ�����ʱxpt������������� x y  
//�˺���û��ת����LCD��ʵ������ 
//ΪУ׼��������ȡ
//���� xpt2046ad.x  ����x����
//     xpt2046ad.y  ����y����
//     xpt2046ad.flag �ɼ��ɹ���־
struct TFT_Pointer xpt2046ad()		                                 
{
 struct TFT_Pointer result;
// u8   ax[8];
 u16 x1,y1;
 u16 x2,y2;

 result.x=0;
 result.y=0;
 result.flag=0;
 
#define ERR_RANGE 20 //��Χ 
  
 if(xpt2046_isClick==0)
 {	 
  delay_ms(1);
  if(xpt2046_isClick==0)
  {
    TP_CS=0; 		//��Ƭѡ
   
   /*�������16ʱ�����ڲɼ�  ��Ϊ �˴������ܲ��õ���SPI����
     ��SPI������ֻ��8λ����  XPT2046��AD�ֱ���Ϊ12λ  
	 ����8λ��SPI�����ǲ��е�
	 ���� ����XPT2046�ֲ��� 16ʱ������ ʱ��ͼ ���Կ���
	 ���Ͳɼ�����  ����һ��SPI���ݺ� �ڷ��Ϳչ��ܵ�SPI����  �ͻ��ʣ�µĲ��ֽ��յ�
	 �����Ƚ��յ� �ǵ��ֽ�����  �ڶ��ν��յ��Ǹ��ֽ�����  ��λ�� ����12λ��ADֵ   
   */

	TP_Read_XY(&x1,&y1);
	TP_Read_XY(&x2,&y2);


if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-ERR_RANGE��
    &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
   {
   	result.flag=1;			//�򿪱�־λ
	result.x=(x1+x2)/2;
	result.y=(y1+y2)/2;
   }
   else result.flag=0;

   TP_CS=1; 		//��Ƭѡ
   
  }
 } 

 return result;
}
												




//��������ת����ʵ�����꺯����
//���� result.x���� result.y���� 
struct TFT_Pointer TFT_Cm(void)		                                  
{
 u16 a,b;				//��ʱ����
 struct TFT_Pointer result;

 result=xpt2046ad();	 //��ȡADֵ



//������X Y
   a=result.x;
   b=result.y;
/* ���������㹫ʽ
   lcdx=xa*tpx+xb;
   lcdy=ya*tpy+yb;
   lcdx,lcdyΪ������  tpx,tpyΪ������ֵ xa,yaΪ��������  xb,ybΪƫ����

   ���㷽�� 
   ����Ļ��ָ��lcdx,lcdyλ�û���ʮ��ͼ�� �ֱ�����Ļ�ϵ�4����λ��
   �ô����ʷֱ��� �õ����еĴ���ֵ
   ��������Ĺ�ʽ ����	xa,ya  xb,yb  ��������ʹ�ô��������ĻУ׼
  */
	 
		 result.x=240-(0.065894*a-16);			//���õ���ADֵ���빫ʽ ����lcd����x y���� 
		 result.y=320-(0.084031*b-14);		//���y�� ���µߵ� ��Ϊ	result.y=0.084031*b-14;

 return result;								//��������ֵ
}








