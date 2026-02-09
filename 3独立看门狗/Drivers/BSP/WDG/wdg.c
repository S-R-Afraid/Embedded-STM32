
#include "./BSP/WDG/wdg.h"


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


