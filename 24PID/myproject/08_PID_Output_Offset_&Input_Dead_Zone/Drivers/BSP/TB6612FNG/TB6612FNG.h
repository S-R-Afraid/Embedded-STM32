#ifndef TB6612FNG_H
#define TB6612FNG_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/TIMER/gtim.h"


/* 引脚宏定义 */
#define TB6612_AN1_GPIO_PORT         GPIOE
#define TB6612_AN2_GPIO_PORT         GPIOB
#define TB6612_BN1_GPIO_PORT         
#define TB6612_BN2_GPIO_PORT         
#define TB6612_STBY_GPIO_PORT        GPIOE

#define TB6612_AN1_GPIO_PIN          GPIO_PIN_5
#define TB6612_AN2_GPIO_PIN          GPIO_PIN_5
#define TB6612_BN1_GPIO_PIN          
#define TB6612_BN2_GPIO_PIN          
#define TB6612_STBY_GPIO_PIN         GPIO_PIN_6

#define TB6612_AN1_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)
#define TB6612_AN2_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)
#define TB6612_BN1_GPIO_CLK_ENABLE() 
#define TB6612_BN2_GPIO_CLK_ENABLE() 
#define TB6612_STBY_GPIO_CLK_ENABLE() do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)


#define TB6612_PWMA_PORT        GTIM_TIMX_PWM_CHY_GPIO_PORT
#define TB6612_PWMA_PIN         GTIM_TIMX_PWM_CHY_GPIO_PIN      /* 引脚请在gtim.h里修改 */


/* 宏函数 */
#define TB6612_STBY_ENABLE()         do{HAL_GPIO_WritePin(TB6612_STBY_GPIO_PORT, TB6612_STBY_GPIO_PIN, GPIO_PIN_SET) ; }while(0)
#define TB6612_STBY_DISABLE()        do{HAL_GPIO_WritePin(TB6612_STBY_GPIO_PORT, TB6612_STBY_GPIO_PIN, GPIO_PIN_RESET) ; }while(0)
    /* A端正转 */
#define TB6612_AHL()                 do{ \
                                        HAL_GPIO_WritePin(TB6612_AN1_GPIO_PORT, TB6612_AN1_GPIO_PIN, GPIO_PIN_SET) ; \
                                        HAL_GPIO_WritePin(TB6612_AN2_GPIO_PORT, TB6612_AN2_GPIO_PIN, GPIO_PIN_RESET) ; \
                                     }while(0)
    /* A端反转 */
#define TB6612_ALH()                 do{ \
                                        HAL_GPIO_WritePin(TB6612_AN1_GPIO_PORT, TB6612_AN1_GPIO_PIN, GPIO_PIN_RESET) ; \
                                        HAL_GPIO_WritePin(TB6612_AN2_GPIO_PORT, TB6612_AN2_GPIO_PIN, GPIO_PIN_SET) ; \
                                     }while(0)
    /* A端翻转 */
#define TB6612_ATOGGLE()             do{ \
                                        HAL_GPIO_TogglePin(TB6612_AN1_GPIO_PORT, TB6612_AN1_GPIO_PIN) ; \
                                        HAL_GPIO_TogglePin(TB6612_AN2_GPIO_PORT, TB6612_AN2_GPIO_PIN) ; \
                                     }while(0)
    /* A端停止 */
#define TB6612_ASTOP()               do{ \
                                        HAL_GPIO_WritePin(TB6612_AN1_GPIO_PORT, TB6612_AN1_GPIO_PIN, GPIO_PIN_RESET) ; \
                                        HAL_GPIO_WritePin(TB6612_AN2_GPIO_PORT, TB6612_AN2_GPIO_PIN, GPIO_PIN_RESET) ; \
                                     }while(0)

    /* B端正转 */
#define TB6612_BHL()                 do{ \
                                        HAL_GPIO_WritePin(TB6612_BN1_GPIO_PORT, TB6612_BN1_GPIO_PIN, GPIO_PIN_SET) ; \
                                        HAL_GPIO_WritePin(TB6612_BN2_GPIO_PORT, TB6612_BN2_GPIO_PIN, GPIO_PIN_RESET) ; \
                                     }while(0)
    /* B端反转 */
#define TB6612_BLH()                 do{ \
                                        HAL_GPIO_WritePin(TB6612_BN1_GPIO_PORT, TB6612_BN1_GPIO_PIN, GPIO_PIN_RESET) ; \
                                        HAL_GPIO_WritePin(TB6612_BN2_GPIO_PORT, TB6612_BN2_GPIO_PIN, GPIO_PIN_SET) ; \
                                     }while(0)
    /* B端翻转 */
#define TB6612_BTOGGLE()             do{ \
                                        HAL_GPIO_TogglePin(TB6612_BN1_GPIO_PORT, TB6612_BN1_GPIO_PIN) ; \
                                        HAL_GPIO_TogglePin(TB6612_BN2_GPIO_PORT, TB6612_BN2_GPIO_PIN) ; \
                                     }while(0)
    /* B端停止 */
#define TB6612_BSTOP()               do{ \
                                        HAL_GPIO_WritePin(TB6612_BN1_GPIO_PORT, TB6612_BN1_GPIO_PIN, GPIO_PIN_RESET) ; \
                                        HAL_GPIO_WritePin(TB6612_BN2_GPIO_PORT, TB6612_BN2_GPIO_PIN, GPIO_PIN_RESET) ; \
                                     }while(0) /*  */


/* 函数声明 */

/*初始化函数，初始化PWM以及AN/BN*/
void TB6612_init();


/* 设置比较值，即设置转速 */
void TB6612_SET_A_VALUE(uint16_t val);

                                     /* 设置转速和方向，满值为2048，正数为正转， */










#endif


