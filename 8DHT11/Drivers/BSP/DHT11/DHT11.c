#include "DHT11.h"
#include "stm32f1xx_hal.h"
#include "./SYSTEM/delay/delay.h"


//void Delay_us(uint16_t us)
//{
//    __HAL_TIM_SET_COUNTER(&htim2,0);
//    while(__HAL_TIM_GET_COUNTER(&htim2)<us);
//}

void DHT11_W_DATA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(DHT_DATA_GPIO_Port,DHT_DATA_Pin,(GPIO_PinState)BitValue);
}

uint8_t DHT11_R_DATA(void)
{    
    return HAL_GPIO_ReadPin(DHT_DATA_GPIO_Port,DHT_DATA_Pin);
}

void DHT11_Mode_Out_PP(void) 
{ 
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DHT_DATA_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_Mode_IPU(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT_DATA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT_DATA_GPIO_Port, &GPIO_InitStruct);
}

void DHT11_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};  // 初始化GPIO配置结构体


    __HAL_RCC_GPIOA_CLK_ENABLE();  // 替换为实际的GPIO端口时钟

    // 2. 配置GPIO为推挽输出模式
    GPIO_InitStruct.Pin = DHT_DATA_Pin  ;    // 替换为实际的引脚号
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // 设置为推挽输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;           // 使用内部上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // 设置GPIO速度为高速（50MHz）
    HAL_GPIO_Init(DHT_DATA_GPIO_Port, &GPIO_InitStruct);       // 初始化GPIOA的引脚

    // 3. 将GPIO引脚设置为高电平
    HAL_GPIO_WritePin(DHT_DATA_GPIO_Port, DHT_DATA_Pin, GPIO_PIN_SET);  // 设置引脚为高电平
}



uint8_t DHT11_ReadByte(void)
{  
    uint8_t i, temp = 0;

    for (i = 0; i < 8; i++)    
    {    
       while (DHT11_R_DATA() == RESET)
           ; 
        delay_us(54);
        if(DHT11_R_DATA() == SET)
        {
            while (DHT11_R_DATA() == SET);
            temp |= 0x80 >> i;
        }
    }
    return temp;
}

uint8_t DHT11_ReadData(DHT11_Data *Data)
{
    DHT11_Mode_Out_PP() ;
    DHT11_W_DATA(RESET);
    HAL_Delay(20);

    DHT11_W_DATA(SET);

    delay_us(100);

    DHT11_Mode_IPU();

    if(DHT11_R_DATA() == RESET)
    {
        while (DHT11_R_DATA() == RESET);
        while(DHT11_R_DATA() == SET);

        Data->Humidity_H = DHT11_ReadByte();
        Data->Humidity_L = DHT11_ReadByte();
        Data->Temperature_H = DHT11_ReadByte();
        Data->Temperature_L = DHT11_ReadByte();
        Data->CheckSum = DHT11_ReadByte();

        DHT11_Mode_Out_PP() ;
        DHT11_W_DATA(SET);

        uint8_t sum = Data->Humidity_H + Data->Humidity_L + Data->Temperature_H + Data->Temperature_L;

        if(sum != Data->CheckSum)
        {
            return ERROR;
        }
        else
        {
            return SUCCESS;
        }
    }
    
    else
    {
        return ERROR;
    }
}
