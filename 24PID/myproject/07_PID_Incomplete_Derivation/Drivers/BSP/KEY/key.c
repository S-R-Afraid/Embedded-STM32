#include "./BSP/KEY/key.h"
#include "./SYSTEM/delay/delay.h"

uint8_t Key_Num;
/**
 * @brief       按键初始化函数
 * @param       无
 * @retval      无
 */
void key_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    KEY0_GPIO_CLK_ENABLE();                                     /* KEY0时钟使能 */
    KEY1_GPIO_CLK_ENABLE();                                     /* KEY1时钟使能 */
    WKUP_GPIO_CLK_ENABLE();                                     /* WKUP时钟使能 */

    gpio_init_struct.Pin = KEY0_GPIO_PIN;                       /* K1引脚 */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;                    /* 输入 */
    gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 高速 */
    HAL_GPIO_Init(KEY0_GPIO_PORT, &gpio_init_struct);           /* KEY0引脚模式设置,上拉输入 */

    gpio_init_struct.Pin = KEY1_GPIO_PIN;                       /* K2引脚 */
    HAL_GPIO_Init(KEY1_GPIO_PORT, &gpio_init_struct);           /* KEY1引脚模式设置,上拉输入 */

    gpio_init_struct.Pin = WKUP_GPIO_PIN;                       /* K3引脚 */
    HAL_GPIO_Init(WKUP_GPIO_PORT, &gpio_init_struct);           /* WKUP引脚模式设置,下拉输入 */
    
    gpio_init_struct.Pin = GPIO_PIN_12;                         /* K4引脚 */
    HAL_GPIO_Init(WKUP_GPIO_PORT, &gpio_init_struct);           /* WKUP引脚模式设置,下拉输入 */

}


uint8_t Key_GetState(void)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == 0)
    {
        return 1;
    }
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == 0)
    {
        return 2;
    }
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == 0)
    {
        return 3;
    }
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == 0)
    {
        return 4;
    }
    return 0;
}


uint8_t Key_GetNum(void)
{
    uint8_t Temp;
    if (Key_Num)
    {
        Temp = Key_Num;
        Key_Num = 0;
        return Temp;
    }
    return 0;
}



void Key_Tick(void)
{
    static uint8_t Count;
    static uint8_t CurrState, PrevState;
    
    Count ++;
    if (Count >= 20)
    {
        Count = 0;
        
        PrevState = CurrState;
        CurrState = Key_GetState();
        
        if (CurrState == 0 && PrevState != 0){
            Key_Num = PrevState;
        }
//        if (CurrState == PrevState ){
//            Key_Num = 0;
//        }
        
    }
}










