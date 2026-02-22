//#include "stm32f10x.h"                  // Device header
#include "./BSP/Encoder/Encoder.h"
/**
  * 函    数：编码器初始化
  * 参    数：无
  * 返 回 值：无
  */
TIM_Encoder_InitTypeDef Encoder_ConfigStructure;
TIM_HandleTypeDef TIM_EncoderHandle;        /* 定时器3句柄 */

void Encoder_Init(void)
{
	/*开启时钟*/
	__HAL_RCC_GPIOB_CLK_ENABLE();
    
	/*GPIO初始化*/
    GPIO_InitTypeDef gpio_init_struct;
    gpio_init_struct.Pin = GPIO_PIN_6;                      /* 引脚 */
    gpio_init_struct.Mode = GPIO_MODE_INPUT;                /* 输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* 高速 */
    //gpio_init_struct.Alternate = ENCODER_TIM_CH1_GPIO_AF;    /* 设置复用 */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);                /* 初始化引脚 */
    
    
    gpio_init_struct.Pin = GPIO_PIN_7;                      /* 引脚 */
    //gpio_init_struct.Alternate = ENCODER_TIM_CH2_GPIO_AF;    /* 设置复用 */
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);                /* 初始化引脚 */
    //将PA6和PA7引脚初始化为上拉输入
    
    
	/*时基单元初始化*/
    ENCODER_TIM_CLK_ENABLE();
    TIM_EncoderHandle.Instance = ENCODER_TIM;
    TIM_EncoderHandle.Init.Prescaler = ENCODER_TIM_PRESCALER;
    TIM_EncoderHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    TIM_EncoderHandle.Init.Period = ENCODER_TIM_PERIOD;
    TIM_EncoderHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    TIM_EncoderHandle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
     
    /* 设置编码器倍频数 */
    Encoder_ConfigStructure.EncoderMode = TIM_ENCODERMODE_TI12;
    /* 编码器接口通道1设置 */
    Encoder_ConfigStructure.IC1Polarity = TIM_ICPOLARITY_RISING;
    Encoder_ConfigStructure.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    Encoder_ConfigStructure.IC1Prescaler = TIM_ICPSC_DIV1;
    Encoder_ConfigStructure.IC1Filter = 0;
    /* 编码器接口通道2设置 */
    Encoder_ConfigStructure.IC2Polarity = TIM_ICPOLARITY_FALLING;
    Encoder_ConfigStructure.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    Encoder_ConfigStructure.IC2Prescaler = TIM_ICPSC_DIV1;
    Encoder_ConfigStructure.IC2Filter = 0;
    /* 初始化编码器接口 */
    HAL_TIM_Encoder_Init(&TIM_EncoderHandle, &Encoder_ConfigStructure);
   
    /* 清零计数器 */
    __HAL_TIM_SET_COUNTER(&TIM_EncoderHandle, ENCODER_TIM_PERIOD/2);
   
    /* 清零中断标志位 */
    __HAL_TIM_CLEAR_IT(&TIM_EncoderHandle,TIM_IT_UPDATE);
    /* 使能定时器的更新事件中断 */
    __HAL_TIM_ENABLE_IT(&TIM_EncoderHandle,TIM_IT_UPDATE);
    /* 设置更新事件请求源为：定时器溢出 */
    __HAL_TIM_URS_ENABLE(&TIM_EncoderHandle);
   
    /* 设置中断优先级 */
    HAL_NVIC_SetPriority(TIM3_IRQn, 5, 1);
    /* 使能定时器中断 */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
   
    /* 使能编码器接口 */
    HAL_TIM_Encoder_Start(&TIM_EncoderHandle, TIM_CHANNEL_ALL);
	
//	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
//	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
//	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //计数器模式，选择向上计数
//	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;               //计数周期，即ARR的值
//	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;                //预分频器，即PSC的值
//	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            //重复计数器，高级定时器才会用到
//	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);             //将结构体变量交给TIM_TimeBaseInit，配置TIM3的时基单元
//	
//	/*输入捕获初始化*/
//	TIM_ICInitTypeDef TIM_ICInitStructure;							//定义结构体变量
//	TIM_ICStructInit(&TIM_ICInitStructure);							//结构体初始化，若结构体没有完整赋值
//																	//则最好执行此函数，给结构体所有成员都赋一个默认值
//																	//避免结构体初值不确定的问题
//	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;				//选择配置定时器通道1
//	TIM_ICInitStructure.TIM_ICFilter = 0xF;							//输入滤波器参数，可以过滤信号抖动
//	TIM_ICInit(TIM3, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道
//	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;				//选择配置定时器通道2
//	TIM_ICInitStructure.TIM_ICFilter = 0xF;							//输入滤波器参数，可以过滤信号抖动
//	TIM_ICInit(TIM3, &TIM_ICInitStructure);							//将结构体变量交给TIM_ICInit，配置TIM3的输入捕获通道
//	
//	/*编码器接口配置*/
//	TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
//																	//配置编码器模式以及两个输入通道是否反相
//																	//注意此时参数的Rising和Falling已经不代表上升沿和下降沿了，而是代表是否反相
//																	//此函数必须在输入捕获初始化之后进行，否则输入捕获的配置会覆盖此函数的部分配置
//	
//	/*TIM使能*/
//	TIM_Cmd(TIM3, ENABLE);			//使能TIM3，定时器开始运行
}

/**
  * 函    数：获取编码器的增量值
  * 参    数：无
  * 返 回 值：自上此调用此函数后，编码器的增量值
  */
int16_t Encoder_Get(void)
{
	/*使用Temp变量作为中继，目的是返回CNT后将其清零*/
	int16_t Temp;
	Temp = __HAL_TIM_GetCounter(&TIM_EncoderHandle)-ENCODER_TIM_PERIOD/2;
	__HAL_TIM_SetCounter(&TIM_EncoderHandle, ENCODER_TIM_PERIOD/2);
	return Temp;
}
