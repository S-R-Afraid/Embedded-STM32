#include "./BSP/TIMER/gtim.h"
#include "./BSP/LED/led.h"
/*------------------------基本介绍------------------------*/

/*
通用定时器：TIM2、3、4、5
    和基本定时器相比，通用定时器的计数器允许递增、递减、中心对齐三种计数模式；
        用途上不仅可以触发DAC（数模转换），还可以触发ADC（模数转换）；
        中断条件上，除了更新事件外还有触发事件、输入捕获、输出比较
        
        额外拥有4个独立通道，可用于输入捕获、输出比较、PMW输出、单脉冲模式
        
    可以用外部信号控制定时器，可以实现多个定时器互联的同步电路（级联，即用某个定时器的溢出事件来作为下一个定时器的时钟信号）
    
    支持编码器和霍尔传感器电路
    
    时钟源可以来自于：系统总线（APB,即内部时钟）、内部触发输入时钟（来自鱼其它定时器的控制器等）、IO引脚复用为定时器ETR引脚（外部时钟模式二）、本定时器通道一、通道二（外部时钟模式一）
    IO引脚复用查看STM32F103ZET6.pdf，搜索ETR即可。搜到的第二个TIM8_ETR就是，前面的PA0-WKUP就是引脚名称，在前面的数字即使引脚编号，查看自己芯片的封装类别即可。
    通道同理，搜CH1和CH2即可。

    控制器：可以接入其它定时器的内部触发输入，也可以到DAC/ADC。

    输入捕获、捕获/比较、输出比较：以CH1通道为例
        外部信号->IO引脚->（滤波器、边沿检测器）->产生捕获信号到捕获寄存器1或者2（除了事件外还可以产生中断，只不过中断需要配置）->
                将计数器当前值转移到捕获/比较影子寄存器，输入模式下再转移到捕获/比较预装载寄存器用于程序读取，输出模式下将捕获/比较预装载寄存器值转移到其影子寄存器（缓冲）->与计数器值比较->若相等->
                改变输出信号(输出信号高电平有效)，并产生比较事件（如果配置了还会产生中断）->
                信号传到输出控制模块->输出控制模块产生输出信号到本通道输出引脚。
                    注：输出引脚和输入引脚是同一引脚，它们是分时复用的关系，同时只有一边有效。

*/



/*------------------------时钟源介绍------------------------*/
/*
    内部时钟，设置 TIMx_SMCR 的 SMS=0000，与基本定时器一样，时钟源来自系统总线（APB）时钟，预分频器大于2时会*2 。
    外部时钟模式一，设置 TIMx_SMCR 的 SMS=1111，来自外部输入引脚（TIx），一共有三种信号：
        当时钟源来自边沿检测器前的信号（TI1F_ED信号），时钟源信号是一个双边沿检测信号，不论是上升沿还是下降沿都会触发计数器的计数，注意这个信号只对通道1有效。
        来自于边沿检测器后的信号（TI1FP1、TI2FP2信号），是一个单边沿检测信号，根据配置决定上升沿还是下降沿，对通道1和2都有效。
        这三种信号都可以设置是否滤波（TIMx_CCMR1的IC1F位），以及滤波器滤波的频率（TIMx_CR1的CDK位）。
            数字滤波器的详细说明：在设置好工作频率后，还会设置采样次数（N），只有在采样器以工作频率连续采样到N个与当前输出值不同的输入信号时，才会更新输出值，当 8 次采样中有高有低，那就保持原来的输出。
                比如N等于8，原本输出低电平，第一次采样信号为高电平，第二次采样为低电平，但此时输出仍是低电平，且这8次采样的结果不会计入下一轮采样；只有当9~16次采样信号全部都是高电平时输出才会是高电平。
    外部时钟模式2，设置 TIMx_SMCR 的 ECE=1，来自外部触发输入（TIMx_ETR）；
    内部触发输入，来自其它定时器的控制器（TIRx，x=0、1、2、3）。设置方式参考F10XXX参考手册的定时器同步章节，各个TIR信号编码查看从模式控制寄存器(TIMx_SMCR)的TS位，关于每个定时器中ITRx的细节，参见参考手册的表78。
*/

/*------------------------输出比较部分介绍------------------------*/
/*
    捕获/比较预装载寄存器何时会将值装在到其影子寄存器里(三个条件必须同时满足)：
        当CCR1寄存器未处于被操作状态时；
        当通道被配置未输出时；
        该寄存器没有配置缓冲使能或定时器产生更新事件时。

    捕获/比较通道的输出部分(以通道1为例)：
        寄存器将比较结果传到输出控制模块，输出控制模块根据配置（CCMR1的OC1M位）产生输出信号(OC1REF信号，高电平有效)，这个信号再根据配置（CCER的CC1P位）决定是否反转，并输出到IO引脚（OC1信号）。
    
    输出PWM原理：
        当计数器的值大于等于捕获/比较寄存器(CRR)的值时，输出逻辑为1，否则为0。
        那么我们通过设置计数周期以及重装载寄存器（ARR）的值就可以产生一个我们需要的波形。
        ARR越大，周期越大、频率越小；CRR则与占空比相关，具体看计数器的计数方式。

*/ 

/*------------------------PWM模式介绍------------------------*/
/*
    PWM有两种模式。
        PWM模式一：CNT<CCR1是为有效电平（OC1REF=1），CNT>=CR1是为无效电平（OC1REF=0）。
        PWM模式二：与模式一相反。

*/

/*---------------输入捕获测量脉宽信号原理---------------*/
/*
简单来说就是通过两次捕获事件时计数器值之差，再加上这段时间内溢出事件的次数，就可以计算两次捕获事件的间隔时间。

具体过程：捕获到一个沿上升沿检测事件后，计数器清零，同时把检测改成沿下降沿检测。记录溢出事件发生的次数，发生第二次捕获事件时再次读取计数器的值，计算。

公式：T=(N*(ARR+1)+CRR)*(psc+1)/Ft；
    T是持续时间，N为两次捕获事件期间的溢出次数，
    ARR为重装载值，CRR是第二次捕获事件时计数器值，
    psc是定时器预分频系数，Ft是定时器时钟源频率。
*/





TIM_HandleTypeDef g_tim3_handle; /* 定时器3句柄 */
TIM_HandleTypeDef g_tim2_handle; /* 定时器2句柄 */

/**
 * @brief       通用定时器TIMX定时中断初始化函数
 * @note
 *              通用定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              通用定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void gtim_tim3_int_init(uint16_t arr, uint16_t psc)
{
    __HAL_RCC_TIM3_CLK_ENABLE();                                 /* 使能TIMx时钟 */

    g_tim3_handle.Instance = TIM3;                              /* 通用定时器x */
    g_tim3_handle.Init.Prescaler = psc;                         /* 预分频系数 */
    g_tim3_handle.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
    g_tim3_handle.Init.Period = arr;                            /* 自动装载值 */
    HAL_TIM_Base_Init(&g_tim3_handle);

    HAL_NVIC_SetPriority(TIM3_IRQn, 1, 3);                      /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ(TIM3_IRQn);                              /* 开启ITMx中断 */

    HAL_TIM_Base_Start_IT(&g_tim3_handle);                      /* 使能定时器x和定时器x更新中断 */
}

void gtim_tim2_int_init(uint16_t arr, uint16_t psc)
{
    __HAL_RCC_TIM2_CLK_ENABLE();                                 /* 使能TIMx时钟 */

    g_tim2_handle.Instance = TIM2;                              /* 通用定时器x */
    g_tim2_handle.Init.Prescaler = psc;                         /* 预分频系数 */
    g_tim2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
    g_tim2_handle.Init.Period = arr;                            /* 自动装载值 */
    HAL_TIM_Base_Init(&g_tim2_handle);

    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 2);                      /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ(TIM2_IRQn);                              /* 开启ITMx中断 */

    HAL_TIM_Base_Start_IT(&g_tim2_handle);                      /* 使能定时器x和定时器x更新中断 */
}

/**
 * @brief       定时器中断服务函数
 * @param       无
 * @retval      无
 */
void TIM3_IRQHandler(void)
{
    /* 直接通过判断中断标志位的方式 */
    if(__HAL_TIM_GET_FLAG(&g_tim3_handle, TIM_FLAG_UPDATE) != RESET)
    {
        //LED0_TOGGLE();
        //HAL_TIM_IRQHandler(&g_tim3_handle); /* 定时器中断公共处理函数,在btim.c里定义的 如果取消注释会看见0和1一起闪烁 */
        //__HAL_TIM_CLEAR_IT(&g_tim3_handle, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
    }
}

void TIM2_IRQHandler(void)
{
    /* 直接通过判断中断标志位的方式 */
    if(__HAL_TIM_GET_FLAG(&g_tim2_handle, TIM_FLAG_UPDATE) != RESET)
    {
        LED1_TOGGLE();
        //HAL_TIM_IRQHandler(&g_tim2_handle); 
        __HAL_TIM_CLEAR_IT(&g_tim2_handle, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
    }
}

/*********************************以下是通用定时器PWM输出实验程序*************************************/

TIM_HandleTypeDef g_timx_pwm_chy_handle;        /* 定时器x句柄 */


/**
 * @brief       通用定时器TIMX 通道Y PWM输出 初始化函数（使用PWM模式1）
 * @note
 *              通用定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              通用定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void gtim_timx_pwm_chy_init(uint16_t arr, uint16_t psc)
{
    TIM_OC_InitTypeDef timx_oc_pwm_chy  = {0};                          /* 定时器PWM输出配置 */

    g_timx_pwm_chy_handle.Instance = GTIM_TIMX_PWM;                     /* 定时器x */
    g_timx_pwm_chy_handle.Init.Prescaler = psc;                         /* 定时器分频 */
    g_timx_pwm_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
    g_timx_pwm_chy_handle.Init.Period = arr;                            /* 自动重装载值 */
    HAL_TIM_PWM_Init(&g_timx_pwm_chy_handle);                           /* 初始化PWM */

    timx_oc_pwm_chy.OCMode = TIM_OCMODE_PWM1;                           /* 模式选择PWM1 */
    timx_oc_pwm_chy.Pulse = arr / 2;                                    /* 设置比较值,此值用来确定占空比 */
                                                                        /* 默认比较值为自动重装载值的一半,即占空比为50% */
    timx_oc_pwm_chy.OCPolarity = TIM_OCPOLARITY_LOW;                    /* 输出比较极性为低 */
    HAL_TIM_PWM_ConfigChannel(&g_timx_pwm_chy_handle, &timx_oc_pwm_chy, GTIM_TIMX_PWM_CHY); /* 配置TIMx通道y */
    HAL_TIM_PWM_Start(&g_timx_pwm_chy_handle, GTIM_TIMX_PWM_CHY);       /* 开启对应PWM通道 */
}

/**
 * @brief       定时器底层驱动，时钟使能，引脚配置
                此函数会被HAL_TIM_PWM_Init()调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == GTIM_TIMX_PWM)
    {
        GPIO_InitTypeDef gpio_init_struct;
        GTIM_TIMX_PWM_CHY_GPIO_CLK_ENABLE();               /* 开启通道y的CPIO时钟 */
        GTIM_TIMX_PWM_CHY_CLK_ENABLE();

        gpio_init_struct.Pin = GTIM_TIMX_PWM_CHY_GPIO_PIN; /* 通道y的CPIO口 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;           /* 复用推完输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;               /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;     /* 高速 */
        HAL_GPIO_Init(GTIM_TIMX_PWM_CHY_GPIO_PORT, &gpio_init_struct);
        GTIM_TIMX_PWM_CHY_GPIO_REMAP();                    /* IO口REMAP设置, 是否必要查看头文件配置的说明 */
    }
}

/*********************************通用定时器输入捕获实验程序*************************************/

TIM_HandleTypeDef g_timx_cap_chy_handle;      /* 定时器x句柄 */

/**
 * @brief       通用定时器TIMX 通道Y 输入捕获 初始化函数
 * @note
 *              通用定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              通用定时器的时钟为APB1时钟的2倍, 而APB1为36M, 所以定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void gtim_timx_cap_chy_init(uint16_t arr, uint16_t psc)
{
    TIM_IC_InitTypeDef timx_ic_cap_chy = {0};

    g_timx_cap_chy_handle.Instance = GTIM_TIMX_CAP;                     /* 定时器5 */
    g_timx_cap_chy_handle.Init.Prescaler = psc;                         /* 定时器分频 */
    g_timx_cap_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
    g_timx_cap_chy_handle.Init.Period = arr;                            /* 自动重装载值 */
    HAL_TIM_IC_Init(&g_timx_cap_chy_handle);

    timx_ic_cap_chy.ICPolarity = TIM_ICPOLARITY_RISING;                 /* 上升沿捕获 一共有三种模式RISING、FALLING、BOTHEDGE */
    timx_ic_cap_chy.ICSelection = TIM_ICSELECTION_DIRECTTI;             /* 映射到TI1上，即映射到通道1上；一共有三种模式DIRECTTI、INDIRECTTITRC，即TI1、YI2、TRC */
    timx_ic_cap_chy.ICPrescaler = TIM_ICPSC_DIV1;                       /* 配置输入分频，不分频 可以设置为每2、4、8个事件触发一次捕获，吧1改成对应数字即可*/
    timx_ic_cap_chy.ICFilter = 0;                                       /* 配置输入滤波器，不滤波。这个值可以是0~16之间的整数，对应16个模式见捕获/比较模式寄存器1(TIMx_CCMR1)的位7：4 */
    HAL_TIM_IC_ConfigChannel(&g_timx_cap_chy_handle, &timx_ic_cap_chy, GTIM_TIMX_CAP_CHY);  /* 配置TIM5通道1 */

    __HAL_TIM_ENABLE_IT(&g_timx_cap_chy_handle, TIM_IT_UPDATE);         /* 使能更新中断 */
    HAL_TIM_IC_Start_IT(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY);     /* 开始捕获TIM5的通道1 */
}

/**
 * @brief       通用定时器输入捕获初始化接口
                HAL库调用的接口，用于配置不同的输入捕获
 * @param       htim:定时器句柄
 * @note        此函数会被HAL_TIM_IC_Init()调用
 * @retval      无
 */
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == GTIM_TIMX_CAP)                    /*输入通道捕获*/
    {
        GPIO_InitTypeDef gpio_init_struct;
        GTIM_TIMX_CAP_CHY_CLK_ENABLE();                     /* 使能TIMx时钟 */
        GTIM_TIMX_CAP_CHY_GPIO_CLK_ENABLE();                /* 开启捕获IO的时钟 */

        gpio_init_struct.Pin = GTIM_TIMX_CAP_CHY_GPIO_PIN;  /* 输入捕获的GPIO口 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLDOWN;              /* 下拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
        HAL_GPIO_Init(GTIM_TIMX_CAP_CHY_GPIO_PORT, &gpio_init_struct);

        HAL_NVIC_SetPriority(GTIM_TIMX_CAP_IRQn, 1, 3);     /* 抢占1，子优先级3 */
        HAL_NVIC_EnableIRQ(GTIM_TIMX_CAP_IRQn);             /* 开启ITMx中断 */
    }
}

/* 输入捕获状态(g_timxchy_cap_sta)
 * [7]  :0,没有成功的捕获;1,成功捕获到一次.
 * [6]  :0,还没捕获到高电平;1,已经捕获到高电平了.
 * [5:0]:捕获高电平后溢出的次数,最多溢出63次,所以最长捕获值 = 63*65536 + 65535 = 4194303
 *       注意:为了通用,我们默认ARR和CCRy都是16位寄存器,对于32位的定时器(如:TIM5),也只按16位使用
 *       按1us的计数频率,最长溢出时间为:4194303 us, 约4.19秒
 *
 *      (说明一下：正常32位定时器来说,1us计数器加1,溢出时间:4294秒)
 */
uint8_t g_timxchy_cap_sta = 0;    /* 输入捕获状态 */
uint16_t g_timxchy_cap_val = 0;   /* 输入捕获值 */


/**
 * @brief       定时器中断服务函数
 * @param       无
 * @retval      无
 */
void GTIM_TIMX_CAP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_timx_cap_chy_handle);  /* 定时器HAL库共用处理函数 */
}

/**
 * @brief       定时器输入捕获中断处理回调函数
 * @param       htim:定时器句柄指针
 * @note        该函数在HAL_TIM_IRQHandler中会被调用
 * @retval      无
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == GTIM_TIMX_CAP)
    {
        if ((g_timxchy_cap_sta & 0X80) == 0)                /* 还未成功捕获（第7位为0）（即上一次捕获还未完成） */
        {
            if (g_timxchy_cap_sta & 0X40)                   /* 已经捕获过上升沿了，这次捕获到的是一个下降沿（第6位为1） */
            {
                g_timxchy_cap_sta |= 0X80;                  /* 标记成功捕获到一次高电平脉宽 */
                g_timxchy_cap_val = HAL_TIM_ReadCapturedValue(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY);  /* 获取当前的捕获值 */
                TIM_RESET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY);                      /* 一定要先清除原来的设置 */
                TIM_SET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY, TIM_ICPOLARITY_RISING); /* 配置TIM5通道1上升沿捕获 */
            }
            else /* 还未开始,第一次捕获上升沿 */
            {
                g_timxchy_cap_sta = 0;                              /* 清空 */
                g_timxchy_cap_val = 0;
                g_timxchy_cap_sta |= 0X40;                          /* 标记捕获到了上升沿 */
                __HAL_TIM_SET_COUNTER(&g_timx_cap_chy_handle, 0);   /* 定时器5计数器清零 */
                TIM_RESET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY);   /* 一定要先清除原来的设置！！ */
                TIM_SET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY, TIM_ICPOLARITY_FALLING); /* 定时器5通道1设置为下降沿捕获 */
            }
        }
    }
}

/**
 * @brief       定时器更新中断回调函数
 * @param        htim:定时器句柄指针
 * @note        此函数会被定时器中断函数共同调用的
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == GTIM_TIMX_CAP)
    {
        if ((g_timxchy_cap_sta & 0X80) == 0)            /* 还未成功捕获 */
        {
            if (g_timxchy_cap_sta & 0X40)               /* 已经捕获到高电平了 */
            {
                if ((g_timxchy_cap_sta & 0X3F) == 0X3F) /* 高电平太长了，已经累计满了 */
                {
                    TIM_RESET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY);                     /* 一定要先清除原来的设置 */
                    TIM_SET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, GTIM_TIMX_CAP_CHY, TIM_ICPOLARITY_RISING);/* 配置TIM5通道1上升沿捕获 */
                    g_timxchy_cap_sta |= 0X80;          /* 标记成功捕获了一次 */
                    g_timxchy_cap_val = 0XFFFF;
                }
                else      /* 累计定时器溢出次数 */
                {
                    g_timxchy_cap_sta++;
                }
            }
        }
    }
}












