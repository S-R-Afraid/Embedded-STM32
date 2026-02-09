/**
 ****************************************************************************************************
 * @file        atk_aht20.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-11-01
 * @brief       AHT20温湿度传感器 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 M48Z-M3最小系统板STM32F103版
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __ATK_AHT20_H
#define __ATK_AHT20_H



#include "E:\projects\STM32\MyProject\KaoQingSystem_Server_V3\Hardware\I2C\MyI2C.h"
#include "E:\projects\STM32\MyProject\KaoQingSystem_Server_V3\Hardware\Delay\delay.h"


/**************************************************************************************************/

#define ATK_AHT20_IIC_ADDR          0x70    /* AHT20上电默认IIC通讯地址 */

/* AHT20指令 */
#define INIT                        0xBE    /* 初始化指令 */
#define START_TEST                  0xAC    /* 开始测量指令 */

/**************************************************************************************************/

uint8_t atk_aht20_init(void);                                                   /* AHT20传感器初始化 */
uint8_t atk_aht20_read_status(void);                                            /* 读取AHT20状态 */
uint8_t atk_aht20_check(void);                                                  /* 检测AHT20是否就绪 */
void atk_aht20_read_data(float *temp, float *humi) ;                            /* 读取AHT20的温度和湿度数据 */
uint8_t atk_aht20_write_nbytes(uint8_t reg_addr, uint8_t *data, uint8_t len);   /* 写N个字节数据 */
uint8_t atk_aht20_read_nbytes(uint8_t *data, uint8_t len);                      /* 读N个字节数据 */

#endif





