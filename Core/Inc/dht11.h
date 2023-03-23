/* 使用方法：
1.   B9 配置成 GPIO 输出; TIM1 分频 100-1
2.   示例代码
定义变量：
  DHT11_Data_TypeDef DHT11_Data;
循环读取值到   DHT11_Data
  DHT11_ReadData(&DHT11_Data)
*/
#ifndef DHT11_H
#define DHT11_H
 
#include "stm32f4xx_hal.h"
 
#define DHT11_PORT		    GPIOB
#define DHT11_PIN		      GPIO_PIN_9
 
#define DHT11_OUT_1				HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET)
#define DHT11_OUT_0				HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET)
 
#define DHT11_IN					HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)
 
typedef struct
{
	uint8_t humi_int;				// 湿度的整数部分
	uint8_t humi_deci;	 		// 湿度的小数部分
	uint8_t temp_int;	 			// 温度的整数部分
	uint8_t temp_deci;	 		// 温度的小数部分
	uint8_t check_sum;	 		// 校验和
		                 
} DHT11_Data_TypeDef;
 
uint8_t DHT11_ReadData(DHT11_Data_TypeDef* DHT11_Data);
 
#endif	/* DHT11_H */


