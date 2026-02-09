#include "./stm32f1xx_it.h"
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/BEEP/beep.h"
#include "./BSP/EXTI/exti.h"
//#include "./BSP/OLED/OLED.h"
#include "./BSP/LCD/lcd.h"
/*
当按下KEY_UP时，切换exit.c里的exir_bool变量，
只有当此变量为1时按KEY1\KEY0才有反应。
*/
int key_num = 0;
int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(RCC_PLL_MUL9);
    delay_init(72);
    usart_init(115200);
    led_init();
    extix_init();
    LED0(0);
    toggle_exit_bool();
    lcd_init();
    g_back_color = BLACK ;
    g_point_color=WHITE;
    lcd_clear(BLACK);
    
    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.Pin = GPIO_PIN_6;                   /* LED0ҽ½Š*/
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* ͆ͬʤ³ö */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* ɏÀ­ */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* ¸ߋ٠*/
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);
    while (1)
    {   
        //LED1_TOGGLE(); 
        HAL_GPIO_TogglePin(GPIOE,GPIO_PIN_6);
        lcd_show_num(10,10,key_num,4,32,WHITE);
        delay_ms(10);
    }
}


