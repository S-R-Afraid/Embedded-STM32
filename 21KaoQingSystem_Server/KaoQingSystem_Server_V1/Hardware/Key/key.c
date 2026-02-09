#include "key.h"

uint8_t Key1_Status(void)
{
    if (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET)
    {
        while (HAL_GPIO_ReadPin(Key1_GPIO_Port, Key1_Pin) == GPIO_PIN_SET)
            ;

        return Key_Press;
    }

    return Key_Release;
}

uint8_t Key2_Status(void)
{
    if (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_RESET)
    {
        while (HAL_GPIO_ReadPin(Key2_GPIO_Port, Key2_Pin) == GPIO_PIN_RESET)
            ;

        return Key_Press;
    }

    return Key_Release;
}

uint8_t Key3_Status(void)
{
    if (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_RESET)
    {
        while (HAL_GPIO_ReadPin(Key3_GPIO_Port, Key3_Pin) == GPIO_PIN_RESET)
            ;

        return Key_Press;
    }

    return Key_Release;
}


volatile uint8_t key1_flag = 0;
volatile uint8_t key2_flag = 0;
volatile uint8_t key3_flag = 0;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == Key1_Pin)
    {
        key1_flag = 1;
    }
    else if(GPIO_Pin == Key2_Pin)
    {
        key2_flag = 1;
    }
    else if(GPIO_Pin == Key3_Pin)
    {
        key3_flag = 1;
    }
}
