#include "./BSP/LED/led.h"
#include "./BSP/TIMER/btim.h"
/*
基本定时器：TIM6、7
寄存器：
    CR1：控制寄存器1，用于设置ARR寄存器是否又缓冲，并使能计数器。
        位7（ARPE）：自动重装载预装载使能：为1时表示ARR寄存器有缓冲作用；
            缓冲是指当我们设置完重装载值后并不会立刻生效，而是等到产生一个更新事件后才生效。这样做可以避免由于操作寄存器而造成的误差；
        位0（CEN）：计数器使能：为1时使能计数器。
        位3（OPM）：单脉冲模式： 为1时在发生下次更新事件时，计数器停止计数(清除CEN位)。
        位2（URS）：更新请求源：该位由软件设置和清除，以选择UEV事件的请求源。
            0：如果使能了中断或DMA，以下任一事件可以产生一个更新中断或DMA请求：
                    - 计数器上溢或下溢
                    - 设置UG位
                    - 通过从模式控制器产生的更新
            1：如果使能了中断或DMA，只有计数器上溢或下溢可以产生更新中断或DMA请求。
        位1（UDIS）：禁止更新 (Update disable)：该位由软件设置和清除，以使能或禁止UEV事件的产生。
            0：UEV使能。更新事件(UEV)可以由下列事件产生：
                    - 计数器上溢或下溢
                    - 设置UG位
                    - 通过从模式控制器产生的更新
                    产生更新事件后，带缓冲的寄存器被加载为预加载数值。
            1：禁止UEV。不产生更新事件(UEV)，影子寄存器保持它的内容(ARR、PSC)。但是如果设置了UG位或从模式控制器产生了一个硬件复位，则计数器和预分频器将被重新初始化。
        
    CR2：
        位6:4（MMS）：主模式选择 (Master mode selection)  
            这些位用于选择在主模式下向从定时器发送的同步信息(TRGO)，有以下几种组合：
                000：复位 – 使用TIMx_EGR寄存器的UG位作为触发输出(TRGO)。如果触发输入产生了复位
                (从模式控制器配置为复位模式)，则相对于实际的复位信号，TRGO上的信号有一定的延迟。
                001：使能 – 计数器使能信号CNT_EN被用作为触发输出(TRGO)。它可用于在同一时刻启动多
                个定时器，或控制使能从定时器的时机。计数器使能信号是通过CEN控制位和配置为门控模式
                时的触发输入的’逻辑或’产生。
                当计数器使能信号是通过触发输入控制时，在TRGO输出上会有一些延迟，除非选择了主/从模
                式(见TIMx_SMCR寄存器的MSM位)。
                010：更新 – 更新事件被用作为触发输出(TRGO)。例如一个主定时器可以作为从定时器的预分
                频器使用。
            
    DIER：DMA/中断使能寄存器，使能更新中断。
        位8（UDE）：更新DMA请求使能：为1时使能。发生溢出事件时，会产生一个更新DMA请求。
        位0（UIE）：更新中断使能：为1时使能。
        
    SR：状态寄存器，由于判断是否发生更新中断。
        位0（UIF）：更新中断标志，由软件清除，在发生更新中断时由硬件置位。
        
    EGR：事件产生寄存器，用软件产生更新事件。
        位0（UG）：产生更新事件 (Update generation)
            该位由软件设置，由硬件自动清除。
                0：无作用
                1：重新初始化定时器的计数器并产生对寄存器的更新。注意：预分频器也被清除(但预分频系
                数不变)。
        这里解释一下，在定时器溢出时会自动产生一个更新事件，EGR寄存器的作用是为了可以随时随地用软件产生事件。
    CNT：16位计数器，和窗口看门狗不同，16个位全部用于计数，并可在任意时间读写它。
    
    PSC：预分配器：16位全部用于预分频器数值。
        计数器时钟频率：Fck_psc/(PSC+1)。
        在每一次更新事件时才会把PSC的数值传到实际的预分频寄存器里。
        
    ARR：自动重装载寄存器：16位全部用于重装载数值。
        如果值设为0，则计数器停止。
    
*/
TIM_HandleTypeDef g_tim6_handle;  /* 定时器6句柄 */
TIM_HandleTypeDef g_tim7_handle;  /* 定时器7句柄 */

/**
 * @brief       基本定时器TIMX定时中断初始化函数
 * @note
 *              基本定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              基本定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
                正点原子采用的是
 *
 * @param       arr: 自动重装值，0~65535。
 * @param       psc: 时钟预分频数,数值从0~65535，实际分频数值为psc+1
                这两个寄存器只起缓存作用，实际上起作用的是这两个寄存器的影子寄存器
 * @retval      无
 */
void btim_tim6_int_init(uint16_t arr, uint16_t psc)
{
    g_tim6_handle.Instance = TIM6;                      /* 通用定时器X基地址 */
    g_tim6_handle.Init.Prescaler = psc;                          /* 设置预分频系数 */
    g_tim6_handle.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 递增计数模式 基本定时器只允许递增模式*/
    /* 计数模式分为递增、递减以及中心对齐：
        TIM_COUNTERMODE_UP：向上计数模式。计数器的值的变化长这样：////
        TIM_COUNTERMODE_DOWN：向下计数模式。计数器的值的变化长这样：\\\\
        TIM_COUNTERMODE_CENTERAligned1：中心对齐模式1，计数器从 0 开始递增到 ARR，然后递减到 0，再递增，只有在开始递减时才触发更新事件（触发周期被延长了一倍）。长这样：/\/\/\/
        TIM_COUNTERMODE_CENTERAligned2：中心对齐模式2，计数器从 0 开始递增到 ARR，然后递减到 0，再递增。只有在开始递增时才触发更新事件。长这样：/\/\/\/
        TIM_COUNTERMODE_CENTERAligned3：中心对齐模式3，计数器从 0 开始递增到 ARR，然后递减到 0，再递增，递增递减时都会触发更新事件。长这样：/\/\/\/    */
    g_tim6_handle.Init.Period = arr;                             /* 自动装载值 */
    g_tim6_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; 
                                                                 /*启用ARR的缓存功能，如果不想启用，请把ENABLE改成DIEABLE*/
    HAL_TIM_Base_Init(&g_tim6_handle);                           /* 配置定时器参数 */

    HAL_TIM_Base_Start_IT(&g_tim6_handle);    /* 使能定时器x及其更新中断 */
}

void btim_tim7_int_init(uint16_t arr, uint16_t psc)
{
    g_tim7_handle.Instance = TIM7;                      /* 通用定时器X基地址 */
    g_tim7_handle.Init.Prescaler = psc;                          /* 设置预分频系数 */
    g_tim7_handle.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 递增计数模式 基本定时器只允许递增模式*/
    g_tim7_handle.Init.Period = arr;                             /* 自动装载值 */
    g_tim7_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; 
                                                                 /*启用ARR的缓存功能，如果不想启用，请把ENABLE改成DIEABLE*/
    HAL_TIM_Base_Init(&g_tim7_handle);                           /* 配置定时器参数 */

    HAL_TIM_Base_Start_IT(&g_tim7_handle);    /* 使能定时器x及其更新中断 */
}


/**
 * @brief       定时器底层驱动，开启时钟，设置中断优先级
                此函数会被HAL_TIM_Base_Init()函数调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)                /* 如果是TIM6 */
    {
        __HAL_RCC_TIM6_CLK_ENABLE();                     /* 宏函数，使能TIM时钟，即__HAL_RCC_TIM6_CLK_ENABLE() */
        HAL_NVIC_SetPriority(TIM6_IRQn, 1, 3); /* 抢占2，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM6_IRQn);         /* 开启TIM6中断 */
    }
    if (htim->Instance == TIM7)                         /* 如果是TIM7 */
    {
        __HAL_RCC_TIM7_CLK_ENABLE();                    /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM7_IRQn, 2, 2);          /* 抢占3，子优先级2，组2 */
        HAL_NVIC_EnableIRQ(TIM7_IRQn);                  /* 开启TIM7中断 */
    }
}

/**
 * @brief       定时器TIMX中断服务函数，当定时器TIMx溢出时，会调用这个函数
                    在本例中x=6 。
 * @param       无
 * @retval      无
 */
void TIM6_IRQHandler(void)
{
    static int TIM6_IT_NUM = 0;
    TIM6_IT_NUM++;
    //__HAL_TIM_CLEAR_IT(&g_tim6_handle, TIM_IT_UPDATE);
    LED1_TOGGLE(); /* LED1反转 */
    lcd_show_num(10,10,TIM6_IT_NUM,5,32,WHITE);
    //printf("TIM6 interrupt!\r\n");
    HAL_TIM_IRQHandler(&g_tim6_handle); /* 定时器中断公共处理函数 */
}

void TIM7_IRQHandler(void)
{
    static int TIM7_IT_NUM = 0;
    TIM7_IT_NUM++;
    //__HAL_TIM_CLEAR_IT(&g_tim7_handle, TIM_IT_UPDATE);
    LED0_TOGGLE(); /* LED0反转 */
    lcd_show_num(10,50,TIM7_IT_NUM,5,32,WHITE);
    //printf("TIM7 interrupt!\r\n");
    HAL_TIM_IRQHandler(&g_tim7_handle); /* 定时器中断公共处理函数 */
}

/**
 * @brief       定时器更新中断回调函数，所有的定时器中断都会调用这个函数，因此要判断
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == BTIM_TIMX_INT)
    {
        //LED1_TOGGLE(); /* LED1反转 */
    }
}




