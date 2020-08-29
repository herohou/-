
#include "mcusys.h"
#include "delay.h"
#include "rtc.h" 	
#include "led.h"
#include "usart.h" 	    
#include "gui.h" 
#include "xpt2046.h"



NVIC_InitTypeDef   NVIC_InitStructure;

//RTC时间设置
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(0~31)
//week:星期(1~7,0,非法!)
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}

//RTC初始化
//返回值:0,初始化成功;
//       1,LSE开启失败;
//       2,进入初始化模式失败;
u8 My_RTC_Init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	u16 retry=0X1FFF; 
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问 
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050)		//是否第一次配置?
	{
		RCC_LSEConfig(RCC_LSE_ON);//LSE 开启    
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
			{
			retry++;
			delay_ms(10);
			}
		if(retry==0)return 1;		//LSE 开启失败. 
			
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟    
		RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟 

    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;//RTC异步分频系数(1~0X7F)
    RTC_InitStructure.RTC_SynchPrediv  = 0xFF;//RTC同步分频系数(0~7FFF)
    RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;//RTC设置为,24小时格式
    RTC_Init(&RTC_InitStructure);
 
		RTC_Set_Time(23,59,56,RTC_H12_AM);	//设置时间
		RTC_Set_Date(20,3,2,1);		//设置日期
	 
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//标记已经初始化过了
	} 
 
	return 0;
}

//设置闹钟时间(按星期闹铃,24小时制)
//week:星期几(1~7) @ref  RTC_Alarm_Definitions
//hour,min,sec:小时,分钟,秒钟
void RTC_Set_AlarmA(u8 week,u8 hour,u8 min,u8 sec)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	RTC_AlarmTypeDef RTC_AlarmTypeInitStructure;
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);//关闭闹钟A 
	
  RTC_TimeTypeInitStructure.RTC_Hours=hour;//小时
	RTC_TimeTypeInitStructure.RTC_Minutes=min;//分钟
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;//秒
	RTC_TimeTypeInitStructure.RTC_H12=RTC_H12_AM;
  
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDay=week;//星期
	RTC_AlarmTypeInitStructure.RTC_AlarmDateWeekDaySel=RTC_AlarmDateWeekDaySel_WeekDay;//按星期闹
	RTC_AlarmTypeInitStructure.RTC_AlarmMask=RTC_AlarmMask_None;//精确匹配星期，时分秒
	RTC_AlarmTypeInitStructure.RTC_AlarmTime=RTC_TimeTypeInitStructure;
  RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_A,&RTC_AlarmTypeInitStructure);
 
	
	RTC_ClearITPendingBit(RTC_IT_ALRA);//清除RTC闹钟A的标志
  EXTI_ClearITPendingBit(EXTI_Line17);//清除LINE17上的中断标志位 
	
	RTC_ITConfig(RTC_IT_ALRA,ENABLE);//开启闹钟A中断
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);//开启闹钟A 
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;//LINE17
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE17
  EXTI_Init(&EXTI_InitStructure);//配置

	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
}

//周期性唤醒定时器设置  
/*wksel:  @ref RTC_Wakeup_Timer_Definitions
#define RTC_WakeUpClock_RTCCLK_Div16        ((uint32_t)0x00000000)
#define RTC_WakeUpClock_RTCCLK_Div8         ((uint32_t)0x00000001)
#define RTC_WakeUpClock_RTCCLK_Div4         ((uint32_t)0x00000002)
#define RTC_WakeUpClock_RTCCLK_Div2         ((uint32_t)0x00000003)
#define RTC_WakeUpClock_CK_SPRE_16bits      ((uint32_t)0x00000004)
#define RTC_WakeUpClock_CK_SPRE_17bits      ((uint32_t)0x00000006)
*/
//cnt:自动重装载值.减到0,产生中断.
void RTC_Set_WakeUp(u32 wksel,u16 cnt)
{ 
	EXTI_InitTypeDef   EXTI_InitStructure;
	
	RTC_WakeUpCmd(DISABLE);//关闭WAKE UP
	
	RTC_WakeUpClockConfig(wksel);//唤醒时钟选择
	
	RTC_SetWakeUpCounter(cnt);//设置WAKE UP自动重装载寄存器
	
	
	RTC_ClearITPendingBit(RTC_IT_WUT); //清除RTC WAKE UP的标志
  EXTI_ClearITPendingBit(EXTI_Line22);//清除LINE22上的中断标志位 
	 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);//开启WAKE UP 定时器中断
	RTC_WakeUpCmd( ENABLE);//开启WAKE UP 定时器　
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;//LINE22
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断事件
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //上升沿触发 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能LINE22
  EXTI_Init(&EXTI_InitStructure);//配置
 
 
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级1
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;//子优先级2
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure);//配置
}

//RTC闹钟中断服务函数
void RTC_Alarm_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_ALRAF)==SET)//ALARM A中断?
	{
		RTC_ClearFlag(RTC_FLAG_ALRAF);//清除中断标志
		printf("ALARM A!\r\n");
	}   
	EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断线17的中断标志 											 
}

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

//RTC WAKE UP中断服务函数
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF)==SET)//WK_UP中断?
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);	//清除中断标志
		RTC_GetTime(RTC_Format_BIN,&RTC_TimeStruct);  //获取时分秒
		//led_d2=!led_d2; 
	}   
	EXTI_ClearITPendingBit(EXTI_Line22);//清除中断线22的中断标志 								
}





u8 time;				 //秒时间更新标志

// 时间 年月日 星期 整体显示函数
//x y  显示的时间的初始坐标  星期在其上面显示  年月日 在星期上面显示
//如
//2020-12-12
//Friday
//20:25:32
//dcolor 字体颜色	  bgcolor  背景颜色
//mode  为1时 支持 年月日 星期显示  其它 只显示时间
void Time_scan(u8 x,u16 y,u16 dcolor,u16 bgcolor,u8 mode)
{

  if(time!=RTC_TimeStruct.RTC_Seconds)
   {
   	 time=RTC_TimeStruct.RTC_Seconds;
		 RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct); //获取年月日

	   GUI_sprintf_hzstr16x(x+16,y,":",dcolor,bgcolor);  //显示秒 分 时 之间的 “：”	
	   GUI_sprintf_hzstr16x(x+40,y,":",dcolor,bgcolor);
				
		 number10(x,y,RTC_TimeStruct.RTC_Hours,dcolor,bgcolor);		      //小时							  
		 number10(x+24,y,RTC_TimeStruct.RTC_Minutes,dcolor,bgcolor);		//分钟							  
	   number10(x+48,y,RTC_TimeStruct.RTC_Seconds,dcolor,bgcolor);		//秒
						 
	   if(mode==1)		 //判断 是否开启 日期 年月显示   mode为1表示开启
	   {			
					 
		   switch(RTC_DateStruct.RTC_WeekDay)					  //星期
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
		
			GUI_sprintf_hzstr16x(x,y-32,"20",dcolor,bgcolor);	//显示日 月 年 之间的 “-”
			GUI_sprintf_hzstr16x(x+32,y-32,"-",dcolor,bgcolor);
			GUI_sprintf_hzstr16x(x+56,y-32,"-",dcolor,bgcolor);


			number10(x+16,y-32,RTC_DateStruct.RTC_Year,dcolor,bgcolor);		  //年						  
			number10(x+16+24,y-32,RTC_DateStruct.RTC_Month,dcolor,bgcolor);	//月								  
			number10(x+16+48,y-32,RTC_DateStruct.RTC_Date,dcolor,bgcolor);	//日

									                             
	 }
	}
}




//触摸调用函数
//返回对应值
u8 Click_T(u16 x,u16 y)	   
{
 u8 result=0;

  if(x>=50&&x<=70)
  {
   if(y>=95&&y<=114) result=1;		//秒 下调
   if(y>=125&&y<=144) result=3;		//分 下调
   if(y>=155&&y<=174) result=5;		//时 下调
	 if(y>=185&&y<=204) result=7;	  //星期 下调	
   if(y>=215&&y<=234) result=9;	  //日 下调 
   if(y>=245&&y<=264) result=11;	//月 下调
   if(y>=275&&y<=294) result=13;	//年 下调 
  }

  if(x>=100&&x<=140)
  {
   if(y>=95&&y<=114) result=2;		//秒 上调
   if(y>=125&&y<=144) result=4;		//分 上调
   if(y>=155&&y<=174) result=6;		//时 上调
	 if(y>=185&&y<=204) result=8;	  //星期 上调 	
   if(y>=215&&y<=234) result=10;	  //日 上调 
   if(y>=245&&y<=264) result=12;	//月 上调
   if(y>=275&&y<=294) result=14;	//年 上调 
  }


  if(x>=180&&x<=220&&y>=245&&y<=275)		//设置完成 
  result=15;
  if(x>=1&&x<=120&&y>=70&&y<=90)		//时间重设置
  result=16;
  if(x>=200&&x<=239&&y>=295&&y<=325)	 //返回
  result=17;
 return result;
}


//触摸函数
u8  Time_Touch(void)
{
	struct TFT_Pointer sp;
    u8 cp;

	sp=TFT_Cm();	                // 扫描触摸屏

	if(sp.flag==1)					//有触摸现象
	 {	 
	   cp=Click_T(sp.x,sp.y);  //判断触摸发生位置的对应值

	    return cp; 		            //单点触摸 直接返回采集数据
	 }
    return 0;
}



//时间设置时，操作界面的显示 设置前后和设置中是两种颜色状态。
//dcolor 显示的字体颜色  bgcolor 显示的背景颜色
void T_set_show(u16 dcolor,u16 bgcolor)
{
  u8 i;
  for(i=0;i<7;i++)		   //显示设置区域
	{
	 GUI_box(50,95+i*30,140,114+i*30,bgcolor);
	 GUI_box(71,97+i*30,119,112+i*30,White);
	 GUI_sprintf_hzstr16x(55,97+i*30,"-        +",dcolor,1);	
	} 
	GUI_sprintf_hzstr16x(7,97,"秒:",dcolor,1);
	GUI_sprintf_hzstr16x(7,127,"分:",dcolor,1);
	GUI_sprintf_hzstr16x(7,157,"时:",dcolor,1);
	GUI_sprintf_hzstr16x(7,187,"周:",dcolor,1);
	GUI_sprintf_hzstr16x(7,217,"日:",dcolor,1);	
	GUI_sprintf_hzstr16x(7,247,"月:",dcolor,1);
	GUI_sprintf_hzstr16x(7,277,"年:",dcolor,1);
}


//触摸设置数据
void touch_set_T(u8 touch)
{
u8 Time_m[6];//秒分时星期日月
u16 Time_y;  //年
u8 i;
if(touch==16)				   //进入设置 日历 模式
{
   GUI_sprintf_hzstr16x(0,71,"SET Time 点击此处设置时间！",Red,1);
   delay_ms(200);
   GUI_sprintf_hzstr16x(0,71,"SET Time 点击此处设置时间！",White,1);

   T_set_show(Red3,Blue);

   led_d2=0;					   //进入后点亮指示灯  

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
	  touch=Time_Touch();	   //再次进入 触摸扫描状态  提取设置值

      if(touch!=0&&touch!=15)
	   {
		  if(touch==1) {Time_m[0]--;if(Time_m[0]==255)Time_m[0]=59;
		                GUI_sprintf_hzstr16x(55,97,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,97,"-",Red3,1);}	//秒
		  if(touch==2) {Time_m[0]++;if(Time_m[0]==60) Time_m[0]=0; 
		                GUI_sprintf_hzstr16x(127,97,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,97,"+",Red3,1);}
		
		  if(touch==3) {Time_m[1]--;if(Time_m[1]==255)Time_m[1]=59;
		                GUI_sprintf_hzstr16x(55,127,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,127,"-",Red3,1);}	//分
		  if(touch==4) {Time_m[1]++;if(Time_m[1]==60) Time_m[1]=0; 
		                GUI_sprintf_hzstr16x(127,127,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,127,"+",Red3,1);}
					
		  if(touch==5) {Time_m[2]--;if(Time_m[2]==255)Time_m[2]=23;
		                GUI_sprintf_hzstr16x(55,157,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,157,"-",Red3,1);}	//时
		  if(touch==6) {Time_m[2]++;if(Time_m[2]==24) Time_m[2]=0; 
		                GUI_sprintf_hzstr16x(127,157,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,157,"+",Red3,1);}
		
		  if(touch==7) {Time_m[3]--;if(Time_m[3]==0)Time_m[3]=7;
		                GUI_sprintf_hzstr16x(55,187,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,187,"-",Red3,1);}	//星期
		  if(touch==8) {Time_m[3]++;if(Time_m[3]==8) Time_m[3]=1; 
		                GUI_sprintf_hzstr16x(127,187,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,187,"+",Red3,1);}						
												
		  if(touch==9) {Time_m[4]--;if(Time_m[4]==0)Time_m[4]=31;
		                GUI_sprintf_hzstr16x(55,217,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,217,"-",Red3,1);}	//日
		  if(touch==10) {Time_m[4]++;if(Time_m[4]==32) Time_m[4]=1; 
		                GUI_sprintf_hzstr16x(127,217,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,217,"+",Red3,1);}
			
		  if(touch==11) {Time_m[5]--;if(Time_m[5]==0)Time_m[5]=12; 
		                GUI_sprintf_hzstr16x(55,247,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,247,"-",Red3,1);}	//月
		  if(touch==12){Time_m[5]++;if(Time_m[5]==13)  Time_m[5]=1;  
		                GUI_sprintf_hzstr16x(127,247,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,247,"+",Red3,1);}
	
		  if(touch==13){Time_y--;
		                GUI_sprintf_hzstr16x(55,277,"-",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(55,277,"-",Red3,1);}	//年
		  if(touch==14){Time_y++;
		                GUI_sprintf_hzstr16x(127,277,"+",White,1);delay_ms(120);
						GUI_sprintf_hzstr16x(127,277,"+",Red3,1);}
  
	   }
	  //显示设置值
	  for(i=0;i<6;i++)								
	  number10(87,97+i*30,Time_m[i],Blue,White);    //显示包括秒 分 时 日 月 年    设置以后用英文表示
	  number(79,97+6*30,Time_y,Blue,White);

	  		
	  if(touch==15)										//触摸设置完成 进行程序向DS1302写入设置数据 
	  {
	   GUI_sprintf_hzstr16x(187,250,"OK",Black,1);
	   delay_ms(120);
	   GUI_sprintf_hzstr16x(187,250,"OK",White,1);

		//设置日期 时间	
		 RTC_Set_Time(Time_m[2],Time_m[1],Time_m[0],RTC_H12_AM);
		 RTC_Set_Date(Time_y-2000,Time_m[5],Time_m[4],Time_m[3]);				
			

	   T_set_show(Blue,Gray); //恢复
	   GUI_box(180,240,210,297,White);

	   led_d2=1;									    //退出设置后 关掉指示灯
	   break;											//退出设置  进行时间显示
	  }

	}

}

}





//DS1302模块 主体函数  
//可进行到 秒时间 不挺更新 
//可 设置 时间 日期等
void Time_set(void)
{
  u8 touch;					   //

	//显示区域

	GUI_Clear(White);	       //清屏
    GUI_box(0,0,239,70,Black); //黑色清显示
	
	GUI_box(0,70,239,87,Green1); 														 
	GUI_sprintf_hzstr16x(0,71,"SET Time 点击此处设置时间！",White,1);

	T_set_show(Blue,Gray);

    tian(0);                          //下填充
	GUI_sprintf_hzstr16x(0,303,"时钟RTC",White,1);//显示对应提示

   //  死循环 更新时间  并等待触摸 判断是否设置时间 日期
  while(1)
  {
    touch=Time_Touch();		        //扫描触摸数据
	if(touch==17){tian(2);break;}		//返回主菜单	Return()返回 按键动态显示
    touch_set_T(touch);			    //扫描触摸功能

    if(touch==17)break;
	Time_scan(50,40,White,Black,1);	//更新显示
  } 


}



