#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "RP.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "AD.h"

/*角度传感器测试*/
/*下载此段程序后，旋转角度传感器，OLED显示的AD值会对应变化*/
int main(void)
{
	/*模块初始化*/
	OLED_Init();		//OLED初始化
	AD_Init();			//角度传感器初始化
	
	while (1)
	{
		/*不断读取角度传感器的AD值并显示在OLED上*/
		OLED_Printf(0, 0, OLED_8X16, "AD:%04d", AD_GetValue());
		OLED_Update();
	}
}
