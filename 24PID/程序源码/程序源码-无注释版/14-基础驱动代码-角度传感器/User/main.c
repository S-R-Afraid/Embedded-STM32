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
int main(void)
{
	OLED_Init();
	AD_Init();
	
	while (1)
	{
		OLED_Printf(0, 0, OLED_8X16, "AD:%04d", AD_GetValue());
		OLED_Update();
	}
}
