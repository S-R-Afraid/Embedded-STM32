#include "led.h"

void LED_Trun_On(uint8_t led)
{
    if (R_LED == led)
    {
        // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(R_LED_GPIO_Port, R_LED_Pin, GPIO_PIN_RESET);
    }
    else if (B_LED == led)
    {
        HAL_GPIO_WritePin(B_LED_GPIO_Port, B_LED_Pin, GPIO_PIN_RESET);
    }
}

void LED_Trun_Off(uint8_t led)
{
    if (R_LED == led)
    {
        // HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(R_LED_GPIO_Port, R_LED_Pin, GPIO_PIN_SET);
    }
    else if (B_LED == led)
    {
        HAL_GPIO_WritePin(B_LED_GPIO_Port, B_LED_Pin, GPIO_PIN_SET);
    }
}

void LED_Trun_ALL_Off(void)
{
    HAL_GPIO_WritePin(R_LED_GPIO_Port, R_LED_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(B_LED_GPIO_Port, B_LED_Pin, GPIO_PIN_SET);
}

void LED_Trun_ALL_On(void)
{
    HAL_GPIO_WritePin(R_LED_GPIO_Port, R_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(B_LED_GPIO_Port, B_LED_Pin, GPIO_PIN_RESET);
}

void LED_Toggle(uint8_t led)
{
    if (R_LED == led)
    {
        HAL_GPIO_TogglePin(R_LED_GPIO_Port, R_LED_Pin);
    }
    else if (B_LED == led)
    {
        HAL_GPIO_TogglePin(B_LED_GPIO_Port, B_LED_Pin);
    }
}

void LED_Loop(void)
{
    LED_Trun_ALL_Off();
    
    for(int i = 0; i < 5; i++)
    {
        LED_Trun_On(R_LED);
        HAL_Delay(500);//ms
        LED_Trun_Off(R_LED);
        HAL_Delay(500);

        LED_Trun_On(B_LED);
        HAL_Delay(500);//ms
        LED_Trun_Off(B_LED);
        HAL_Delay(500);
    }
}
