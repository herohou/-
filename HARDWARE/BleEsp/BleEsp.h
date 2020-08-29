

#ifndef __BLEESP_H
#define __BLEESP_H
#include "mcusys.h"




void ble_esp_test(u8 mode);	   //蓝牙模块 ESPwifi模块 测试主函数	

extern u8 esp_init;	  //esp是否初始化标志

u8 Click_ble(u8 x,u16 y);
void ble_led_block(u16 x,u16 y,u8 lth,u8 mode);


#endif 















