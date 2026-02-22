#include "stm32f10x.h"                  // Device header

/**
  * 函    数：电位器旋钮初始化
  * 参    数：无
  * 返 回 值：无
  */
void RP_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);	//开启ADC2的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	
	/*设置ADC时钟*/
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);						//选择时钟6分频，ADCCLK = 72MHz / 6 = 12MHz
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA2、PA3、PA4、PA5引脚初始化为模拟输入
	
	/*ADC初始化*/
	ADC_InitTypeDef ADC_InitStructure;						//定义结构体变量
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;      //模式，选择独立模式，即单独使用ADC2
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;  //数据对齐，选择右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//外部触发，使用软件触发，不需要外部触发gConv_None;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;     //连续转换，失能，每转换一次规则组序列后停止
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;           //扫描模式，失能，只转换规则组的序列1这一个位置
	ADC_InitStructure.ADC_NbrOfChannel = 1;                 //通道数，为1，仅在扫描模式下，才需要指定大于1的数，在非扫描模式下，只能是1
	ADC_Init(ADC2, &ADC_InitStructure);                     //将结构体变量交给ADC_Init，配置ADC2
	
	/*ADC使能*/
	ADC_Cmd(ADC2, ENABLE);									//使能ADC2，ADC开始运行
	
	/*ADC校准*/
	ADC_ResetCalibration(ADC2);								//固定流程，内部有电路会自动执行校准
	while (ADC_GetResetCalibrationStatus(ADC2) == SET);
	ADC_StartCalibration(ADC2);
	while (ADC_GetCalibrationStatus(ADC2) == SET);
}

/**
  * 函    数：获取电位器旋钮的AD值
  * 参    数：n 指定电位器旋钮，范围：1~4，分别表示RP1、RP2、RP3、RP4
  * 返 回 值：AD转换的值，范围：0~4095
  */
uint16_t RP_GetValue(uint8_t n)
{
	if (n == 1)				//指定读取RP1
	{
		/*配置规则组为通道2（PA2）*/
		ADC_RegularChannelConfig(ADC2, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5);
	}
	else if (n == 2)		//指定读取RP2
	{
		/*配置规则组为通道3（PA3）*/
		ADC_RegularChannelConfig(ADC2, ADC_Channel_3, 1, ADC_SampleTime_55Cycles5);
	}
	else if (n == 3)		//指定读取RP3
	{
		/*配置规则组为通道4（PA4）*/
		ADC_RegularChannelConfig(ADC2, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);
	}
	else if (n == 4)		//指定读取RP4
	{
		/*配置规则组为通道5（PA5）*/
		ADC_RegularChannelConfig(ADC2, ADC_Channel_5, 1, ADC_SampleTime_55Cycles5);
	}
	
	ADC_SoftwareStartConvCmd(ADC2, ENABLE);					//软件触发AD转换一次
	while (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == RESET);	//等待EOC标志位，即等待AD转换结束
	return ADC_GetConversionValue(ADC2);					//读数据寄存器，得到AD转换的结果
}
