
#include "mcusys.h"
#include "delay.h"
#include "rtc.h" 	
#include "led.h"
#include "usart.h" 	    
#include "gui.h" 
#include "xpt2046.h"



NVIC_InitTypeDef   NVIC_InitStructure;

//RTCʱ������
//hour,min,sec:Сʱ,����,����
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC��������
//year,month,date:��(0~99),��(1~12),��(0~31)
//week:����(1~7,0,�Ƿ�!)
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}

//RTC��ʼ��
//����ֵ:0,��ʼ���ɹ�;
//       1,LSE����ʧ��;
//       2,�����ʼ��ģʽʧ��;
u8 My_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry=0X1FFF; 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//ʹ��PWRʱ��
	PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ������� 
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//�Ƿ��һ������?
	{
		RCC_LSEConfig(RCC_LSE_ON);//LSE ����    
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//���ָ����RCC��־λ�������,�ȴ����پ������
			{
			retry++;
			delay_ms(10);
			}
		if(retry==0)return 1;		//LSE ����ʧ��. 
			
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��    
		RCC_RTCCLKCmd(ENABLE);	//ʹ��RTCʱ�� 

    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC�첽��Ƶϵ��(1~0X7F)
    RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTCͬ����Ƶϵ��(0~7FFF)
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC����Ϊ,24Сʱ��ʽ
    RTC_Init(&RTC_InitStructure);
 
		RTC_Set_Time(23,59,56,RTC_H12_AM);	//����ʱ��
		RTC_Set_Date(20,3,2,1);		//��������
	 
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//����Ѿ���ʼ������
	} 
 
	return 0;
}

//��������ʱ��(����������,24Сʱ��)
//week:���ڼ�(1~7) @ref  RTC_Alarm_Definitions
//hour,min,sec:Сʱ,����,����
void RTC_Set_AlarmA(u8 week,u8 hour,u8 min,u8 sec)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);//�ر�����A 
	
  RTC_TimeTypeInitStructure.RTC_Hours=hour;//Сʱ
	RTC_TimeTypeInitStructure.RTC_Minutes=min;//����
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;//��
	RTC_TimeTypeInitStructure.RTC_H12=RTC_H12_AM;
  
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay=week;//����
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel=RTC_AlarmDateWeekDaySel_WeekDay;//��������
	RTC_AlarmTypeInitStructure.RTC_AlarmMask=RTC_AlarmMask_None;//��ȷƥ�����ڣ�ʱ����
	RTC_AlarmTypeInitStructure.RTC_AlarmTime=RTC_TimeTypeInitStructure;
  RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_A,&RTC_AlarmTypeInitStructure);
 
	
	RTC_ClearITPendingBit(RTC_IT_ALRA);//���RTC����A�ı�־
  EXTI_ClearITPendingBit(EXTI_Line17);//���LINE17�ϵ��жϱ�־λ 
	
	RTC_ITConfig(RTC_IT_ALRA,ENABLE);//��������A�ж�
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);//��������A 
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;//LINE17
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE17
  EXTI_Init(&EXTI_InitStructure);//����

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
}

//�����Ի��Ѷ�ʱ������  
/*wksel:  @ref RTC_Wakeup_Timer_Definitions
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
*/
//cnt:�Զ���װ��ֵ.����0,�����ж�.
void RTC_Set_WakeUp(u32 wksel,u16 cnt)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RTC_WakeUpCmd(DISABLE);//�ر�WAKE UP
	
	RTC_WakeUpClockConfig(wksel);//����ʱ��ѡ��
	
	RTC_SetWakeUpCounter(cnt);//����WAKE UP�Զ���װ�ؼĴ���
	
	
	RTC_ClearITPendingBit(RTC_IT_WUT); //���RTC WAKE UP�ı�־
  EXTI_ClearITPendingBit(EXTI_Line22);//���LINE22�ϵ��жϱ�־λ 
	 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);//����WAKE UP ��ʱ���ж�
	RTC_WakeUpCmd( ENABLE);//����WAKE UP ��ʱ����
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;//LINE22
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//�ж��¼�
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //�����ش��� 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//ʹ��LINE22
  EXTI_Init(&EXTI_InitStructure);//����
 
 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//�����ȼ�2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure);//����
}

//RTC�����жϷ�����
void RTC_Alarm_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A�ж�?
	{
		RTC_ClearFlag(RTC_FLAG_ALRAF);//����жϱ�־
		printf("ALARM A!\r\n");
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//����ж���17���жϱ�־ 											 
}

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

//RTC WAKE UP�жϷ�����
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF)==SET)//WK_UP�ж�?
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);	//����жϱ�־
		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);  //��ȡʱ����
		//led_d2=!led_d2; 
	}   
	EXTI_ClearITPendingBit(EXTI_Line22);//����ж���22���жϱ�־ 								
}





u8 time;				 //��ʱ����±�־

// ʱ�� ������ ���� ������ʾ����
//x y  ��ʾ��ʱ��ĳ�ʼ����  ��������������ʾ  ������ ������������ʾ
//��
//2020-12-12
//Friday
//20:25:32
//dcolor ������ɫ	  bgcolor  ������ɫ
//mode  Ϊ1ʱ ֧�� ������ ������ʾ  ���� ֻ��ʾʱ��
void Time_scan(u8 x,u16 y,u16 dcolor,u16 bgcolor,u8 mode)
{

  if(time!=RTC_TimeStruct.RTC_Seconds)
   {
   	 time=RTC_TimeStruct.RTC_Seconds;
		 RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct); //��ȡ������

	   GUI_sprintf_hzstr16x(x+16,y,":",dcolor,bgcolor);  //��ʾ�� �� ʱ ֮��� ������	
	   GUI_sprintf_hzstr16x(x+40,y,":",dcolor,bgcolor);
				
		 number10(x,y,RTC_TimeStruct.RTC_Hours,dcolor,bgcolor);		      //Сʱ							  
		 number10(x+24,y,RTC_TimeStruct.RTC_Minutes,dcolor,bgcolor);		//����							  
	   number10(x+48,y,RTC_TimeStruct.RTC_Seconds,dcolor,bgcolor);		//��
						 
	   if(mode==1)		 //�ж� �Ƿ��� ���� ������ʾ   modeΪ1��ʾ����
	   {			
					 
		   switch(RTC_DateStruct.RTC_WeekDay)					  //����
		   { 

				 case 1:
						GUI_sprintf_hzstr16x(x,y-16,"Monday",dcolor,bgcolor);
					break;
				 case 2:
						GUI_sprintf_hzstr16x(x,y-16,"Tuesday",dcolor,bgcolor);
					break;
				 case 3:
						GUI_sprintf_hzstr16x(x,y-16,"Wednesday",dcolor,bgcolor);
					break;
				 case 4:
						GUI_sprintf_hzstr16x(x,y-16,"Thursday",dcolor,bgcolor);
					break;
				 case 5:
						GUI_sprintf_hzstr16x(x,y-16,"Friday",dcolor,bgcolor);
					break;
				 case 6:
						GUI_sprintf_hzstr16x(x,y-16,"Saturday",dcolor,bgcolor);
					break;
				 case 7:
						GUI_sprintf_hzstr16x(x,y-16,"Sunday",dcolor,bgcolor);
					break;				 
				 
		   }
		
			GUI_sprintf_hzstr16x(x,y-32,"20",dcolor,bgcolor);	//��ʾ�� �� �� ֮��� ��-��
			GUI_sprintf_hzstr16x(x+32,y-32,"-",dcolor,bgcolor);
			GUI_sprintf_hzstr16x(x+56,y-32,"-",dcolor,bgcolor);


			number10(x+16,y-32,RTC_DateStruct.RTC_Year,dcolor,bgcolor);		  //��						  
			number10(x+16+24,y-32,RTC_DateStruct.RTC_Month,dcolor,bgcolor);	//��								  
			number10(x+16+48,y-32,RTC_DateStruct.RTC_Date,dcolor,bgcolor);	//��

									                             
	 }
	}
}




//�������ú���
//���ض�Ӧֵ
u8 Click_T(u16 x,u16 y)	   
{
 u8 result=0;

  if(x>=50&&x<=70)
  {
   if(y>=95&&y<=114) result=1;		//�� �µ�
   if(y>=125&&y<=144) result=3;		//�� �µ�
   if(y>=155&&y<=174) result=5;		//ʱ �µ�
	 if(y>=185&&y<=204) result=7;	  //���� �µ�	
   if(y>=215&&y<=234) result=9;	  //�� �µ� 
   if(y>=245&&y<=264) result=11;	//�� �µ�
   if(y>=275&&y<=294) result=13;	//�� �µ� 
  }

  if(x>=100&&x<=140)
  {
   if(y>=95&&y<=114) result=2;		//�� �ϵ�
   if(y>=125&&y<=144) result=4;		//�� �ϵ�
   if(y>=155&&y<=174) result=6;		//ʱ �ϵ�
	 if(y>=185&&y<=204) result=8;	  //���� �ϵ� 	
   if(y>=215&&y<=234) result=10;	  //�� �ϵ� 
   if(y>=245&&y<=264) result=12;	//�� �ϵ�
   if(y>=275&&y<=294) result=14;	//�� �ϵ� 
  }


  if(x>=180&&x<=220&&y>=245&&y<=275)		//������� 
  result=15;
  if(x>=1&&x<=120&&y>=70&&y<=90)		//ʱ��������
  result=16;
  if(x>=200&&x<=239&&y>=295&&y<=325)	 //����
  result=17;
 return result;
}


//��������
u8  Time_Touch(void)
{
	struct TFT_Pointer sp;
    u8 cp;

	sp=TFT_Cm();	                // ɨ�败����

	if(sp.flag==1)					//�д�������
	 {	 
	   cp=Click_T(sp.x,sp.y);  //�жϴ�������λ�õĶ�Ӧֵ

	    return cp; 		            //���㴥�� ֱ�ӷ��زɼ�����
	 }
    return 0;
}



//ʱ������ʱ�������������ʾ ����ǰ�����������������ɫ״̬��
//dcolor ��ʾ��������ɫ  bgcolor ��ʾ�ı�����ɫ
void T_set_show(u16 dcolor,u16 bgcolor)
{
  u8 i;
  for(i=0;i<7;i++)		   //��ʾ��������
	{
	 GUI_box(50,95+i*30,140,114+i*30,bgcolor);
	 GUI_box(71,97+i*30,119,112+i*30,White);
	 GUI_sprintf_hzstr16x(55,97+i*30,"-        +",dcolor,1);	
	} 
	GUI_sprintf_hzstr16x(7,97,"��:",dcolor,1);
	GUI_sprintf_hzstr16x(7,127,"��:",dcolor,1);
	GUI_sprintf_hzstr16x(7,157,"ʱ:",dcolor,1);
	GUI_sprintf_hzstr16x(7,187,"��:",dcolor,1);
	GUI_sprintf_hzstr16x(7,217,"��:",dcolor,1);	
	GUI_sprintf_hzstr16x(7,247,"��:",dcolor,1);
	GUI_sprintf_hzstr16x(7,277,"��:",dcolor,1);
}


//������������
void touch_set_T(u8 touch)
{
u8 Time_m[6];//���ʱ��������
u16 Time_y;  //��
u8 i;
if(touch==16)				   //�������� ���� ģʽ
{
   GUI_sprintf_hzstr16x(0,71,"SET Time ����˴�����ʱ�䣡",Red,1);
   delay_ms(200);
   GUI_sprintf_hzstr16x(0,71,"SET Time ����˴�����ʱ�䣡",White,1);

   T_set_show(Red3,Blue);

   led_d2=0;					   //��������ָʾ��  

   Time_m[0]=RTC_TimeStruct.RTC_Seconds;
   Time_m[1]=RTC_TimeStruct.RTC_Minutes;
   Time_m[2]=RTC_TimeStruct.RTC_Hours;
	 Time_m[3]=RTC_DateStruct.RTC_WeekDay;
   Time_m[4]=RTC_DateStruct.RTC_Date;
   Time_m[5]=RTC_DateStruct.RTC_Month;
   Time_y = RTC_DateStruct.RTC_Year+2000;

   for(i=0;i<6;i++)									
   number10(87,97+i*30,Time_m[i],Blue,White);
   number(79,97+6*30,Time_y,Blue,White);


   GUI_arcrectangle(180,248,30,20,3,Red3,Red);
   GUI_sprintf_hzstr16x(187,250,"OK",White,1);


   while(1)
   {
	  touch=Time_Touch();	   //�ٴν��� ����ɨ��״̬  ��ȡ����ֵ

      if(touch!=0&&touch!=15)
	   {
		  if(touch==1) {Time_m[0]--;if(Time_m[0]==255)Time_m[0]=59;
		                GUI_sprintf_hzstr16x(55,97,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,97,"-",Red3,1);}	//��
		  if(touch==2) {Time_m[0]++;if(Time_m[0]==60) Time_m[0]=0; 
		                GUI_sprintf_hzstr16x(127,97,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,97,"+",Red3,1);}
		
		  if(touch==3) {Time_m[1]--;if(Time_m[1]==255)Time_m[1]=59;
		                GUI_sprintf_hzstr16x(55,127,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,127,"-",Red3,1);}	//��
		  if(touch==4) {Time_m[1]++;if(Time_m[1]==60) Time_m[1]=0; 
		                GUI_sprintf_hzstr16x(127,127,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,127,"+",Red3,1);}
					
		  if(touch==5) {Time_m[2]--;if(Time_m[2]==255)Time_m[2]=23;
		                GUI_sprintf_hzstr16x(55,157,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,157,"-",Red3,1);}	//ʱ
		  if(touch==6) {Time_m[2]++;if(Time_m[2]==24) Time_m[2]=0; 
		                GUI_sprintf_hzstr16x(127,157,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,157,"+",Red3,1);}
		
		  if(touch==7) {Time_m[3]--;if(Time_m[3]==0)Time_m[3]=7;
		                GUI_sprintf_hzstr16x(55,187,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,187,"-",Red3,1);}	//����
		  if(touch==8) {Time_m[3]++;if(Time_m[3]==8) Time_m[3]=1; 
		                GUI_sprintf_hzstr16x(127,187,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,187,"+",Red3,1);}						
												
		  if(touch==9) {Time_m[4]--;if(Time_m[4]==0)Time_m[4]=31;
		                GUI_sprintf_hzstr16x(55,217,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,217,"-",Red3,1);}	//��
		  if(touch==10) {Time_m[4]++;if(Time_m[4]==32) Time_m[4]=1; 
		                GUI_sprintf_hzstr16x(127,217,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,217,"+",Red3,1);}
			
		  if(touch==11) {Time_m[5]--;if(Time_m[5]==0)Time_m[5]=12; 
		                GUI_sprintf_hzstr16x(55,247,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,247,"-",Red3,1);}	//��
		  if(touch==12){Time_m[5]++;if(Time_m[5]==13)  Time_m[5]=1;  
		                GUI_sprintf_hzstr16x(127,247,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,247,"+",Red3,1);}
	
		  if(touch==13){Time_y--;
		                GUI_sprintf_hzstr16x(55,277,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,277,"-",Red3,1);}	//��
		  if(touch==14){Time_y++;
		                GUI_sprintf_hzstr16x(127,277,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,277,"+",Red3,1);}
  
	   }
	  //��ʾ����ֵ
	  for(i=0;i<6;i++)								
	  number10(87,97+i*30,Time_m[i],Blue,White);    //��ʾ������ �� ʱ �� �� ��    �����Ժ���Ӣ�ı�ʾ
	  number(79,97+6*30,Time_y,Blue,White);

	  		
	  if(touch==15)										//����������� ���г�����DS1302д���������� 
	  {
	   GUI_sprintf_hzstr16x(187,250,"OK",Black,1);
	   delay_ms(120);
	   GUI_sprintf_hzstr16x(187,250,"OK",White,1);

		//�������� ʱ��	
		 RTC_Set_Time(Time_m[2],Time_m[1],Time_m[0],RTC_H12_AM);
		 RTC_Set_Date(Time_y-2000,Time_m[5],Time_m[4],Time_m[3]);				
			

	   T_set_show(Blue,Gray); //�ָ�
	   GUI_box(180,240,210,297,White);

	   led_d2=1;									    //�˳����ú� �ص�ָʾ��
	   break;											//�˳�����  ����ʱ����ʾ
	  }

	}

}

}





//DS1302ģ�� ���庯��  
//�ɽ��е� ��ʱ�� ��ͦ���� 
//�� ���� ʱ�� ���ڵ�
void Time_set(void)
{
  u8 touch;					   //

	//��ʾ����

	GUI_Clear(White);	       //����
    GUI_box(0,0,239,70,Black); //��ɫ����ʾ
	
	GUI_box(0,70,239,87,Green1); 														 
	GUI_sprintf_hzstr16x(0,71,"SET Time ����˴�����ʱ�䣡",White,1);

	T_set_show(Blue,Gray);

    tian(0);                          //�����
	GUI_sprintf_hzstr16x(0,303,"ʱ��RTC",White,1);//��ʾ��Ӧ��ʾ

   //  ��ѭ�� ����ʱ��  ���ȴ����� �ж��Ƿ�����ʱ�� ����
  while(1)
  {
    touch=Time_Touch();		        //ɨ�败������
	if(touch==17){tian(2);break;}		//�������˵�	Return()���� ������̬��ʾ
    touch_set_T(touch);			    //ɨ�败������

    if(touch==17)break;
	Time_scan(50,40,White,Black,1);	//������ʾ
  } 


}



