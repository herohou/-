
/*

����Ϊ����ģ��  ����4.0ģ��
����ģʽ��  ����2 PA2 PA3����ͨѶ  
����2ͨѶ������ Ϊ9600

ͨ�� ������������ ���� �����ṩ�� ��׿APP��������Զ԰�����LED ������ ���п���  
֧�� DHT11��ʪ�ȴ��������ݻض�  ����ʱ��У׼




ESP8266ģ�� Ҳ���ڴ���2�� ����������ģ��ֻ��ѡ��һ��ͨѶ��ʽ
 
ESP8266����ΪAP+Stationģʽ server������ģʽ ������һ��wifi�ȵ�

����2�� wifiģ��ͨѶ �䲨����Ϊ115200

1. �ó���ESP8266����ΪAP+STATION����ģʽ��������TCP ��������Ĭ��IPΪ192.168.4.1�� ���Ŷ˿�5000
2. �����ֻ���WIFI��������ESP_***�ȵ�󣬽������ӡ�
3. �򿪰�װ���ֻ��������TCP/IP�������֣����������ṩ��APP��� ��TCP������192.168.4.1���˿�5000���������ӡ�
4. �ֻ����� ����չ��ܺ�������һ����

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


//����ָ��
 u8 led1_open_m[]="led1 open";         // 
 u8 led1_close_m[]="led1 close";       //
 u8 led2_open_m[]="led2 open";         // 
 u8 led2_close_m[]="led2 close";       //
 u8 fm_open_m[]="fm open";             // 
 u8 fm_close_m[]="fm close";           //


///////////////////////////
//�ش���Ӧ
 u8 led1_open[]="QGled1 is open/";         // 
 u8 led1_close[]="QGled1 is close/";       //
 u8 led2_open[]="QGled2 is open/";         // 
 u8 led2_close[]="QGled2 is close/";       //
 u8 fm_open[]="QGfm is open/";             // 
 u8 fm_close[]="QGfm is close/";           //
 u8 time_ok[]="QGtime updata ok/";			//ʱ��У�����

 u8 inst_error[]="QGinstruction error/";      //����ָ��
///////////////////////////
///////////////////////////


///////////////////////////
//espͨѶ����ָ��
 u8 esp_at[]="AT\r\n";                  // ��������ָ�����"OK"
 u8 esp_cwmode[]="AT+CWMODE=3\r\n";     // ����ESP8266�Ĺ���ģʽ3 AP+Station������"OK"����"no change"
 u8 esp_cifsr[]="AT+CIFSR\r\n";         // ����IP��ַ��ѯָ��
 u8 esp_cipsend[]="AT+CIPSEND=\r\n";    // ���÷������ݳ���
// u8 esp_test[]="sunny\r\n";   			//  ��������
 u8 esp_cipserver[]="AT+CIPSERVER=1,5000\r\n";  //����TCP�����������Ŷ˿�5000							
 u8 esp_cipmux[]="AT+CIPMUX=1\r\n";   			//�򿪶�����	
 u8 esp_rst[]="AT+RST\r\n"; 					// �����λ




u8 esp_init;	  //esp�Ƿ��ʼ����־

u8 suf; //���ݽ���
u8 z_bz=0;
u32 dht11_num=0;	//��ʪ�Ȳɼ� ��������
u32 bel_num=0;		//����״̬��� ��������
u8  ble_sta=0;		//�����Ƿ�����״̬
u8 dht_array[20];	//dht11�ɼ����ݷ�������
u8 T_tmp[7]={0,0,0,0,0,0,0};//���ʱ��������
//bit ledx1=0,ledx2=0,fm=0;			  //״̬��־





//���������ж�
//x y  Ϊ��������ֵ
//���� ���������������ֵ 0�����ڸ���
u8 Click_ble(u8 x,u16 y)
{
 u8 result=0;							  
 if(x>=200&&x<=239&&y>=300&&y<=320)	 //����
  result=17;

 return result;
}



//for ��Ļ��ʾ��������˸��  ��ʾ����״̬
//x y ��ʾλ��
//lth ����߳�
//mode 0 ��ʾ���ڰ�  1 ��ʾ������	2 ��ǰ�������ʾ��ֹ����
void ble_led_block(u16 x,u16 y,u8 lth,u8 mode)
{
  u8 k=3;
  if(mode==1)
  {
   GUI_box(x+k,y+k,x+lth,y+lth,Red3); //��ʾ��ɫ
  }
   if(mode==0)
  {
   GUI_box(x+k,y+k,x+lth,y+lth,White); //��ʾ��ɫ
  }
//  if(mode==2)
//  {
//   GUI_box(x,y,x+lth,y+lth,Black); //��ʾ���
//   lth=lth-k;
//   GUI_box(x+k,y+k,x+lth,y+lth,White); //��ʾ��ɫ
//  }
}



#define led1_x 20
#define led1_y 143
#define led_r 12

#define led2_x 58
#define led2_y 143

#define led3_x 96
#define led3_y 143


//����ģ�� ESPwifiģ�� ����������
//mode0 0 ����ģʽ   1 ESP8266ģʽ
void ble_esp_test(u8 mode0)		
{
u8 cp;
u8 time_i=0,i_esp,esp_ok=0;	
u8 ble_block;		 //����״̬���� ��˸��־
u8 dht_block=0;		 //dht״̬����  ��˸��־


//u16 zf_Tem,zf_Hp;

struct TFT_Pointer sp;		 //���崥���ṹ�����sp

GUI_Clear(White);				  //��������
GUI_box(0,300,239,301,Black);	  //�����ɫ
GUI_box(0,302,239,319,Red);


if(mode0==0)	//����ģʽ
{
	flash_binplay(0,0,60,60,224);	  //��ʾ����ͼ��
	GUI_box(61,0,239,59,Blue1);

	GUI_sprintf_hzstr16x(0,303,"BLE��������",White,1);	
	GUI_sprintf_hzstr16x(70,22,"����״̬���",Black,1);

	uart_init(9600);	 //����1 2��ʼ��Ϊ9600  
	esp_init=1;       //����ģʽ�ر�ESP��ʼ��

}
else if(mode0==1)  //esp ģʽ
{
	flash_binplay(0,0,60,60,222);	  //��ʾwifiͼ��
	GUI_box(61,0,239,59,Yellow);
	
	GUI_sprintf_hzstr16x(0,303,"ESP wifi����",White,1);

	GUI_sprintf_hzstr16x(70,2,"esp8622 wifiģ��",Black,1);

	GUI_sprintf_hzstr16x(70,22,"��������sever��",Black,1);
	uart_init(115200);	 //����1 2��ʼ��Ϊ115200  

	esp_init=0;      //espδ��ʼ����־
}



GUI_box(0,100,239,100,Black);			  //������	  ������
GUI_box(120,101,120,299,Black);
GUI_box(0,198,239,198,Black);

////////////////////////////
////////////////////////////
GUI_box(1,102,118,196,Green1);			  //��LED�����
GUI_sprintf_hzstr16x(1,102,"LED״̬",White,Green1);

GUI_arc(led1_x,led1_y,led_r,Red3,0);						 //��ѡ��ԲȦ��
GUI_arc(led2_x,led2_y,led_r,Red3,0);						 //��ѡ��ԲȦ��
GUI_arc(led3_x,led3_y,led_r,Red3,0);						 //��ѡ��ԲȦ��
GUI_arc(led1_x,led1_y,led_r-2,White,1);
GUI_arc(led2_x,led2_y,led_r-2,White,1);
GUI_arc(led3_x,led3_y,led_r-2,White,1);

GUI_sprintf_hzstr16x(6,162,"LED1",Black,1);
GUI_sprintf_hzstr16x(44,162,"LED2",Black,1);
GUI_sprintf_hzstr16x(82,162,"LED3",Black,1);

////////////////////////////
////////////////////////////
GUI_box(122,102,238,196,Black);			  //�������������
GUI_sprintf_hzstr16x(122,102,"������״̬",Yellow,1);
GUI_sprintf_hzstr16x(122,120,"����IO Pc13",Blue3,1);
GUI_sprintf_hzstr16x(122,142,"��ǰ״̬��",White,1);
GUI_sprintf_hzstr16x(203,142,"ֹͣ",White,1);

////////////////////////////
////////////////////////////
GUI_box(1,200,118,298,Gray);			  //����ʪ�������
GUI_sprintf_hzstr16x(1,200,"��ʪ�Ȳɼ�",Black,1);
GUI_sprintf_hzstr16x(1,230,"�¶�:    C",Red2,1);
GUI_sprintf_hzstr16x(1,260,"ʪ��:   %RH",Blue2,1);

////////////////////////////
////////////////////////////
GUI_box(122,200,238,298,Red);			  //�������������
GUI_sprintf_hzstr16x(122,200,"��ǰʱ��",Blue,1);


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
if(mode0==1)  //esp ģʽ   ��ʼ��ʼ������
{
    //����AT ��������				  ������ѭ��10�εȴ� 
    for(i_esp=0;i_esp<6;i_esp++)
	{

	 Uart2SendStr(esp_at);	   //����2��wifiģ�鷢������ָ�� ��AT

	 if(Data_compare2("OK"))
	 {
		 GUI_sprintf_hzstr16x(0,70,"OK,success with ESP!         ",Blue,White);
		 memset(esp_recive, 0, esp_tb);//�建������
		 re_stbz=0;				   //���ռ���������0
		 //delay_ms(600);	 
		 esp_ok++;         //ok��־�ݼ�1
		 break;		 
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR1,some problems with ESP",Blue,White);
	 delay_ms(600);
	}	 

	//����wifi����ģʽΪap+staģʽ
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cwmode);	   //����2��wifiģ�鹤��ģʽ��������  
	 if(Data_compare2("OK")||Data_compare("no change"))
	 {
		 GUI_sprintf_hzstr16x(0,70,"OK,set AP+Station with ESP   ",Blue,White);	 
		 memset(esp_recive, 0, esp_tb);//�建������
		 re_stbz=0;				   //���ռ���������0
		 //delay_ms(600);
		 esp_ok++;        //ok��־�ݼ�2
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR2,some problems with ESP",Blue,White);
	 delay_ms(600);
	}
			  
	//����wifiΪ��·������ģʽ
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cipmux);	   //����2��wifiģ�� ���ö����� ����·����ģʽ��
	 if(Data_compare2("OK"))
	 {
		 memset(esp_recive, 0, esp_tb);//�建������
		 re_stbz=0;				   //���ռ���������0
		 //delay_ms(600);		 
		 esp_ok++;        //ok��־�ݼ�3	 
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR3,some problems with ESP",Blue,White);
	 delay_ms(600);
	}

	//����wifiΪ������ģʽ ���ö˿ں�Ϊ5000
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cipserver);	   //����2����wifiģ�� ΪTCP������ģʽ�������ö˿�Ϊ5000 
	 if(Data_compare2("OK"))
	 {
		 memset(esp_recive, 0, esp_tb);//�建������
		 re_stbz=0;				   //���ռ���������0
		 //delay_ms(600);	
		 esp_ok++;        //ok��־�ݼ�4
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR4,some problems with ESP",Blue,White);
	 delay_ms(600);
	}

	//��ѯģ��ip
    for(i_esp=0;i_esp<6;i_esp++)
	{
	 Uart2SendStr(esp_cifsr);	   //����2��ѯwifiģ�� ��ǰip��ַ 
	 if(Data_compare2("OK"))
	 {
		 memset(esp_recive, 0, esp_tb);//�建������
		 re_stbz=0;				   //���ռ���������0
		 //delay_ms(600);
		 esp_ok++;        //ok��־�ݼ�5		 
		 break;
	 }
	 else  GUI_sprintf_hzstr16x(0,70,"ERROR5,some problems with ESP",Blue,White);
	 delay_ms(600);
	}

	if(esp_ok>=4)GUI_sprintf_hzstr16x(0,70,"ESP8266 wifiģ�� ��ʼ�� OK��  ",Red,White);
	else         GUI_sprintf_hzstr16x(0,70,"ESP8266 wifiģ�� ��ʼ�� ERROR!",Black,White);
	esp_init=1;   //esp��ʼ��ok ��־λ��λ

}


//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//��ѭ����
while(1)
{
  sp=TFT_Cm();	   //ɨ�败����


if(mode0==0)	   //���������ģʽ
{
    bel_num++;
	if(bel_num>=400000)
	{
		  bel_num=0;
		  Uart2SendStr(ble);	//����������atָ��  �ж��Ƿ񷵻�OK  �������ok˵������û���������� �����෴
		  if(Data_compare3("OK"))
		  {
		   ble_block=~ble_block;
		   if(ble_block==0)
		   ble_led_block(170,20,16,1); //����״ָ̬ʾ�����
		   else
		   ble_led_block(170,20,16,0); //����״ָ̬ʾ�����

		   GUI_sprintf_hzstr16x(0,70,"�������ڴ�����״̬ ������...",Blue,White);
		   ble_sta=0;//����δ����
		  }
		  else
		  {
		   ble_led_block(170,20,16,1); //����״ָ̬ʾ�����
		   GUI_sprintf_hzstr16x(0,70,"���������ӳɹ� �뷢����     ",Red,White); 
		   ble_sta=1;//����������
		  }
		  memset(RX2_buffer, 0, tbuf0);//�建������
		  RX2_num=0;				   //���ռ���������0
	
	}
}


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//����ָ����ռ���Ӧ����

if(receive_end)//���ս�����־λ	 ˵����ָ����� ��ʼ�ж�
{
  receive_end=0; //���־λ
////////////////////////////
////////////////////////////
//LED �ƿ���
  if(Data_compare(led1_open_m))
  {
	  memset(u2receive_str, 0, tbuf1);//�建������
	  led_d2=0;GUI_arc(led1_x,led1_y,led_r-2,Red,1);

	  if(mode0==0) //������״̬�·���ִ��״̬
	  Uart2SendStr(led1_open);		 //����2���Ͷ�Ӧ��Ӧ����
  }
  else if(Data_compare(led1_close_m))
  {
      memset(u2receive_str, 0, tbuf1);//�建������
	  led_d2=1; GUI_arc(led1_x,led1_y,led_r-2,White,1);

	  if(mode0==0) //������״̬�·���ִ��״̬
	  Uart2SendStr(led1_close);	 //����2���Ͷ�Ӧ��Ӧ����
  }
  else if(Data_compare(led2_open_m))
  {
	  memset(u2receive_str, 0, tbuf1);//�建������
	  led_d3=0;GUI_arc(led2_x,led2_y,led_r-2,Green,1);

	  if(mode0==0) //������״̬�·���ִ��״̬
	  Uart2SendStr(led2_open);		 //����2���Ͷ�Ӧ��Ӧ����
  }
  else if(Data_compare(led2_close_m))
  {
      memset(u2receive_str, 0, tbuf1);//�建������
	  led_d3=1; GUI_arc(led2_x,led2_y,led_r-2,White,1);

	  if(mode0==0) //������״̬�·���ִ��״̬
	  Uart2SendStr(led2_close);	 //����2���Ͷ�Ӧ��Ӧ����
  }
	///////////////////////////////////////////////////////


////////////////////////////
////////////////////////////
//������ ����
  else if(Data_compare(fm_open_m))
  {
	  memset(u2receive_str, 0, tbuf1);//�建������
	  fm_io=0;GUI_sprintf_hzstr16x(203,142,"����",White,Red);

	  if(mode0==0) //������״̬�·���ִ��״̬
	  Uart2SendStr(fm_open);		 //����2���Ͷ�Ӧ��Ӧ����
  }
  else if(Data_compare(fm_close_m))
  {
      memset(u2receive_str, 0, tbuf1);//�建������
	  fm_io=1;GUI_sprintf_hzstr16x(203,142,"ֹͣ",White,Black);

	  if(mode0==0) //������״̬�·���ִ��״̬
	  Uart2SendStr(fm_close);	     //����2���Ͷ�Ӧ��Ӧ����
  }


////////////////////////////
////////////////////////////
//���յ� ����  ��Time��+13�ֽ�ʱ������ ����ʱ��оƬ  �ֱ��ʾ���ʱ��������
//���� QGTime0010110108119/ ��ʾ 19��08��01�� 11��10��00�� 	1��ʾ������	 2����һ
  else if(u2receive_str[0]=='T'&&u2receive_str[1]=='i'&&u2receive_str[2]=='m'&&u2receive_str[3]=='e')
  {
	  //ascii 0-9ת16����
	  for(time_i=0;time_i<13;time_i++) //ascii���Ӧ16������ֵ
	  u2receive_str[time_i+4]=u2receive_str[time_i+4]-48;   //48��0x30 (0 0x30  1 0x31  2 0x32......9 0x39)
	  //13������ֵת�浽7���ֽ���
	  T_tmp[0]=u2receive_str[4]*10+u2receive_str[5];	//�� 000000000000000
	  T_tmp[1]=u2receive_str[6]*10+u2receive_str[7];	//��
	  T_tmp[2]=u2receive_str[8]*10+u2receive_str[9];	//ʱ
	  T_tmp[3]=u2receive_str[10]*10+u2receive_str[11];	//��
	  T_tmp[4]=u2receive_str[12]*10+u2receive_str[13];	//��
	  T_tmp[5]=u2receive_str[14];						//��
	  T_tmp[6]=u2receive_str[15]*10+u2receive_str[16];	//��
		
		switch(T_tmp[5])					  //����ת�� �����Э���й�ϵ Э��1������   ����1����һ
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
		RTC_Set_Date(T_tmp[6],T_tmp[4],T_tmp[3],T_tmp[5]);		//��������
//		RTC_Set_AlarmA(1,T_tmp[2],T_tmp[1],T_tmp[0]);
      //RTC_Set(2000+T_tmp[6],T_tmp[4],T_tmp[3],T_tmp[2],T_tmp[1],T_tmp[0]);      //����ʱ��
	  //RTC_Set(2020,1,5,8,30,55);      //����ʱ��


	  if(mode0==0) //������״̬�·���ִ��״̬
      Uart2SendStr(time_ok);	     //����2���Ͷ�Ӧ��Ӧ����
      memset(u2receive_str, 0, tbuf1);//�建������
  }

  //else if()   //�˴������µ�ָ��


  else 		 //������˵�������������
  {
	  if(mode0==0) //������״̬�·���ִ��״̬
      Uart2SendStr(inst_error);	  //����2���Ͷ�Ӧ��Ӧ����
      memset(u2receive_str, 0, tbuf1);//�建������  
  }

}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////



//////////////////////////////
//////////////////////////////
//��ʪ�Ȳɼ� ���ϴ�
dht11_num++;    //��ʪ�Ȳɼ������Լ�
if(dht11_num>=800000)
{
 dht11_num=0;
 DHT11_Read();
 memset(dht_array, 0, 20);//�建������

 //sprintf((char *)dht_array,"QGDdata%4x%4x/",DHT_Tempvalue,DHT_Hpvalue);
	sprintf((char *)dht_array,"%4x",DHT_Tempvalue);

 //�ж��¶��Ƿ�Ϊ����
 if(DHT_Tempvalue<0){ DHT_Tempvalue=-DHT_Tempvalue;	GUI_sprintf_char(41,230,'-',Red2,Gray,0);}
 

 DHT_Tempvalue=(DHT_Tempvalue*10)/256;	//10����ת16����
 number(49,230,DHT_Tempvalue,Red2,Gray);//ֻ��ʾ����ֵ

 DHT_Hpvalue=(DHT_Hpvalue*10)/256;	//10����ת16����
 number(49,260,DHT_Hpvalue,Blue2,Gray);//ֻ��ʾ����ֵ

 if(mode0==1) //espģʽ�� ��Ҫ�������ڷ�������
 { 
   Uart2SendStr("AT+CIPSEND=0,16\r\n"); //���espģʽ ����ESP�������� ���ݳ���
   delay_ms(600);
   Uart2SendStr(dht_array); //����2���͸�������
   delay_ms(600);
  }
 else if(ble_sta==1)		//������������״̬�ŷ���
 Uart2SendStr(dht_array); //����2���͸�������

 dht_block=~dht_block;
 if(dht_block==0)
 GUI_box(86,202,96,212,Green);	  //������״̬����˸
 else
 GUI_box(86,202,96,212,White);

}



//////////////////////////
//////////////////////////
//ʱ����ʾ ʱ����
Time_scan(140,260,White,Red,1);	//������ʾ



  if(sp.flag==1)   //����а��������¼�
  {
   cp=Click_ble(sp.x,sp.y);		//�ɼ�����ֵ
   if(cp==17) {tian(2); break;	}	    //ɨ�败���� �ж��Ƿ񷵻����˵�
  }

}


}





















