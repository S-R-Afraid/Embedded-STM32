/**
 ****************************************************************************************************
 * @file        atk_aht20.c
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

#include "atk_aht20.h"


/**
 * @brief       初始化AHT20传感器
 * @param       无
 * @retval      初始化结果，0：成功，1：失败
 */
uint8_t atk_aht20_init(void)
{
    uint8_t command[2] = {0x08, 0x00};

    iic_init();                                     /* 初始化IIC接口 */
    
    delay_ms(40);                                   /* 延时40ms，等硬件稳定 */

    atk_aht20_write_nbytes(INIT, command, 2);       /* 发送初始化指令 */

    delay_ms(500);                                  /* 延时500ms，等硬件稳定 */
    
    return atk_aht20_check();                       /* 检测AHT20的状态 */
}

/**
 * @brief       读取AHT20的状态寄存器
 * @param       无
 * @retval      状态值
 */
uint8_t atk_aht20_read_status(void)
{
    uint8_t ret;
    
    atk_aht20_read_nbytes(&ret, 1);                 /* 读取状态寄存器 */

    return ret;
}

/**
 * @brief       检测AHT20传感器是否就绪
 * @param       无
 * @retval      0, 正常
 *              1, 异常/不存在
 */
uint8_t atk_aht20_check(void)
{
    if((atk_aht20_read_status() & 0x08) == 0x08)    /* 校准输出被启用？ */
    {
        return 0;       /* 正常，返回0 */
    }
    else
    {
        return 1;       /* 异常，返回1 */
    }
}

/**
 * @brief       读取AHT20的温度和湿度数据
 * @param       *temp: 温度值指针
 * @param       *humi: 湿度值指针
 * @retval      无
 */
void atk_aht20_read_data(float *temp, float *humi) 
{
    uint32_t humi_data = 0, temp_data = 0;
    uint8_t raw_data[10];
    uint8_t command[2] = {0x33, 0x00};

    atk_aht20_write_nbytes(START_TEST, command, 2);         /* 发送测量指令 */
    delay_ms(80);                                           /* 等待80ms */

    atk_aht20_read_nbytes(raw_data, 7);                     /* 获取原始数据 */

    if((raw_data[0] & 0x80) == 0x00)
    {
        humi_data = 0;                                      /* 变量清零 */
        humi_data = (humi_data | raw_data[1]) << 8;         /* 取出第一个字节湿度数据，即[19:12]位 */
        humi_data = (humi_data | raw_data[2]) << 8;         /* 取出第二个字节湿度数据，即[11:4]位 */
        humi_data = (humi_data | raw_data[3]);              /* 取出第三个字节湿度数据，即[3:0]位 */
        humi_data = humi_data >> 4;                         /* 湿度有效数据共有20位，第三个字节数据只有高4位有效，因此需要右移4位，才能得到有效数据 */
        *humi = (float)humi_data * 100 /1024 /1024;         /* 计算湿度值 */

        temp_data = 0;                                      /* 变量清零 */
        temp_data = (temp_data | raw_data[3]) << 8;         /* 取出第一个字节温度数据，即[19:16]位 */
        temp_data = (temp_data | raw_data[4]) << 8;         /* 取出第二个字节温度数据，即[15:8]位 */
        temp_data = (temp_data | raw_data[5]);              /* 取出第三个字节温度数据，即[7:0]位 */
        temp_data = temp_data & 0xfffff;                    /* 温度有效数据共有20位，第一个字节数据只有低4位有效，因此需要右移4位，才能得到有效数据 */
        *temp = (float)temp_data * 200 / 1024 /1024 - 50;   /* 计算温度值 */
    }
}

/**
 * @brief   写N个字节到AHT20传感器的寄存器
 * @param   reg_addr:   寄存器地址
 * @param   data:       写入数据
 * @param   len:        数据长度
 * @retval  写入结果
 * @arg     0   : 成功
 * @arg     其他: 失败
 */
uint8_t atk_aht20_write_nbytes(uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    iic_start();                        /* 起始信号 */
    
    iic_send_byte(ATK_AHT20_IIC_ADDR | 0x00);    /* 发送IIC通讯地址 */
    if (iic_wait_ack() == 1)            /* 等待应答 */
    {
        iic_stop();
        return 1;
    }
    
    iic_send_byte(reg_addr);            /* 发送寄存器地址 */
    if (iic_wait_ack() == 1)            /* 等待应答 */
    {
        iic_stop();
        return 1;
    }
    
    for (uint16_t i = 0; i < len; i++)  /* 循环发送数据 */
    {
        iic_send_byte(data[i]);
        if (iic_wait_ack() == 1)        /* 等待应答 */
        {
            iic_stop();
            return 1;
        }
    }
    
    iic_stop();                         /* 停止信号 */
    
    return 0;
}

/**
 * @brief   从AHT20传感器读取N个字节数据
 * @param   data:       数据存储buff
 * @param   len:        数据长度
 * @retval  读出结果
 * @arg     0   : 成功
 * @arg     其他: 失败
 */
uint8_t atk_aht20_read_nbytes(uint8_t *data, uint8_t len)
{    
    iic_start();                        /* 起始信号 */
    
    iic_send_byte(ATK_AHT20_IIC_ADDR | 0x01);
    if (iic_wait_ack() == 1)            /* 等待应答 */
    {
        iic_stop();
        return 1;
    }
    
    while (len)                         /* 循环接收数据 */
    {
        *data = iic_read_byte((len > 1) ? 1 : 0);
        len--;
        data++;
    }
    
    iic_stop();                         /* 停止信号 */
    
    return 0;
}




