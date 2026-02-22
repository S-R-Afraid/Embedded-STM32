
#include "./BSP/WDG/wdg.h"

/*
独立看门狗的时钟源来自LSI，即RC振荡器，即便系统时钟故障也仍然有效，主要用于检测硬件故障
    各个寄存器的作用通过查看寄存器版本的初始化函数可知；
*/

IWDG_HandleTypeDef g_iwdg_handle;  /* 独立看门狗句柄 */

/**
 * @brief       初始化独立看门狗 
 * @param       prer: 分频系数，IWDG_PRESCALER_4、8、16、……256,对应4~256分频因子（stm32f1xx_hal_iwdg.h第80行）
                这些宏实际上是一个三位二进制数，用于预分频器寄存器(IWDG_PR)的0~2位
 *   @arg       分频因子 = 4 * 2^prer. 但最大值只能是256!
 * @param       rlr: 自动重装载值,0~0XFFF(0~4095). 用于重装载寄存器(IWDG_RLR)的0~11位
 * @note        时间计算(大概):Tout=((4 * 2^prer) * rlr) / 40 (ms). 即分频因子*重载值 / 40，这里的40是看门狗的时钟源频率(F1,若是F4、F7、H7则是32)
                解释：时钟频率/分频因子得到看门狗实际工作频率，重装载/工作频率=溢出时间
 * @retval      无
 */
void iwdg_init(uint8_t prer, uint16_t rlr)
{
    g_iwdg_handle.Instance = IWDG;       /* IDWG 是stm32f103xe.h里定义的地址宏，指向独立看门狗的基地址((IWDG_TypeDef *)IWDG_BASE)
                                                顺便一提，它上面就是WWDG*/
    g_iwdg_handle.Init.Prescaler = prer; /* 设置IWDG分频系数 */
    g_iwdg_handle.Init.Reload = rlr;     /* 重装载值 但是由于最大值寄存器（重载寄存器）为12位，所以重装载值不能超过4095*/
    /*
     如果是F7、H7的话，还有第三个参数Window(窗口寄存器)，只有计数值低于该寄存器值之间才可喂狗
    */
    HAL_IWDG_Init(&g_iwdg_handle);       /* 初始化IWDG并启动 */
}

/**
 * @brief       初始化独立看门狗 ,根据溢出时间自动指定分频系数和重装载值
 * @param       t：溢出时间(ms)，最小是0.1最大为26214（40KHz时钟频率下）
*/
void my_iwdg_init(double t)
{
    uint8_t prer=0;
    uint16_t rlr=t*40/4/pow(2,prer);
    while(rlr>=0XFFF){
        prer++;
        rlr=t*40/4/pow(2,prer);
    }
    g_iwdg_handle.Instance = IWDG;
    g_iwdg_handle.Init.Prescaler = prer;
    g_iwdg_handle.Init.Reload = rlr;
    HAL_IWDG_Init(&g_iwdg_handle);
}

/**
 * @brief       喂独立看门狗
 * @param       无
 * @retval      无
 */
void iwdg_feed(void)
{
    HAL_IWDG_Refresh(&g_iwdg_handle);  /* 重装载计数器 */
}


/*--------------寄存器版本-------------*/

void iwdg_init_jcq(uint8_t prer, uint16_t rlr)
{
    IWDG->KR = 0X5555;  /* 使能对IWDG->PR和IWDG->RLR的写 */
    IWDG->PR = prer;    /* 设置分频系数 */
    IWDG->RLR = rlr;    /* 从加载寄存器 IWDG->RLR */
    IWDG->KR = 0XAAAA;  /* reload */
    IWDG->KR = 0XCCCC;  /* 使能看门狗 */
    /*
    另注：状态寄存器（IWDG_SR）：
        位0(PVU):预分频值更新，此位为1表示预分频值正在更新中，0表示更新完毕；
        位1(Rvu):重装载值更新，其余同上
    这两位由硬件自动更改，只需读取判断即可
    */
}


void iwdg_feed_jcq(void)
{
    IWDG->KR = 0XAAAA;  /* reload */
}

/************************以下是窗口看门狗代码********************************/
/*
窗口看门狗由从APB1时钟分频后得到的时钟驱动，通过可配置的时间窗口来检测应用程序非正常的过迟或过早的操作。

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

相比IWDG,WWDG有两个额外的7位寄存器用来储存计数器值(WWDG_CR[6:0])和窗口值(WWDG_CFR[6:0])
    当计数器值小于0x40（即第6位为0）时，将产生复位；因此，WWDG真正用于递减的只有0~5位；
    计数器自由运行，即便看门狗未启动也会递减，所以启动WWDG时一定要确保第6位为1；
    当计数器值大于窗口值时被重载，将产生复位；
        如果启动了看门狗并且允许中断，当递减计数器等于0x40时产生早期唤醒中断(EWI)，它可
        以被用于重装载计数器以避免WWDG复位。这个特性可用于自动喂狗。

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

WWDG_CR或WWDG_CFR与0x7F做与运算即可读取当前计数器值和窗口值；

WWDG_CR与0x80与运算(即第7位)可得知WWDG是否被激活（0未激活1激活）；
    另外，这一位可由软件置1以开启WWDG，但无法关闭WWDG；

WWDG_CFR与0x180与运算(7、8位)可得知预分频器的分频系数(2^运算结果)；

WWDG_CFR与0x200与运算(第9位)可得知是否允许产生EWI中断；

WWDG_SR(状态寄存器)位0为提前唤醒中断标志(EWIF)，当计数器值为0x40时由硬件自动置1，可由程序置0；
    即便未使能中断，此位在计时结束后也会被置1；
    
- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

*/

WWDG_HandleTypeDef g_wwdg_handle;  /* WWDG句柄 */

/**
 * @brief       初始化窗口看门狗
 * @param       tr: T[6:0],计数器值
 * @param       tw: W[6:0],窗口值
 * @note        fprer:分频系数（WDGTB）,范围:WWDG_PRESCALER_1~WWDG_PRESCALER_8,表示2^WDGTB分频
                    数值上分别为0、1、2、3；
 *              Fwwdg=PCLK1/(4096*2^fprer). 一般PCLK1=36Mhz；其中4096为固定分频系数
                超时时间计算：(4096*2^fprer)*(tr-0X3F)/PCLK1；
                最大-最小超时时间参考 RM0008或者《STM32F10xxx参考手册》P321
                    F103最大58.25ms，最小113μs
 * @retval      无
 */
void wwdg_init(uint8_t tr, uint8_t wr, uint32_t fprer)
{
    g_wwdg_handle.Instance = WWDG;
    g_wwdg_handle.Init.Prescaler = fprer;         /* 设置分频系数 */
    g_wwdg_handle.Init.Window = wr;               /* 设置窗口值 */
    g_wwdg_handle.Init.Counter = tr;              /* 设置计数器值 */
    g_wwdg_handle.Init.EWIMode = WWDG_EWI_ENABLE; /* 使能窗口看门狗提前唤醒中断 */
    HAL_WWDG_Init(&g_wwdg_handle);                /* 初始化WWDG */
}

/**
 * @brief       WWDG MSP回调
 * @param       WWDG句柄
 * @note        此函数会被HAL_WWDG_Init()调用
 * @retval      无
 */
void HAL_WWDG_MspInit(WWDG_HandleTypeDef *hwwdg)
{
    __HAL_RCC_WWDG_CLK_ENABLE();            /* 使能窗口看门狗时钟 */

    HAL_NVIC_SetPriority(WWDG_IRQn, 2, 3);  /* 抢占优先级2，子优先级为3 */
    HAL_NVIC_EnableIRQ(WWDG_IRQn);          /* 使能窗口看门狗中断 */
}

/**
 * @brief       窗口看门狗中断服务程序
 * @param       无
 * @retval      无
 */
void WWDG_IRQHandler(void)
{
    HAL_WWDG_IRQHandler(&g_wwdg_handle);  /* 调用WWDG共用中断处理函数 */
}

/**
 * @brief       窗口看门狗喂狗提醒中断服务回调函数
 * @param       wwdg句柄
 * @note        此函数会被HAL_WWDG_IRQHandler()调用，如果你想要在这函数里做一些其它操作的话，最好把喂狗函数放在第一步
 * @retval      无
 */
void HAL_WWDG_EarlyWakeupCallback(WWDG_HandleTypeDef *hwwdg)
{
    HAL_WWDG_Refresh(&g_wwdg_handle); /* 更新窗口看门狗值 */
    LED1(0);                    /* LED1闪烁 */
}
//-------------寄存器版本---------------------------//

/* 保存WWDG计数器的设置值,默认为最大. */
uint8_t g_wwdg_cnt = 0x7f;//7f即为7个1

/**
 * @brief       初始化窗口看门狗
 * @param       tr: T[6:0],计数器值
 * @param       tw: W[6:0],窗口值
 * @note        fprer:分频系数（WDGTB）,范围:0~7,表示2^WDGTB分频
 *              Fwwdg=PCLK1/(4096*2^fprer). 一般PCLK1=36Mhz
 *              溢出时间=(4096*2^fprer)*(tr-0X3F)/PCLK3
 *              假设fprer=4,tr=7f,PCLK1=36Mhz
 *              则溢出时间=4096*16*64/36Mhz=116.50ms
 * @retval      无
 */
void wwdg_init_jcq(uint8_t tr, uint8_t wr, uint8_t fprer)
{
    RCC->APB1ENR |= 1 << 11;        /* 使能wwdg时钟 */
    g_wwdg_cnt = tr & g_wwdg_cnt;   /* 初始化WWDG_CNT. 与7f做与运算是为了确保结果只有7位避免影响*/
    WWDG->CFR |= fprer << 7;        /* PCLK1/4096再除2^fprer 左移7位，使fprer的值处于7、8位上*/
    WWDG->CFR &= 0XFF80;            /* 清除0~6位*/
    WWDG->CFR |= wr;                /* 设定窗口值 */
    WWDG->CR |= g_wwdg_cnt;         /* 设定计数器值 */
    WWDG->CR |= 1 << 7;             /* 开启看门狗 */
//    sys_nvic_init(2, 3, WWDG_IRQn, 2); /* 抢占2，子优先级3，组2 */
    WWDG->SR = 0X00;                /* 清除提前唤醒中断标志位 */
    WWDG->CFR |= 1 << 9;            /* 使能提前唤醒中断 */
}

/**
 * @brief       重设置WWDG计数器的值
 * @param       无
 * @retval      无
 */
void wwdg_set_counter_jcq(uint8_t cnt)
{
    WWDG->CR = (cnt & 0x7F);   /* 重设置7位计数器 */
}

/**
 * @brief       窗口看门狗中断服务程序(寄存器版本)
 * @param       无
 * @retval      无
 */
//void WWDG_IRQHandler(void)
//{  
//    if (WWDG->SR & 0X01)                /* 先判断是否发生了WWDG提前唤醒中断 */
//    {
//        WWDG->SR = 0X00;                /* 清除提前唤醒中断标志位 */
//        wwdg_set_counter_jcq(g_wwdg_cnt);   /* 重设窗口看门狗的值! */
//        LED1_TOGGLE();                  /* 绿灯闪烁 */
//    }
//}



