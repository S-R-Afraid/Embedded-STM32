#include "24C02.h"
#include "i2c.h"
#include "delay.h"  // 假设 delay_ms 等延时函数在此头文件

#define W24C02_DEV_ADDR 0xA0  // 24C02 设备地址，根据实际硬件调整，一般为 0xA0
#define W24C02_PAGE_SIZE 16

void W24C02_Init(void)
{
    MX_I2C1_Init();  // 初始化 I2C 外设，需确保 MX_I2C1_Init 实现正确
}

uint8_t W24C02_WriteByte(uint8_t innerAddr, uint8_t byte) 
{
    uint8_t ret = HAL_I2C_Mem_Write(&hi2c1, W24C02_DEV_ADDR, innerAddr, I2C_MEMADD_SIZE_8BIT, &byte, 1, HAL_MAX_DELAY);
    if (ret == HAL_OK) 
		{
        delay_ms(5);  // 等待 EEPROM 内部写入完成（EEPROM 写周期要求）
    } 
		else 
		{
        // 错误时串口打印（需确保串口已初始化），方便调试
        printf("W24C02_WriteByte failed! ret=0x%02X, addr=0x%02X, data=0x%02X\r\n", ret, innerAddr, byte);
    }
    return ret;
}

uint8_t W24C02_ReadByte(uint8_t innerAddr)
{
    uint8_t byte = 0;
    uint8_t ret = HAL_I2C_Mem_Read(&hi2c1, W24C02_DEV_ADDR, innerAddr, I2C_MEMADD_SIZE_8BIT, &byte, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        printf("ReadByte failed! ret=0x%02X, addr=0x%02X\r\n", ret, innerAddr);
        // 错误时返回默认值0，但更好的做法是通过指针传参并返回错误码
    }
    return byte; // 返回实际读取的数据
}

uint8_t W24C02_WriteBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len)
{
    uint8_t bytesWritten = 0;  
    while (len > 0)
    {
        uint8_t pageOffset = innerAddr % W24C02_PAGE_SIZE;
        uint8_t spaceInPage = W24C02_PAGE_SIZE - pageOffset;
        uint8_t writeLen = (len <= spaceInPage) ? len : spaceInPage;
        
        // 调用 HAL 库进行页写入
        uint8_t ret = HAL_I2C_Mem_Write(&hi2c1, W24C02_DEV_ADDR, innerAddr, I2C_MEMADD_SIZE_8BIT, bytes, writeLen, HAL_MAX_DELAY);
        if (ret != HAL_OK)
        {
            return ret;  // 返回写入错误码
        }
        
        innerAddr += writeLen;
        bytes += writeLen;
        bytesWritten += writeLen;
        len -= writeLen;
        
        delay_ms(5);  // 等待 EEPROM 完成内部写操作
    }
    return HAL_OK; 
}

uint8_t W24C02_ReadBytes(uint8_t innerAddr, uint8_t *bytes, uint8_t len)
{
    return HAL_I2C_Mem_Read(&hi2c1, W24C02_DEV_ADDR, innerAddr, I2C_MEMADD_SIZE_8BIT, bytes, len, HAL_MAX_DELAY);
}
