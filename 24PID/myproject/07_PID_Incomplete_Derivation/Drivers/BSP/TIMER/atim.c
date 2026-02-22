#include "./BSP/TIMER/atim.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/Encoder/Encoder.h"
#include "./BSP/OLED/OLED.h"


/*高级定时器简介*/
/*
重复计数器（RCR）：8位有效（REP:[0:7]）计数器每次溢出都会使重复计数器减一，只有当重复计数器为0使发生溢出才会产生更新事件。
    即：每两次更新事件之间间隔RCR次溢出；或者每RCR+1次溢出才会产生一个更新事件。
    重复计数器有其影子寄存器，当产生更新事件（无论是软件硬件产生的）都会重置其数值。
想要使能atim的通道输出，必须设置BDTR寄存器的MOE位


*/




TIM_HandleTypeDef g_timx_npwm_chy_handle;     /* 定时器x句柄 */

/* g_npwm_remain表示当前还剩下多少个脉冲要发送
 * 每次最多发送256个脉冲
 */
static uint32_t g_npwm_remain = 0;

/**
 * @brief       高级定时器TIMX 通道Y 输出指定个数PWM 初始化函数
 * @note
 *              高级定时器的时钟来自APB2, 而PCLK2 = 72Mhz, 我们设置PPRE2不分频, 因此
 *              高级定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void atim_timx_npwm_chy_init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef gpio_init_struct;
    TIM_OC_InitTypeDef timx_oc_npwm_chy;   /* 定时器输出 */
    ATIM_TIMX_NPWM_CHY_GPIO_CLK_ENABLE();  /* TIMX 通道IO口时钟使能 */
    ATIM_TIMX_NPWM_CHY_CLK_ENABLE();       /* TIMX 时钟使能 */

    g_timx_npwm_chy_handle.Instance = ATIM_TIMX_NPWM;                  /* 定时器x */
    g_timx_npwm_chy_handle.Init.Prescaler = psc;                       /* 定时器分频 */
    g_timx_npwm_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;      /* 递增计数模式 */
    g_timx_npwm_chy_handle.Init.Period = arr;                          /* 自动重装载值 */
    g_timx_npwm_chy_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /*使能TIMx_ARR进行缓冲 */
    g_timx_npwm_chy_handle.Init.RepetitionCounter = 0;                 /* 重复计数器初始值 */
    HAL_TIM_PWM_Init(&g_timx_npwm_chy_handle);                         /* 初始化PWM */

    gpio_init_struct.Pin = ATIM_TIMX_NPWM_CHY_GPIO_PIN;                /* 通道y的CPIO口 */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                           /* 复用推完输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                               /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;                     /* 高速 */
    HAL_GPIO_Init(ATIM_TIMX_NPWM_CHY_GPIO_PORT, &gpio_init_struct);

    timx_oc_npwm_chy.OCMode = TIM_OCMODE_PWM1;                         /* 模式选择PWM 1*/
    timx_oc_npwm_chy.Pulse = arr / 2;                                  /* 设置比较值,此值用来确定占空比 */
                                                                       /* 这里默认设置比较值为自动重装载值的一半,即占空比为50% */
    timx_oc_npwm_chy.OCPolarity = TIM_OCPOLARITY_HIGH;                 /* 输出比较极性为高 */
    HAL_TIM_PWM_ConfigChannel(&g_timx_npwm_chy_handle, &timx_oc_npwm_chy, ATIM_TIMX_NPWM_CHY); /* 配置TIMx通道y */

    HAL_NVIC_SetPriority(ATIM_TIMX_NPWM_IRQn, 1, 3);                   /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ(ATIM_TIMX_NPWM_IRQn);                           /* 开启ITMx中断 */

    __HAL_TIM_ENABLE_IT(&g_timx_npwm_chy_handle, TIM_IT_UPDATE);       /* 允许更新中断 */
    HAL_TIM_PWM_Start(&g_timx_npwm_chy_handle, ATIM_TIMX_NPWM_CHY);    /* 开启对应PWM通道 */
}

/**
 * @brief       高级定时器TIMX NPWM设置PWM个数
 * @param       rcr: PWM的个数, 1~2^32次方个
 * @retval      无
 */
void atim_timx_npwm_chy_set(uint32_t npwm)
{
    if (npwm == 0) return;

    g_npwm_remain = npwm;                                                   /* 保存脉冲个数 */
    HAL_TIM_GenerateEvent(&g_timx_npwm_chy_handle, TIM_EVENTSOURCE_UPDATE); /* 产生一次更新事件,在中断里面处理脉冲输出 */
    __HAL_TIM_ENABLE(&g_timx_npwm_chy_handle);                              /* 使能定时器TIMX */
}

/**
 * @brief       高级定时器TIMX NPWM中断服务函数
 * @param       无
 * @retval      无
 */
//void ATIM_TIMX_NPWM_IRQHandler(void)
//{
//    uint16_t npwm = 0;

//    /* 以下代码没有使用定时器HAL库共用处理函数来处理，而是直接通过判断中断标志位的方式 */
//    if(__HAL_TIM_GET_FLAG(&g_timx_npwm_chy_handle, TIM_FLAG_UPDATE) != RESET)
//    {
//        if (g_npwm_remain >= 256)           /* 还有大于256个脉冲需要发送 */
//        {
//            g_npwm_remain = g_npwm_remain - 256;
//            npwm = 256;
//        }
//        else if (g_npwm_remain % 256)       /* 还有位数（不到256）个脉冲要发送 */
//        {
//            npwm = g_npwm_remain % 256;
//            g_npwm_remain = 0;              /* 没有脉冲了 */
//        }

//        if (npwm) /* 有脉冲要发送 */
//        {
//            ATIM_TIMX_NPWM->RCR = npwm - 1;                                         /* 设置重复计数寄存器值为npwm-1, 即npwm个脉冲 */
//            HAL_TIM_GenerateEvent(&g_timx_npwm_chy_handle, TIM_EVENTSOURCE_UPDATE); /* 产生一次更新事件,在中断里面处理脉冲输出 */
//            __HAL_TIM_ENABLE(&g_timx_npwm_chy_handle);                              /* 使能定时器TIMX */
//        }
//        else
//        {
//            ATIM_TIMX_NPWM->CR1 &= ~(1 << 0); /* 关闭定时器TIMX，使用HAL Disable会清除PWM通道信息，此处不用 */
//        }

//        __HAL_TIM_CLEAR_IT(&g_timx_npwm_chy_handle, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
//    }
//}


/*******************************以下是高级定时器输出比较模式实验程序**************************************/

TIM_HandleTypeDef g_timx_comp_pwm_handle;       /* 定时器x句柄 */

/**
 * @brief       高级定时器TIMX 输出比较模式 初始化函数（使用输出比较模式）
 * @note
 *              配置高级定时器TIMX 4路输出比较模式PWM输出,实现50%占空比,不同相位控制
 *              注意,本例程输出比较模式,每2个计数周期才能完成一个PWM输出,因此输出频率减半
 *              另外,我们还可以开启中断在中断里面修改CCRx,从而实现不同频率/不同相位的控制
 *              但是我们不推荐这么使用,因为这可能导致非常频繁的中断,从而占用大量CPU资源
 *
 *              高级定时器的时钟来自APB2, 而PCLK2 = 72Mhz, 我们设置PPRE2不分频, 因此
 *              高级定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void atim_timx_comp_pwm_init(uint16_t arr, uint16_t psc)
{
    TIM_OC_InitTypeDef timx_oc_comp_pwm = {0};

    g_timx_comp_pwm_handle.Instance = ATIM_TIMX_COMP;                   /* 定时器8 */
    g_timx_comp_pwm_handle.Init.Prescaler = psc  ;                      /* 定时器分频 */
    g_timx_comp_pwm_handle.Init.CounterMode = TIM_COUNTERMODE_UP;       /* 递增计数模式 */
    g_timx_comp_pwm_handle.Init.Period = arr;                           /* 自动重装载值 */
    g_timx_comp_pwm_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /* 使能影子寄存器TIMx_ARR */
    HAL_TIM_OC_Init(&g_timx_comp_pwm_handle);                           /* 输出比较模式初始化 */

    timx_oc_comp_pwm.OCMode = TIM_OCMODE_TOGGLE;                        /* 比较输出模式翻转功能 */
    timx_oc_comp_pwm.Pulse = 250 - 1;                                   /* 设置输出比较寄存器的值 */
    timx_oc_comp_pwm.OCPolarity = TIM_OCPOLARITY_HIGH;                  /* 输出比较极性为高 */
    HAL_TIM_OC_ConfigChannel(&g_timx_comp_pwm_handle, &timx_oc_comp_pwm, TIM_CHANNEL_1); /* 初始化定时器的输出比较通道1 */
    __HAL_TIM_ENABLE_OCxPRELOAD(&g_timx_comp_pwm_handle, TIM_CHANNEL_1);                 /* 通道1 预装载使能 */

    timx_oc_comp_pwm.Pulse = 500 - 1;
    HAL_TIM_OC_ConfigChannel(&g_timx_comp_pwm_handle, &timx_oc_comp_pwm, TIM_CHANNEL_2); /* 初始化定时器的输出比较通道2 */
    __HAL_TIM_ENABLE_OCxPRELOAD(&g_timx_comp_pwm_handle, TIM_CHANNEL_2);                 /* 通道2 预装载使能 */

    timx_oc_comp_pwm.Pulse = 750 - 1;
    HAL_TIM_OC_ConfigChannel(&g_timx_comp_pwm_handle, &timx_oc_comp_pwm, TIM_CHANNEL_3); /* 初始化定时器的输出比较通道3 */
    __HAL_TIM_ENABLE_OCxPRELOAD(&g_timx_comp_pwm_handle, TIM_CHANNEL_3);                 /* 通道3 预装载使能 */

    timx_oc_comp_pwm.Pulse = 1000 - 1;
    timx_oc_comp_pwm.OCIdleState = TIM_OCIDLESTATE_RESET;
    HAL_TIM_OC_ConfigChannel(&g_timx_comp_pwm_handle, &timx_oc_comp_pwm, TIM_CHANNEL_4); /* 初始化定时器的输出比较通道4 */
    __HAL_TIM_ENABLE_OCxPRELOAD(&g_timx_comp_pwm_handle, TIM_CHANNEL_4);                 /* 通道4 预装载使能 */

    HAL_TIM_OC_Start(&g_timx_comp_pwm_handle, TIM_CHANNEL_1);
    HAL_TIM_OC_Start(&g_timx_comp_pwm_handle, TIM_CHANNEL_2);
    HAL_TIM_OC_Start(&g_timx_comp_pwm_handle, TIM_CHANNEL_3);
    HAL_TIM_OC_Start(&g_timx_comp_pwm_handle, TIM_CHANNEL_4);
}

/**
 * @brief       定时器底层驱动，时钟使能，引脚配置
                此函数会被HAL_TIM_OC_Init()调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == ATIM_TIMX_COMP)
    {
        GPIO_InitTypeDef gpio_init_struct;

        ATIM_TIMX_COMP_CLK_ENABLE();

        ATIM_TIMX_COMP_CH1_GPIO_CLK_ENABLE();
        ATIM_TIMX_COMP_CH2_GPIO_CLK_ENABLE();
        ATIM_TIMX_COMP_CH3_GPIO_CLK_ENABLE();
        ATIM_TIMX_COMP_CH4_GPIO_CLK_ENABLE();

        gpio_init_struct.Pin = ATIM_TIMX_COMP_CH1_GPIO_PIN;
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;
        gpio_init_struct.Pull = GPIO_NOPULL;
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(ATIM_TIMX_COMP_CH1_GPIO_PORT, &gpio_init_struct);

        gpio_init_struct.Pin = ATIM_TIMX_COMP_CH2_GPIO_PIN;
        HAL_GPIO_Init(ATIM_TIMX_COMP_CH2_GPIO_PORT, &gpio_init_struct);

        gpio_init_struct.Pin = ATIM_TIMX_COMP_CH3_GPIO_PIN;
        HAL_GPIO_Init(ATIM_TIMX_COMP_CH3_GPIO_PORT, &gpio_init_struct);

        gpio_init_struct.Pin = ATIM_TIMX_COMP_CH4_GPIO_PIN;
        HAL_GPIO_Init(ATIM_TIMX_COMP_CH4_GPIO_PORT, &gpio_init_struct);
    }
}


/*******************************以下是高级定时器互补输出带死区控制实验程序**************************************/

TIM_HandleTypeDef g_timx_cplm_pwm_handle;                              /* 定时器x句柄 */
TIM_BreakDeadTimeConfigTypeDef g_sbreak_dead_time_config = {0};        /* 死区时间设置 */

/**
 * @brief       高级定时器TIMX 互补输出 初始化函数（使用PWM模式1）
 * @note
 *              配置高级定时器TIMX 互补输出, 一路OCy 一路OCyN, 并且可以设置死区时间
 *
 *              高级定时器的时钟来自APB2, 而PCLK2 = 72Mhz, 我们设置PPRE2不分频, 因此
 *              高级定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值。
 * @param       psc: 时钟预分频数
 * @retval      无
 */

void atim_timx_cplm_pwm_init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    TIM_OC_InitTypeDef tim_oc_cplm_pwm = {0};

    ATIM_TIMX_CPLM_CLK_ENABLE();            /* TIMx 时钟使能 */
    ATIM_TIMX_CPLM_CHY_GPIO_CLK_ENABLE();   /* 通道X对应IO口时钟使能 */
    ATIM_TIMX_CPLM_CHYN_GPIO_CLK_ENABLE();  /* 通道X互补通道对应IO口时钟使能 */
    ATIM_TIMX_CPLM_BKIN_GPIO_CLK_ENABLE();  /* 通道X刹车输入对应IO口时钟使能 */
  
    gpio_init_struct.Pin = ATIM_TIMX_CPLM_CHY_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP; 
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH ;
    HAL_GPIO_Init(ATIM_TIMX_CPLM_CHY_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = ATIM_TIMX_CPLM_CHYN_GPIO_PIN;
    HAL_GPIO_Init(ATIM_TIMX_CPLM_CHYN_GPIO_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = ATIM_TIMX_CPLM_BKIN_GPIO_PIN;
    HAL_GPIO_Init(ATIM_TIMX_CPLM_BKIN_GPIO_PORT, &gpio_init_struct);
    
    ATIM_TIMX_CPLM_CHYN_GPIO_REMAP();     /* 映射定时器IO,PE不是本例程所用定时器的默认IO，需要复用 */

    g_timx_cplm_pwm_handle.Instance = ATIM_TIMX_CPLM;                       /* 定时器x */
    g_timx_cplm_pwm_handle.Init.Prescaler = psc;                            /* 定时器预分频系数 */
    g_timx_cplm_pwm_handle.Init.CounterMode = TIM_COUNTERMODE_UP;           /* 递增计数模式 */
    g_timx_cplm_pwm_handle.Init.Period = arr;                               /* 自动重装载值 */
    g_timx_cplm_pwm_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;     /* CKD[1:0] = 10, tDTS = 4 * tCK_INT = Ft / 4 = 18Mhz */
    g_timx_cplm_pwm_handle.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /* 使能影子寄存器TIMx_ARR */
    HAL_TIM_PWM_Init(&g_timx_cplm_pwm_handle);

    tim_oc_cplm_pwm.OCMode = TIM_OCMODE_PWM1;                               /* PWM模式1 */
    tim_oc_cplm_pwm.OCPolarity = TIM_OCPOLARITY_LOW;                        /* OCy 低电平有效 */
    tim_oc_cplm_pwm.OCNPolarity = TIM_OCNPOLARITY_LOW;                      /* OCyN 低电平有效 */
    tim_oc_cplm_pwm.OCIdleState = TIM_OCIDLESTATE_SET;                      /* 当MOE=0，OCx=1 */
    tim_oc_cplm_pwm.OCNIdleState = TIM_OCNIDLESTATE_SET;                    /* 当MOE=0，OCxN=1 */
    HAL_TIM_PWM_ConfigChannel(&g_timx_cplm_pwm_handle, &tim_oc_cplm_pwm, ATIM_TIMX_CPLM_CHY);

    /* 设置死区参数，开启死区中断 */
    g_sbreak_dead_time_config.OffStateRunMode = TIM_OSSR_DISABLE;           /* 运行模式的关闭输出状态 */
    g_sbreak_dead_time_config.OffStateIDLEMode = TIM_OSSI_DISABLE;          /* 空闲模式的关闭输出状态 */
    g_sbreak_dead_time_config.LockLevel = TIM_LOCKLEVEL_OFF;                /* 不用寄存器锁功能 */
    g_sbreak_dead_time_config.BreakState = TIM_BREAK_ENABLE;                /* 使能刹车输入 */
    g_sbreak_dead_time_config.BreakPolarity = TIM_BREAKPOLARITY_HIGH;       /* 刹车输入有效信号极性为高 */
    g_sbreak_dead_time_config.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE; /* 使能AOE位，允许刹车结束后自动恢复输出 */
    HAL_TIMEx_ConfigBreakDeadTime(&g_timx_cplm_pwm_handle, &g_sbreak_dead_time_config);

    HAL_TIM_PWM_Start(&g_timx_cplm_pwm_handle, ATIM_TIMX_CPLM_CHY);         /* 使能OCy输出 */
    HAL_TIMEx_PWMN_Start(&g_timx_cplm_pwm_handle, ATIM_TIMX_CPLM_CHY);      /* 使能OCyN输出 */
}

/**
 * @brief       定时器TIMX 设置输出比较值 & 死区时间
 * @param       ccr: 输出比较值
 * @param       dtg: 死区时间
 *   @arg       dtg[7:5]=0xx时, 死区时间 = dtg[7:0] * tDTS
 *   @arg       dtg[7:5]=10x时, 死区时间 = (64 + dtg[6:0]) * 2  * tDTS
 *   @arg       dtg[7:5]=110时, 死区时间 = (32 + dtg[5:0]) * 8  * tDTS
 *   @arg       dtg[7:5]=111时, 死区时间 = (32 + dtg[5:0]) * 16 * tDTS
 *   @note      tDTS = 1 / (Ft /  CKD[1:0]) = 1 / 18M = 55.56ns
 * @retval      无
 */
void atim_timx_cplm_pwm_set(uint16_t ccr, uint8_t dtg)
{
    g_sbreak_dead_time_config.DeadTime = dtg;       /* 死区时间设置 */
    HAL_TIMEx_ConfigBreakDeadTime(&g_timx_cplm_pwm_handle, &g_sbreak_dead_time_config);  /* 重设死区时间 */
    __HAL_TIM_MOE_ENABLE(&g_timx_cplm_pwm_handle);  /* MOE=1,使能主输出 */
    ATIM_TIMX_CPLM_CHY_CCRY = ccr;                  /* 设置比较寄存器 */
}


/*******************************以下是高级定时器PWM输入模式程序**************************************/

TIM_HandleTypeDef g_timx_pwmin_chy_handle;   /* 定时器x句柄 */

/* PWM输入状态(g_timxchy_cap_sta)
 * 0,没有成功捕获.
 * 1,已经成功捕获了
 */
uint8_t g_timxchy_pwmin_sta  = 0;   /* PWM输入状态 */
uint16_t g_timxchy_pwmin_psc  = 0;  /* PWM输入分频系数 */
uint32_t g_timxchy_pwmin_hval = 0;  /* PWM的高电平脉宽 */
uint32_t g_timxchy_pwmin_cval = 0;  /* PWM的周期宽度 */

/**
 * @brief       定时器TIMX 通道Y PWM输入模式 初始化函数
 * @note
 *              高级定时器的时钟来自APB2, 而PCLK2 = 72Mhz, 我们设置PPRE2不分频, 因此
 *              高级定时器时钟 = 72Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 *              本函数初始化的时候: 使用psc=0, arr固定为65535. 得到采样时钟频率为72Mhz,精度约13.8ns
 *
 * @param       无
 * @retval      无
 */
void atim_timx_pwmin_chy_init(void)
{
    GPIO_InitTypeDef gpio_init_struct = {0};
    TIM_SlaveConfigTypeDef slave_config = {0};
    TIM_IC_InitTypeDef tim_ic_pwmin_chy = {0};

    ATIM_TIMX_PWMIN_CHY_CLK_ENABLE();
    ATIM_TIMX_PWMIN_CHY_GPIO_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    AFIO_REMAP_PARTIAL(AFIO_EVCR_PORT_PC, AFIO_EVCR_PIN_PX6);       /* 复用TIM8_CH1/PC6 */

    gpio_init_struct.Pin = ATIM_TIMX_PWMIN_CHY_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP; 
    gpio_init_struct.Pull = GPIO_PULLDOWN;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH ;
    HAL_GPIO_Init(ATIM_TIMX_PWMIN_CHY_GPIO_PORT, &gpio_init_struct);

    g_timx_pwmin_chy_handle.Instance = ATIM_TIMX_PWMIN;             /* 定时器8 */
    g_timx_pwmin_chy_handle.Init.Prescaler = 0;                     /* 定时器预分频系数 */
    g_timx_pwmin_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;  /* 递增计数模式 */
    g_timx_pwmin_chy_handle.Init.Period = 65535;                    /* 自动重装载值 */
    HAL_TIM_IC_Init(&g_timx_pwmin_chy_handle);
    
    /* 从模式配置，IT1触发更新 */
    slave_config.SlaveMode = TIM_SLAVEMODE_RESET;                   /* 从模式：复位模式 */
    slave_config.InputTrigger = TIM_TS_TI1FP1;                      /* 定时器输入触发源：TI1FP1 */
    slave_config.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING; /* 上升沿检测 */
    slave_config.TriggerFilter = 0;                                 /* 不滤波 */
    HAL_TIM_SlaveConfigSynchro(&g_timx_pwmin_chy_handle, &slave_config);

    /* IC1捕获：上升沿触发TI1FP1 */
    tim_ic_pwmin_chy.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING; /* 上升沿检测 */
    tim_ic_pwmin_chy.ICSelection = TIM_ICSELECTION_DIRECTTI;       /* 选择输入端IC1映射到TI1 */
    tim_ic_pwmin_chy.ICPrescaler = TIM_ICPSC_DIV1;                 /* 不分频 */
    tim_ic_pwmin_chy.ICFilter = 0;                                 /* 不滤波 */
    HAL_TIM_IC_ConfigChannel( &g_timx_pwmin_chy_handle, &tim_ic_pwmin_chy, TIM_CHANNEL_1 );
    
    /* IC2捕获：上升沿触发TI1FP2 */
    tim_ic_pwmin_chy.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING; /* 下降沿检测 */
    tim_ic_pwmin_chy.ICSelection = TIM_ICSELECTION_INDIRECTTI;      /* 选择输入端IC2映射到TI1 */
    HAL_TIM_IC_ConfigChannel(&g_timx_pwmin_chy_handle,&tim_ic_pwmin_chy,TIM_CHANNEL_2);
    
    HAL_NVIC_SetPriority(ATIM_TIMX_PWMIN_IRQn, 1, 3);               /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ( ATIM_TIMX_PWMIN_IRQn );                     /* 开启TIMx中断 */
    
    /* TIM1/TIM8有独立的输入捕获中断服务函数 */
    if ( ATIM_TIMX_PWMIN == TIM1 || ATIM_TIMX_PWMIN == TIM8)
    {
        HAL_NVIC_SetPriority(ATIM_TIMX_PWMIN_CC_IRQn, 1, 3);        /* 设置中断优先级，抢占优先级1，子优先级3 */
        HAL_NVIC_EnableIRQ(ATIM_TIMX_PWMIN_CC_IRQn);                /* 开启TIMx中断 */
    }

    __HAL_TIM_ENABLE_IT(&g_timx_pwmin_chy_handle, TIM_IT_UPDATE);
    HAL_TIM_IC_Start_IT(&g_timx_pwmin_chy_handle, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&g_timx_pwmin_chy_handle, TIM_CHANNEL_2);
}

/**
 * @brief       定时器TIMX PWM输入模式 重新启动捕获
 * @param       无
 * @retval      无
 */
void atim_timx_pwmin_chy_restart(void)
{
    sys_intx_disable();                                             /* 关闭中断 */

    g_timxchy_pwmin_sta = 0;                                        /* 清零状态,重新开始检测 */
    g_timxchy_pwmin_psc = 0;                                        /* 分频系数清零 */

    __HAL_TIM_SET_PRESCALER(&g_timx_pwmin_chy_handle, 0);           /* 以最大的计数频率采集,以得到最好的精度 */
    __HAL_TIM_SET_COUNTER(&g_timx_pwmin_chy_handle, 0);             /* 计数器清零 */
    
    __HAL_TIM_ENABLE_IT(&g_timx_pwmin_chy_handle, TIM_IT_CC1);      /* 使能通道1捕获中断 */
    __HAL_TIM_ENABLE_IT(&g_timx_pwmin_chy_handle, TIM_IT_UPDATE);   /* 使能溢出中断 */
    __HAL_TIM_ENABLE(&g_timx_pwmin_chy_handle);                     /* 使能定时器TIMX */

    ATIM_TIMX_PWMIN->SR = 0;                                        /* 清除所有中断标志位 */

    sys_intx_enable();                                              /* 打开中断 */
}

/**
 * @brief       定时器TIMX 通道Y PWM输入模式 中断处理函数
 * @note
 *              因为TIM1/TIM8等有多个中断服务函数,而TIM2~5/TIM12/TIM15等普通定时器只有1个中断服务
 *              函数,为了更好的兼容,我们对中断处理统一放到atim_timx_pwin_chy_process函数里面进行处理
 *
 * @param       无
 * @retval      无
 */
static void atim_timx_pwmin_chy_process(void)
{
    static uint8_t sflag = 0;               /* 启动PWMIN输入检测标志 */

    if (g_timxchy_pwmin_sta)
    {
        g_timxchy_pwmin_psc = 0;
        ATIM_TIMX_PWMIN->SR = 0;                                        /* 清除所有中断标志位 */
        __HAL_TIM_SET_COUNTER(&g_timx_pwmin_chy_handle, 0);             /* 计数器清零 */
        return ;
    }

    if (__HAL_TIM_GET_FLAG(&g_timx_pwmin_chy_handle, TIM_FLAG_UPDATE))         /* 发生了溢出中断 */
    { 
        __HAL_TIM_CLEAR_FLAG(&g_timx_pwmin_chy_handle, TIM_FLAG_UPDATE);       /* 清除溢出中断标记 */

        if (__HAL_TIM_GET_FLAG(&g_timx_pwmin_chy_handle, TIM_FLAG_CC1) == 0)   /* 没有发生周期捕获中断,且捕获未完成 */
        {
            sflag = 0;
            if (g_timxchy_pwmin_psc == 0)   /* 从0 到 1 */
            {
                g_timxchy_pwmin_psc ++;
            }
            else
            {
                if (g_timxchy_pwmin_psc == 65535)               /* 已经最大了,可能是无输入状态 */
                {
                    g_timxchy_pwmin_psc = 0;                    /* 重新恢复不分频 */
                }
                else if (g_timxchy_pwmin_psc > 32767)           /* 不能倍增了 */
                {
                    g_timxchy_pwmin_psc = 65535;                /* 直接等于最大分频系数 */
                }
                else
                {
                    g_timxchy_pwmin_psc += g_timxchy_pwmin_psc; /* 倍增 */
                }
            }

            __HAL_TIM_SET_PRESCALER(&g_timx_pwmin_chy_handle, g_timxchy_pwmin_psc); /* 设置定时器预分频系数 */
            __HAL_TIM_SET_COUNTER(&g_timx_pwmin_chy_handle, 0);                     /* 计数器清零 */
            ATIM_TIMX_PWMIN->SR = 0;                                                /* 清除所有中断标志位 */
            return ;
        }
    }

    if (sflag == 0)   /* 第一次采集到捕获中断 */
    {
        if (__HAL_TIM_GET_FLAG(&g_timx_pwmin_chy_handle, TIM_FLAG_CC1))         /* 检测到了第一次周期捕获中断 */
        { 
            sflag = 1;  /* 标记第一次周期已经捕获, 第二次周期捕获可以开始了 */
        }
        
        ATIM_TIMX_PWMIN->SR = 0;    /* 清除所有中断标志位 */
        return ;                    /* 完成此次操作 */
    }

    if (g_timxchy_pwmin_sta == 0)   /* 还没有成功捕获 */
    {
        if (__HAL_TIM_GET_FLAG(&g_timx_pwmin_chy_handle, TIM_FLAG_CC1))         /* 检测到了周期捕获中断 */
        {
            g_timxchy_pwmin_hval = HAL_TIM_ReadCapturedValue(&g_timx_pwmin_chy_handle, TIM_CHANNEL_2) + 1; /* 高定平脉宽捕获值 */
            g_timxchy_pwmin_cval = HAL_TIM_ReadCapturedValue(&g_timx_pwmin_chy_handle, TIM_CHANNEL_1) + 1; /* 周期捕获值 */

            if (g_timxchy_pwmin_hval < g_timxchy_pwmin_cval)    /* 高电平脉宽必定小于周期长度 */
            {
                g_timxchy_pwmin_sta = 1;                        /* 标记捕获成功 */

                g_timxchy_pwmin_psc = ATIM_TIMX_PWMIN->PSC;     /* 获取PWM输入分频系数 */
                
                if (g_timxchy_pwmin_psc == 0)                   /* 分频系数为0的时候, 修正读取数据 */
                {
                    g_timxchy_pwmin_hval++;                     /* 修正系数为1, 加1 */
                    g_timxchy_pwmin_cval++;                     /* 修正系数为1, 加1 */
                }

                sflag = 0;
                /* 每次捕获PWM输入成功后, 停止捕获,避免频繁中断影响系统正常代码运行 */
                ATIM_TIMX_PWMIN->CR1  &= ~(1 << 0);                             /* 关闭定时器TIMX */
                __HAL_TIM_DISABLE_IT(&g_timx_pwmin_chy_handle, TIM_IT_CC1);     /* 关闭通道1捕获中断 */
                __HAL_TIM_DISABLE_IT(&g_timx_pwmin_chy_handle, TIM_IT_CC2);     /* 关闭通道2捕获中断 */
                __HAL_TIM_DISABLE_IT(&g_timx_pwmin_chy_handle, TIM_IT_UPDATE);  /* 关闭溢出中断 */

                ATIM_TIMX_PWMIN->SR = 0;                                        /* 清除所有中断标志位 */
            }
            else
            {
                atim_timx_pwmin_chy_restart();
            }
        }
    }
    
    ATIM_TIMX_PWMIN->SR = 0;                                                    /* 清除所有中断标志位 */
}

/**
 * @brief       定时器TIMX 更新/溢出 中断服务函数
 *   @note      TIM1/TIM8的这个函数仅用于更新/溢出中断服务,捕获在另外一个函数!
 *              其他普通定时器则更新/溢出/捕获,都在这个函数里面处理!
 * @param       无
 * @retval      无
 */
//void ATIM_TIMX_PWMIN_IRQHandler(void)
//{
//    atim_timx_pwmin_chy_process();
//}

/**
 * @brief       定时器TIMX 输入捕获 中断服务函数
 *   @note      仅TIM1/TIM8有这个函数,其他普通定时器没有这个中断服务函数!
 * @param       无
 * @retval      无
 */
void ATIM_TIMX_PWMIN_CC_IRQHandler(void)
{
    atim_timx_pwmin_chy_process();
}




;
TIM_HandleTypeDef g_tim1_handle;     /* 定时器1句柄 */

void Timer_Init(){
    LED_OFF();
    __HAL_RCC_TIM1_CLK_ENABLE();                                 /* 使能TIMx时钟 */

    g_tim1_handle.Instance = TIM1;                              /* 通用定时器x */
    g_tim1_handle.Init.Prescaler = 72-1;                         /* 预分频系数 */
    g_tim1_handle.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
    g_tim1_handle.Init.Period = 1000-1;                            /* 自动装载值 */
    HAL_TIM_Base_Init(&g_tim1_handle);

    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 1, 2);                      /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);                              /* 开启ITMx中断 */

    HAL_TIM_Base_Start_IT(&g_tim1_handle);                      /* 使能定时器x和定时器x更新中断 */
}







