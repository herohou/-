
/*

蓝牙为串口模块  蓝牙4.0模块
蓝牙模式下  串口2 PA2 PA3与其通讯  
串口2通讯波特率 为9600

通过 蓝牙串口助手 或者 我们提供的 安卓APP软件都可以对板子上LED 蜂鸣器 进行控制  
支持 DHT11温湿度传感器数据回读  板子时间校准




ESP8266模块 也接在串口2上 所以这两种模块只能选择一种通讯方式
 
ESP8266设置为AP+Station模式 server服务器模式 并产生一个wifi热点

串口2与 wifi模块通讯 其波特率为115200

1. 该程序将ESP8266设置为AP+STATION工作模式，并建立TCP 服务器，默认IP为192.168.4.1， 开放端口5000
2. 测试手机打开WIFI后，搜索到ESP_***热点后，进行连接。
3. 打开安装在手机里的网络TCP/IP调试助手，或者我们提供的APP软件 与TCP服务器192.168.4.1，端口5000，进行连接。
4. 手机发送 与接收功能和蓝牙是一样的

*/

#include "mcusys.h"
#include "xpt2046.h"
#include "gui.h"
#include "BleEsp.h"
#include "flash.h"
#include "delay.h"
#include "string.h"
#include "stdio.h"
#include "dht11.h"
#include "rtc.h"
#include "usart.h"
#include "led.h"
#include "buzzer.h"
 

///////////////////////////
///////////////////////////

 u8 ble[]="AT"; 


//接收指令
 u8 led1_open_m[]="led1 open";         // 
 u8 led1_close_m[]="led1 close";       //
 u8 led2_open_m[]="led2 open";         // 
 u8 led2_close_m[]="led2 close";       //
 u8 fm_open_m[]="fm open";             // 
 u8 fm_close_m[]="fm close";           //


///////////////////////////
//回传响应
 u8 led1_open[]="QGled1 is open/";         // 
 u8 led1_close[]="QGled1 is close/";       //
 u8 led2_open[]="QGled2 is open/";         // 
 u8 led2_close[]="QGled2 is close/";       //
 u8 fm_open[]="QGfm is open/";             // 
 u8 fm_close[]="QGfm is close/";           //
 u8 time_ok[]="QGtime updata ok/";			//时间校对完成

 u8 inst_error[]="QGinstruction error/";      //错误指令
///////////////////////////
///////////////////////////


///////////////////////////
//esp通讯设置指令
 u8 esp_at[]="AT\r\n";                  // 握手连接指令，返回"OK"
 u8 esp_cwmode[]="AT+CWMODE=3\r\n";     // 设置ESP8266的工作模式3 AP+Station，返回"OK"或者"no change"
 u8 esp_cifsr[]="AT+CIFSR\r\n";         // 本机IP地址查询指令
 u8 esp_cipsend[]="AT+CIPSEND=\r\n";    // 设置发送数据长度
// u8 esp_test[]="sunny\r\n";   			//  数据内容
 u8 esp_cipserver[]="AT+CIPSERVER=1,5000\r\n";  //建立TCP服务器，开放端口5000							
 u8 esp_cipmux[]="AT+CIPMUX=1\r\n";   			//打开多连接	
 u8 esp_rst[]="AT+RST\r\n"; 					// 软件复位




u8 esp_init;	  //esp是否初始化标志

u8 suf; //数据接收
u8 z_bz=0;
u32 dht11_num=0;	//温湿度采集 计数变量
u32 bel_num=0;		//蓝牙状态检查 计数变量
u8  ble_sta=0;		//蓝牙是否在线状态
u8 dht_array[20];	//dht11采集数据发送数组
u8 T_tmp[7]={0,0,0,0,0,0,0};//秒分时日月周年
//bit ledx1=0,ledx2=0,fm=0;			  //状态标志





//触摸返回判断
//x y  为输入坐标值
//返回 所触摸方格的数字值 0代表不在格内
u8 Click_ble(u8 x,u16 y)
{
 u8 result=0;							  
 if(x>=200&&x<=239&&y>=300&&y<=320)	 //返回
  result=17;

 return result;
}



//for 屏幕显示方块形闪烁灯  显示蓝牙状态
//x y 显示位置
//lth 方块边长
//mode 0 表示处于暗  1 表示处于亮	2 提前做外框显示防止干扰
void ble_led_block(u16 x,u16 y,u8 lth,u8 mode)
{
  u8 k=3;
  if(mode==1)
  {
   GUI_box(x+k,y+k,x+lth,y+lth,Red3); //显示灯色
  }
   if(mode==0)
  {
   GUI_box(x+k,y+k,x+lth,y+lth,White); //显示灯色
  }
//  if(mode==2)
//  {
//   GUI_box(x,y,x+lth,y+lth,Black); //显示外框
//   lth=lth-k;
//   GUI_box(x+k,y+k,x+lth,y+lth,White); //显示灯色
//  }
}



#define led1_x 20
#define led1_y 143
#define led_r 12

#define led2_x 58
#define led2_y 143

#define led3_x 96
#define led3_y 143


//蓝牙模块 ESPwifi模块 测试主函数
//mode0 0 蓝牙模式   1 ESP8266模式
void ble_esp_test(u8 mode0)		
{
u8 cp;
u8 time_i=0,i_esp,esp_ok=0;	
u8 ble_block;		 //蓝牙状态方块 闪烁标志
u8 dht_block=0;		 //dht状态方块  闪烁标志


//u16 zf_Tem,zf_Hp;

struct TFT_Pointer sp;		 //定义触摸结构体变量sp

GUI_Clear(White);				  //整体清屏
GUI_box(0,300,239,301,Black);	  //下填充色
GUI_box(0,302,239,319,Red);


if(mode0==0)	//蓝牙模式
{
	flash_binplay(0,0,60,60,224);	  //显示蓝牙图标
	GUI_box(61,0,239,59,Blue1);

	GUI_sprintf_hzstr16x(0,303,"BLE蓝牙控制",White,1);	
	GUI_sprintf_hzstr16x(70,22,"蓝牙状态监测",Black,1);

	uart_init(9600);	 //串口1 2初始化为9600  
	esp_init=1;       //蓝牙模式关闭ESP初始化

}
else if(mode0==1)  //esp 模式
{
	flash_binplay(0,0,60,60,222);	  //显示wifi图标
	GUI_box(61,0,239,59,Yellow);
	
	GUI_sprintf_hzstr16x(0,303,"ESP wifi控制",White,1);

	GUI_sprintf_hzstr16x(70,2,"esp8622 wifi模块",Black,1);

	GUI_sprintf_hzstr16x(70,22,"服务器（sever）",Black,1);
	uart_init(115200);	 //串口1 2初始化为115200  

	esp_init=0;      //esp未初始化标志
}



GUI_box(0,100,239,100,Black);			  //画区域	  横竖横
GUI_box(120,101,120,299,Black);
GUI_box(0,198,239,198,Black);

////////////////////////////
////////////////////////////
GUI_box(1,102,118,196,Green1);			  //画LED区域块
GUI_sprintf_hzstr16x(1,102,"LED状态",White,Green1);

GUI_arc(led1_x,led1_y,led_r,Red3,0);						 //画选择圆圈区
GUI_arc(led2_x,led2_y,led_r,Red3,0);						 //画选择圆圈区
GUI_arc(led3_x,led3_y,led_r,Red3,0);						 //画选择圆圈区
GUI_arc(led1_x,led1_y,led_r-2,White,1);
GUI_arc(led2_x,led2_y,led_r-2,White,1);
GUI_arc(led3_x,led3_y,led_r-2,White,1);

GUI_sprintf_hzstr16x(6,162,"LED1",Black,1);
GUI_sprintf_hzstr16x(44,162,"LED2",Black,1);
GUI_sprintf_hzstr16x(82,162,"LED3",Black,1);

////////////////////////////
////////////////////////////
GUI_box(122,102,238,196,Black);			  //画蜂鸣器区域块
GUI_sprintf_hzstr16x(122,102,"蜂鸣器状态",Yellow,1);
GUI_sprintf_hzstr16x(122,120,"驱动IO Pc13",Blue3,1);
GUI_sprintf_hzstr16x(122,142,"当前状态：",White,1);
GUI_sprintf_hzstr16x(203,142,"停止",White,1);

////////////////////////////
////////////////////////////
GUI_box(1,200,118,298,Gray);			  //画温湿度区域块
GUI_sprintf_hzstr16x(1,200,"温湿度采集",Black,1);
GUI_sprintf_hzstr16x(1,230,"温度:    C",Red2,1);
GUI_sprintf_hzstr16x(1,260,"湿度:   %RH",Blue2,1);

////////////////////////////
////////////////////////////
GUI_box(122,200,238,298,Red);			  //画蜂鸣器区域块
GUI_sprintf_hzstr16x(122,200,"当前时间",Blue,1);


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
if(mode0==1)  //esp 模式   开始初始化设置
{
    //发送AT 进行握手				  各命令循环10次等待 
    for(i_esp=0;i_esp<6;i_esp++)
	{

	 Uart2SendStr(esp_at);	   //串口2对wifi模块发送握手指令 即AT

	 if(Data_compare2("OK"))
	 {
		 GUI_sprintf_hzstr16x(0,70,"OK,success with ESP!         ",Blue,White);
		 memset(esp_recive, 0, esp_tb);//清缓存数据
		 re_stbz=0;				   //接收计数变量清0
		 //delay_ms(600);	 
		 esp_ok++;         //ok标志递加1
		 break;		 
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR1,some problems with ESP",Blue,White);
	 delay_ms(600);
	}	 

	//配置wifi工作模式为ap+sta模式
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cwmode);	   //串口2对wifi模块工作模式进行设置  
	 if(Data_compare2("OK")||Data_compare("no change"))
	 {
		 GUI_sprintf_hzstr16x(0,70,"OK,set AP+Station with ESP   ",Blue,White);	 
		 memset(esp_recive, 0, esp_tb);//清缓存数据
		 re_stbz=0;				   //接收计数变量清0
		 //delay_ms(600);
		 esp_ok++;        //ok标志递加2
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR2,some problems with ESP",Blue,White);
	 delay_ms(600);
	}
			  
	//配置wifi为多路可连接模式
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cipmux);	   //串口2对wifi模块 设置多连接 （多路连接模式）
	 if(Data_compare2("OK"))
	 {
		 memset(esp_recive, 0, esp_tb);//清缓存数据
		 re_stbz=0;				   //接收计数变量清0
		 //delay_ms(600);		 
		 esp_ok++;        //ok标志递加3	 
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR3,some problems with ESP",Blue,White);
	 delay_ms(600);
	}

	//配置wifi为服务器模式 配置端口号为5000
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cipserver);	   //串口2设置wifi模块 为TCP服务器模式，并配置端口为5000 
	 if(Data_compare2("OK"))
	 {
		 memset(esp_recive, 0, esp_tb);//清缓存数据
		 re_stbz=0;				   //接收计数变量清0
		 //delay_ms(600);	
		 esp_ok++;        //ok标志递加4
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR4,some problems with ESP",Blue,White);
	 delay_ms(600);
	}

	//查询模块ip
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cifsr);	   //串口2查询wifi模块 当前ip地址 
	 if(Data_compare2("OK"))
	 {
		 memset(esp_recive, 0, esp_tb);//清缓存数据
		 re_stbz=0;				   //接收计数变量清0
		 //delay_ms(600);
		 esp_ok++;        //ok标志递加5		 
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR5,some problems with ESP",Blue,White);
	 delay_ms(600);
	}

	if(esp_ok>=4)GUI_sprintf_hzstr16x(0,70,"ESP8266 wifi模块 初始化 OK！  ",Red,White);
	else         GUI_sprintf_hzstr16x(0,70,"ESP8266 wifi模块 初始化 ERROR!",Black,White);
	esp_init=1;   //esp初始化ok 标志位置位

}


//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//主循环体
while(1)
{
  sp=TFT_Cm();	   //扫描触摸屏


if(mode0==0)	   //如果是蓝牙模式
{
    bel_num++;
	if(bel_num>=400000)
	{
		  bel_num=0;
		  Uart2SendStr(ble);	//向蓝牙发送at指令  判断是否返回OK  如果返回ok说明外设没有连接蓝牙 否则相反
		  if(Data_compare3("OK"))
		  {
		   ble_block=~ble_block;
		   if(ble_block==0)
		   ble_led_block(170,20,16,1); //蓝牙状态指示方块灯
		   else
		   ble_led_block(170,20,16,0); //蓝牙状态指示方块灯

		   GUI_sprintf_hzstr16x(0,70,"蓝牙处于待连接状态 请搜索...",Blue,White);
		   ble_sta=0;//蓝牙未连接
		  }
		  else
		  {
		   ble_led_block(170,20,16,1); //蓝牙状态指示方块灯
		   GUI_sprintf_hzstr16x(0,70,"蓝牙已连接成功 请发命令     ",Red,White); 
		   ble_sta=1;//蓝牙已连接
		  }
		  memset(RX2_buffer, 0, tbuf0);//清缓存数据
		  RX2_num=0;				   //接收计数变量清0
	
	}
}


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//串口指令接收及响应区域

if(receive_end)//接收结束标志位	 说明有指令进来 开始判断
{
  receive_end=0; //清标志位
////////////////////////////
////////////////////////////
//LED 灯控制
  if(Data_compare(led1_open_m))
  {
	  memset(u2receive_str, 0, tbuf1);//清缓存数组
	  led_d2=0;GUI_arc(led1_x,led1_y,led_r-2,Red,1);

	  if(mode0==0) //仅蓝牙状态下返回执行状态
	  Uart2SendStr(led1_open);		 //串口2发送对应响应数据
  }
  else if(Data_compare(led1_close_m))
  {
      memset(u2receive_str, 0, tbuf1);//清缓存数组
	  led_d2=1; GUI_arc(led1_x,led1_y,led_r-2,White,1);

	  if(mode0==0) //仅蓝牙状态下返回执行状态
	  Uart2SendStr(led1_close);	 //串口2发送对应响应数据
  }
  else if(Data_compare(led2_open_m))
  {
	  memset(u2receive_str, 0, tbuf1);//清缓存数组
	  led_d3=0;GUI_arc(led2_x,led2_y,led_r-2,Green,1);

	  if(mode0==0) //仅蓝牙状态下返回执行状态
	  Uart2SendStr(led2_open);		 //串口2发送对应响应数据
  }
  else if(Data_compare(led2_close_m))
  {
      memset(u2receive_str, 0, tbuf1);//清缓存数组
	  led_d3=1; GUI_arc(led2_x,led2_y,led_r-2,White,1);

	  if(mode0==0) //仅蓝牙状态下返回执行状态
	  Uart2SendStr(led2_close);	 //串口2发送对应响应数据
  }
	///////////////////////////////////////////////////////


////////////////////////////
////////////////////////////
//蜂鸣器 控制
  else if(Data_compare(fm_open_m))
  {
	  memset(u2receive_str, 0, tbuf1);//清缓存数组
	  fm_io=0;GUI_sprintf_hzstr16x(203,142,"启动",White,Red);

	  if(mode0==0) //仅蓝牙状态下返回执行状态
	  Uart2SendStr(fm_open);		 //串口2发送对应响应数据
  }
  else if(Data_compare(fm_close_m))
  {
      memset(u2receive_str, 0, tbuf1);//清缓存数组
	  fm_io=1;GUI_sprintf_hzstr16x(203,142,"停止",White,Black);

	  if(mode0==0) //仅蓝牙状态下返回执行状态
	  Uart2SendStr(fm_close);	     //串口2发送对应响应数据
  }


////////////////////////////
////////////////////////////
//当收到 数据  “Time”+13字节时间数据 更新时间芯片  分别表示秒分时日月周年
//例如 QGTime0010110108119/ 表示 19年08月01日 11点10分00秒 	1表示星期日	 2星期一
  else if(u2receive_str[0]=='T'&&u2receive_str[1]=='i'&&u2receive_str[2]=='m'&&u2receive_str[3]=='e')
  {
	  //ascii 0-9转16进制
	  for(time_i=0;time_i<13;time_i++) //ascii变对应16进制数值
	  u2receive_str[time_i+4]=u2receive_str[time_i+4]-48;   //48即0x30 (0 0x30  1 0x31  2 0x32......9 0x39)
	  //13个数据值转存到7个字节内
	  T_tmp[0]=u2receive_str[4]*10+u2receive_str[5];	//秒 000000000000000
	  T_tmp[1]=u2receive_str[6]*10+u2receive_str[7];	//分
	  T_tmp[2]=u2receive_str[8]*10+u2receive_str[9];	//时
	  T_tmp[3]=u2receive_str[10]*10+u2receive_str[11];	//日
	  T_tmp[4]=u2receive_str[12]*10+u2receive_str[13];	//月
	  T_tmp[5]=u2receive_str[14];						//周
	  T_tmp[6]=u2receive_str[15]*10+u2receive_str[16];	//年
		
		switch(T_tmp[5])					  //星期转换 这里和协议有关系 协议1是周日   函数1是周一
		 { 
			 case 1:
					T_tmp[5]=7;
				break;
			 case 2:
					T_tmp[5]=1;
				break;
			 case 3:
					T_tmp[5]=2;
				break;
			 case 4:
					T_tmp[5]=3;
				break;
			 case 5:
					T_tmp[5]=4;
				break;
			 case 6:
					T_tmp[5]=5;
				break;
			 case 7:
					T_tmp[5]=6;
				break;
		 }		
				
		RTC_Set_Time(T_tmp[2],T_tmp[1],T_tmp[0],RTC_H12_AM);
		RTC_Set_Date(T_tmp[6],T_tmp[4],T_tmp[3],T_tmp[5]);		//设置日期
//		RTC_Set_AlarmA(1,T_tmp[2],T_tmp[1],T_tmp[0]);
      //RTC_Set(2000+T_tmp[6],T_tmp[4],T_tmp[3],T_tmp[2],T_tmp[1],T_tmp[0]);      //设置时间
	  //RTC_Set(2020,1,5,8,30,55);      //设置时间


	  if(mode0==0) //仅蓝牙状态下返回执行状态
      Uart2SendStr(time_ok);	     //串口2发送对应响应数据
      memset(u2receive_str, 0, tbuf1);//清缓存数组
  }

  //else if()   //此处增加新的指令


  else 		 //到这里说明以上命令都不是
  {
	  if(mode0==0) //仅蓝牙状态下返回执行状态
      Uart2SendStr(inst_error);	  //串口2发送对应响应数据
      memset(u2receive_str, 0, tbuf1);//清缓存数组  
  }

}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////



//////////////////////////////
//////////////////////////////
//温湿度采集 及上传
dht11_num++;    //温湿度采集变量自加
if(dht11_num>=800000)
{
 dht11_num=0;
 DHT11_Read();
 memset(dht_array, 0, 20);//清缓存数组

 //sprintf((char *)dht_array,"QGDdata%4x%4x/",DHT_Tempvalue,DHT_Hpvalue);
	sprintf((char *)dht_array,"%4x",DHT_Tempvalue);

 //判断温度是否为负数
 if(DHT_Tempvalue<0){ DHT_Tempvalue=-DHT_Tempvalue;	GUI_sprintf_char(41,230,'-',Red2,Gray,0);}
 

 DHT_Tempvalue=(DHT_Tempvalue*10)/256;	//10进制转16进制
 number(49,230,DHT_Tempvalue,Red2,Gray);//只显示整数值

 DHT_Hpvalue=(DHT_Hpvalue*10)/256;	//10进制转16进制
 number(49,260,DHT_Hpvalue,Blue2,Gray);//只显示整数值

 if(mode0==1) //esp模式下 需要先配置在发送数据
 { 
   Uart2SendStr("AT+CIPSEND=0,16\r\n"); //如果esp模式 配置ESP发送数据 数据长度
   delay_ms(600);
   Uart2SendStr(dht_array); //串口2发送更新数据
   delay_ms(600);
  }
 else if(ble_sta==1)		//蓝牙是在现在状态才发送
 Uart2SendStr(dht_array); //串口2发送更新数据

 dht_block=~dht_block;
 if(dht_block==0)
 GUI_box(86,202,96,212,Green);	  //方块形状态灯闪烁
 else
 GUI_box(86,202,96,212,White);

}



//////////////////////////
//////////////////////////
//时间显示 时分秒
Time_scan(140,260,White,Red,1);	//更新显示



  if(sp.flag==1)   //如果有按键按下事件
  {
   cp=Click_ble(sp.x,sp.y);		//采集触摸值
   if(cp==17) {tian(2); break;	}	    //扫描触摸屏 判断是否返回主菜单
  }

}


}





















