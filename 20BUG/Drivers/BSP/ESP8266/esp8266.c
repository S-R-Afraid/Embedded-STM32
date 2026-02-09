

/*****************辰哥单片机设计******************
											STM32
 * 项目			:	ESP8266模块通信实验                     
 * 版本			: V1.0
 * 日期			: 2024.9.30
 * MCU			:	STM32F103C8T6
 * 接口			:	串口2					
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥 

**********************BEGIN***********************/

#include "./BSP/ESP8266/esp8266.h"
//#include "common.h"
/*
ESP8266-01S使用串口与其它设被进行有线通信，*并不是说ESP8266是一个串口*！
我们设置的串口是用于和ESP8266直接进行通信，而不是用于ESP和其它设备通信。
我们如果想要通过ESP与外部通信，需要使用AT指令
*/

/*
功能导航：
    处理接收：stm32f1xx_it.c里USART2_IRQHandler();
        该函数会把接收到的数据存入结构体里，并逐字节的通过串口一发送；
        当接收完毕后，还会判断本次是否接收到了“CLOSED\r\n”
    
*/

volatile uint8_t ucTcpClosedFlag = 0;
volatile char ESP8266_USART_receiveByte = 0;    //当前ESP8266发给串口的字节

char cStr [ 1500 ] = { 0 };
static void                   ESP8266_GPIO_Config                 ( void );
static void                   ESP8266_USART_Config                ( void );
static void                   ESP8266_USART_NVIC_Configuration    ( void );
struct  STRUCT_USARTx_Fram strEsp8266_Fram_Record = { 0 };




/*
透传（Transparent Transmission）是指数据在传输过程中不被修改或检查，直接从发送端传输到接收端。
在 ESP8266 的 STA（Station）TCP 客户端模式下，
透传模式允许 ESP8266 模块将从串口接收到的数据
    直接通过 WiFi 网络发送到指定的 TCP 服务器，反之亦然。
*/
/**
  * @brief  ESP8266 （Sta Tcp Client）透传
  * @param  无
  * @retval 无
  */
void ESP8266_StaTcpClient ( void )
{
//	uint8_t ucStatus;
//	char cStr [ 100 ] = { 0 };
  my_USART_printf ( 1,"\r\n正在配置 ESP8266 ......\r\n" );

    ESP8266_CH_ENABLE();
    LED1_TOGGLE();
    delay_ms(50);
    ESP8266_AT_Test ();
    LED1_TOGGLE();
    delay_ms(50);
    ESP8266_Net_Mode_Choose ( STA );    //设置工作模式为STA
    LED1_TOGGLE();
    delay_ms(50);
    while ( ! ESP8266_JoinAP ( User_ESP8266_ApSsid, User_ESP8266_ApPwd ) ){//尝试链接热点
        //LED0_TOGGLE();
        my_USART_printf ( 1,"\r\n未连接成功 ......\r\n" );
    }
    LED0_TOGGLE();
    delay_ms(50);
    ESP8266_Cmd ( "AT+CIFSR", "OK", 0, 1000 );      //查询本地 IP 地址
    LED1_TOGGLE();
    delay_ms(50);
    ESP8266_Cmd ( "AT+CIPMUX=1", "OK", 0, 1000 );   //设置为TCP多链接模式
    LED1_TOGGLE();
    delay_ms(50);
    
    ESP8266_Cmd ( "AT+CIPSERVER=1,8288", "OK", 0, 1000 );//以8288为端口建立服务器
    //ESP8266_Cmd("AT+CIPSTART="TCP",192.168.1.1,8000","OK",0,1000);
    
    my_USART_printf( 1,"\r\n配置 ESP8266 完毕\r\n" ); 
    
    
        
}


/**
  * @brief  ESP8266初始化函数
  * @param  无
  * @retval 无
  */
void ESP8266_Init ( void )
{
    ESP8266_GPIO_Config (); 
	
	ESP8266_USART_Config (); 
	
	
//	ESP8266_RST_HIGH_LEVEL();

//	ESP8266_CH_DISABLE();
	
	
}


/**
  * @brief  初始化ESP8266用到的GPIO引脚
  * @param  无
  * @retval 无
  */
static void ESP8266_GPIO_Config ( void )
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;


	/* 配置 CH_PD 引脚*/
	ESP8266_CH_PD_APBxClock_FUN(); 

	GPIO_InitStructure.Pin = ESP8266_CH_PD_PIN;	

	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;   
   
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; 

	HAL_GPIO_Init ( ESP8266_CH_PD_PORT, & GPIO_InitStructure );	 

	
	/* 配置 RST 引脚*/
	ESP8266_RST_APBxClock_FUN(); 

	GPIO_InitStructure.Pin = ESP8266_RST_PIN;

	HAL_GPIO_Init ( ESP8266_RST_PORT, & GPIO_InitStructure );	 


}


/**
  * @brief  初始化ESP8266用到的 USART
  * @param  无
  * @retval 无
  */

static void ESP8266_USART_Config ( void )
{
	GPIO_InitTypeDef gpio_init_struct;
	USART_InitTypeDef USART_InitStructure;
	
	
	/* config USART clock */
	ESP8266_USART_APBxClock_FUN( );
	ESP8266_USART_GPIO_APBxClock_FUN ( );
	
	/* USART GPIO config */
    gpio_init_struct.Pin = ESP8266_USART_TX_PIN;               /* 串口发送引脚号 */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* 复用推挽输出 */
    //IO复用是指IO口由非GPIO外设控制(如串口)，相反，通用是指由GPIO外设控制
    //每个IO口支持的复用功能不一样，可以在芯片手册的引脚定义里查找
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;          /* IO速度设置为高速 */
    HAL_GPIO_Init(ESP8266_USART_TX_PORT, &gpio_init_struct);
            
    gpio_init_struct.Pin = ESP8266_USART_RX_PIN;               /* 串口RX脚 模式设置 */
    gpio_init_struct.Mode = GPIO_MODE_AF_INPUT;    
    HAL_GPIO_Init(ESP8266_USART_RX_PORT, &gpio_init_struct);   /* 串口RX脚 必须设置成输入模式 */
	
	/* USART1 mode config */
    usart_init(ESP8266_USARTx,ESP8266_USART_BAUD_RATE);

	
	/* 中断配置 */
    HAL_UART_Receive_IT(&g_uartx_handle[ESP8266_USARTx], &g_uart2_rx_buffer, 1);
    //HAL_UART_Receive_IT(&g_uartx_handle[ESP8266_USARTx], (uint8_t *)strEsp8266_Fram_Record.Data_RX_BUF, RX_BUF_MAX_LEN);//使能串口接收中断 该语句在USART2_IRQHandler里写了
    __HAL_UART_ENABLE_IT(&g_uartx_handle[ESP8266_USARTx], UART_IT_IDLE);//使能串口总线空闲中断
//	USART_ITConfig ( ESP8266_USARTx, USART_IT_RXNE, ENABLE ); //使能串口接收中断
//	USART_ITConfig ( ESP8266_USARTx, USART_IT_IDLE, ENABLE ); //使能串口总线空闲中断 	

	ESP8266_USART_NVIC_Configuration ();
	
	
	//USART_Cmd(ESP8266_USARTx, ENABLE);    //串口使能在usart_init里实现了，因此这里注释掉
	
	
}


/**
  * @brief  配置 ESP8266 USART 的 NVIC 中断
  * @param  无
  * @retval 无
  */
static void ESP8266_USART_NVIC_Configuration ( void )
{
    //    // 配置 NVIC 优先级分组，由于在stm32f1xx_hal.c里设置过了，在此不用设置
//    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_2);

    // 配置 USART 中断优先级
    HAL_NVIC_SetPriority(ESP8266_USART_IRQ, 3, 3);

    // 使能 USART 中断
    HAL_NVIC_EnableIRQ(ESP8266_USART_IRQ);
    

}


/*
 * 函数名：ESP8266_Rst
 * 描述  ：重启WF-ESP8266模块
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被 ESP8266_AT_Test 调用
 */
void ESP8266_Rst ( void )
{
	#if 0
	 ESP8266_Cmd ( "AT+RST", "OK", "ready", 2500 );   	
	
	#else
	 ESP8266_RST_LOW_LEVEL();
	 delay_ms ( 500 ); 
	 ESP8266_RST_HIGH_LEVEL();
	#endif

}


/*
 * 函数名：ESP8266_Cmd
 * 描述  ：对WF-ESP8266模块发送AT指令
 * 输入  ：cmd，待发送的指令
 *         reply1，reply2，期待的响应，为NULL表不需响应，两者为或逻辑关系
 *         waittime，等待响应的时间
 * 返回  : 1，指令发送成功
 *         0，指令发送失败
 * 调用  ：被外部调用
 */
bool ESP8266_Cmd ( char * cmd, char * reply1, char * reply2, uint32_t waittime )
{    
	strEsp8266_Fram_Record .InfBit .FramLength = 0;               //从新开始接收新的数据包

	PC_Usart("正在发送%s\r\n", cmd);
    ESP8266_Usart ( "%s\r\n", cmd );

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //不需要接收数据
		return true;
	
	delay_ms( waittime );                 //延时
	
	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength++ ]  = '\0';

	PC_Usart ( "%s\r\n", strEsp8266_Fram_Record .Data_RX_BUF );
  
	if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) || 
						 ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) ); 
 	
	else if ( reply1 != 0 )
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply1 ) );
	
	else
		return ( ( bool ) strstr ( strEsp8266_Fram_Record .Data_RX_BUF, reply2 ) );
	
}


/*
 * 函数名：ESP8266_AT_Test
 * 描述  ：对WF-ESP8266模块进行AT测试启动
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */

void ESP8266_AT_Test ( void )
{
	char count=0;
	
	ESP8266_RST_HIGH_LEVEL();	
	delay_ms( 1000 );
	while ( count < 10 )
	{
		if( ESP8266_Cmd ( "AT", "OK", NULL, 500 ) ) return;
		ESP8266_Rst();
		++ count;
	}
}


/*
 * 函数名：ESP8266_Net_Mode_Choose
 * 描述  ：选择WF-ESP8266模块的工作模式
 * 输入  ：enumMode，工作模式
 * 返回  : 1，选择成功
 *         0，选择失败
 * 调用  ：被外部调用
 */
bool ESP8266_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
        case STA:
            return ESP8266_Cmd ( "AT+CWMODE=1", "OK", "no change", 2500 ); 
        
        case AP:
            return ESP8266_Cmd ( "AT+CWMODE=2", "OK", "no change", 2500 ); 
    
        case STA_AP:
            return ESP8266_Cmd ( "AT+CWMODE=3", "OK", "no change", 2500 ); 
        
        default:
            return false;
    }
    
}


/*
 * 函数名：ESP8266_JoinAP
 * 描述  ：WF-ESP8266模块连接外部WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool ESP8266_JoinAP ( char * pSSID, char * pPassWord )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWJAP_DEF=\"%s\",\"%s\"", pSSID, pPassWord );
	
	return ESP8266_Cmd ( cCmd, "OK", NULL, 5000 );
	
}


/*
 * 函数名：ESP8266_BuildAP
 * 描述  ：WF-ESP8266模块创建WiFi热点
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 *       ：enunPsdMode，WiFi加密方式代号字符串
 * 返回  : 1，创建成功
 *         0，创建失败
 * 调用  ：被外部调用
 */
bool ESP8266_BuildAP ( char * pSSID, char * pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode )
{
	char cCmd [120];

	sprintf ( cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode );
	
	return ESP8266_Cmd ( cCmd, "OK", 0, 1000 );
	
}


/*
 * 函数名：ESP8266_Enable_MultipleId
 * 描述  ：WF-ESP8266模块启动多连接
 * 输入  ：enumEnUnvarnishTx，配置是否多连接
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
bool ESP8266_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
	
	return ESP8266_Cmd ( "AT+CIPMUX=%d", "OK", 0, 500 );
	
}


/*
 * 函数名：ESP8266_Link_Server
 * 描述  ：WF-ESP8266模块连接外部服务器
 * 输入  ：enumE，网络协议
 *       ：ip，服务器IP字符串
 *       ：ComNum，服务器端口字符串
 *       ：id，模块连接服务器的ID
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
bool ESP8266_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];

  switch (  enumE )
  {
        case enumTCP:
        sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
        break;
        
        case enumUDP:
        sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
        break;
        
        default:
            break;
  }

  if ( id < 5 )
    sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);

  else
	  sprintf ( cCmd, "AT+CIPSTART=%s", cStr );

	return ESP8266_Cmd ( cCmd, "OK", "ALREAY CONNECT", 4000 );
	
}


/*
 * 函数名：ESP8266_StartOrShutServer
 * 描述  ：WF-ESP8266模块开启或关闭服务器模式
 * 输入  ：enumMode，开启/关闭
 *       ：pPortNum，服务器端口号字符串
 *       ：pTimeOver，服务器超时时间字符串，单位：秒
 * 返回  : 1，操作成功
 *         0，操作失败
 * 调用  ：被外部调用
 */
bool ESP8266_StartOrShutServer ( FunctionalState enumMode, char * pPortNum, char * pTimeOver )
{
	char cCmd1 [120], cCmd2 [120];

	if ( enumMode )
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum );
		
		sprintf ( cCmd2, "AT+CIPSTO=%s", pTimeOver );

		return ( ESP8266_Cmd ( cCmd1, "OK", 0, 500 ) &&
						 ESP8266_Cmd ( cCmd2, "OK", 0, 500 ) );
	}
	
	else
	{
		sprintf ( cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum );

		return ESP8266_Cmd ( cCmd1, "OK", 0, 500 );
	}
	
}


/*
 * 函数名：ESP8266_Get_LinkStatus
 * 描述  ：获取 WF-ESP8266 的连接状态，较适合单端口时使用
 * 输入  ：无
 * 返回  : 2，获得ip
 *         3，建立连接
 *         3，失去连接
 *         0，获取状态失败
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Get_LinkStatus ( void )
{
	if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:2\r\n" ) )
			return 2;
		
		else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:3\r\n" ) )
			return 3;
		
		else if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "STATUS:4\r\n" ) )
			return 4;

	}
	
	return 0;
	
}


/*
 * 函数名：ESP8266_Get_IdLinkStatus
 * 描述  ：获取 WF-ESP8266 的端口（Id）连接状态，较适合多端口时使用
 * 输入  ：无
 * 返回  : 端口（Id）的连接状态，低5位为有效位，分别对应Id5~0，某位若置1表该Id建立了连接，若被清0表该Id未建立连接
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Get_IdLinkStatus ( void )
{
	uint8_t ucIdLinkStatus = 0x00;
	
	
	if ( ESP8266_Cmd ( "AT+CIPSTATUS", "OK", 0, 500 ) )
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:0," ) )
			ucIdLinkStatus |= 0x01;
		else 
			ucIdLinkStatus &= ~ 0x01;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:1," ) )
			ucIdLinkStatus |= 0x02;
		else 
			ucIdLinkStatus &= ~ 0x02;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:2," ) )
			ucIdLinkStatus |= 0x04;
		else 
			ucIdLinkStatus &= ~ 0x04;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:3," ) )
			ucIdLinkStatus |= 0x08;
		else 
			ucIdLinkStatus &= ~ 0x08;
		
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+CIPSTATUS:4," ) )
			ucIdLinkStatus |= 0x10;
		else 
			ucIdLinkStatus &= ~ 0x10;	

	}
	
	return ucIdLinkStatus;
	
}


/*
 * 函数名：ESP8266_Inquire_ApIp
 * 描述  ：获取 F-ESP8266 的 AP IP
 * 输入  ：pApIp，存放 AP IP 的数组的首地址
 *         ucArrayLength，存放 AP IP 的数组的长度
 * 返回  : 0，获取失败
 *         1，获取成功
 * 调用  ：被外部调用
 */
uint8_t ESP8266_Inquire_ApIp ( char * pApIp, uint8_t ucArrayLength )
{
	char uc;
	
	char * pCh;
	
	
  ESP8266_Cmd ( "AT+CIFSR", "OK", 0, 500 );
	
	pCh = strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "APIP,\"" );
	
	if ( pCh )
		pCh += 6;
	
	else
		return 0;
	
	for ( uc = 0; uc < ucArrayLength; uc ++ )
	{
		pApIp [ uc ] = * ( pCh + uc);
		
		if ( pApIp [ uc ] == '\"' )
		{
			pApIp [ uc ] = '\0';
			break;
		}
		
	}
	
	return 1;
	
}


/*
 * 函数名：ESP8266_UnvarnishSend
 * 描述  ：配置WF-ESP8266模块进入透传发送
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
//bool ESP8266_UnvarnishSend ( void )
//{
//	 ESP8266_Cmd ( "AT+CIPMODE=1", "OK", 0, 500 );
//}




/*
 * 函数名：ESP8266_ExitUnvarnishSend
 * 描述  ：配置WF-ESP8266模块退出透传模式
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void ESP8266_ExitUnvarnishSend ( void )
{
	delay_ms( 1000 );
	
	ESP8266_Usart ( "+++" );
	
	delay_ms( 500 ); 
	
}


/*
 * 函数名：ESP8266_SendString
 * 描述  ：WF-ESP8266模块发送字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 *       ：pStr，要发送的字符串
 *       ：ulStrLength，要发送的字符串的字节数
 *       ：ucId，哪个ID发送的字符串
 * 返回  : 1，发送成功
 *         0，发送失败
 * 调用  ：被外部调用
 */
bool ESP8266_SendString ( FunctionalState enumEnUnvarnishTx, char * pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId )
{
	char cStr [20];
	bool bRet = false;
	
		
	if ( enumEnUnvarnishTx )
	{
		ESP8266_Usart ( "%s", pStr );
		
		bRet = true;
		
	}

	else
	{
		if ( ucId < 5 )
			sprintf ( cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2 );

		else
			sprintf ( cStr, "AT+CIPSEND=%d", ulStrLength + 2 );
		
		ESP8266_Cmd ( cStr, "> ", 0, 1000 );

		bRet = ESP8266_Cmd ( pStr, "SEND OK", 0, 1000 );
  }
	
	return bRet;

}


/*
 * 函数名：ESP8266_ReceiveString
 * 描述  ：WF-ESP8266模块接收字符串
 * 输入  ：enumEnUnvarnishTx，声明是否已使能了透传模式
 * 返回  : 接收到的字符串首地址
 * 调用  ：被外部调用
 */
char * ESP8266_ReceiveString ( FunctionalState enumEnUnvarnishTx )
{
	char * pRecStr = 0;
	
	
	strEsp8266_Fram_Record .InfBit .FramLength = 0;
	strEsp8266_Fram_Record .InfBit .FramFinishFlag = 0;
	
	while ( ! strEsp8266_Fram_Record .InfBit .FramFinishFlag );
	strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ] = '\0';
	
	if ( enumEnUnvarnishTx )
		pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;
	
	else 
	{
		if ( strstr ( strEsp8266_Fram_Record .Data_RX_BUF, "+IPD" ) )
			pRecStr = strEsp8266_Fram_Record .Data_RX_BUF;

	}

	return pRecStr;
	
}




