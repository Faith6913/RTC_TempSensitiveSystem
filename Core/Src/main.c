/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dht11.h"
#include "stdio.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t temp;
uint8_t humi;
DHT11_Data_TypeDef DHT11_Data;
RTC_DateTypeDef Nowdate;
RTC_TimeTypeDef Nowtime;
RTC_TimeTypeDef Reset_Nowtime;
RTC_DateTypeDef Reset_Nowdate;
uint8_t Time_Select[3] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define HC138Ax_PORT GPIOC
#define HC138A0_PIN GPIO_PIN_10
#define HC138A1_PIN GPIO_PIN_11
#define HC138A2_PIN GPIO_PIN_12
#define HC595_PORT GPIOB
#define HC595_DATA_PIN GPIO_PIN_3
#define HC595_LCLK_PIN GPIO_PIN_4
#define HC595_SCLK_PIN GPIO_PIN_5
uint32_t DuanMa[] = {
		0x3F,  0x06,  0x5B,  0x4F,  0x66,                 /**< 0 - 9 */
		0x6D,  0x7D,  0x07,  0x7F,  0x6F,
		0x00,  0x40,  0x64,  0x54,  0x74,                 /**< NULL - s n h  */
		0x76,  0x39,  0x70,  0x40,                        //H  C  T  -
};
static uint32_t buf[8] = {                                //缓冲区
		0x00,0x00,0x00,0x00,  //1 ~ 4
		0x00,0x00,0x00,0x00   //5 ~ 8
};
void ShuMaGuan_Enable(uint32_t numb){                     //使能某一个数码管
	uint32_t HC138_PIN_INPUT[8][3] = {
			{0,0,0},{0,0,1},{0,1,0},{0,1,1},
			{1,0,0},{1,0,1},{1,1,0},{1,1,1}
	};

	HAL_GPIO_WritePin(HC138Ax_PORT,HC138A0_PIN,HC138_PIN_INPUT[numb-1][2]);
	HAL_GPIO_WritePin(HC138Ax_PORT,HC138A1_PIN,HC138_PIN_INPUT[numb-1][1]);
	HAL_GPIO_WritePin(HC138Ax_PORT,HC138A2_PIN,HC138_PIN_INPUT[numb-1][0]);
}
void Change_BUFF(uint32_t N_Digital,uint32_t Number){    //更改缓冲区
	buf[N_Digital-1] = DuanMa[Number];
}
void Write_Number_to_ST(uint32_t N_Digital){             //写入数据等待触发
	HAL_GPIO_WritePin(HC595_PORT,HC595_LCLK_PIN,GPIO_PIN_RESET);
	for(int i = 0;i < 8;i ++){
		HAL_GPIO_WritePin(HC595_PORT,HC595_SCLK_PIN,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(HC595_PORT,HC595_DATA_PIN,((buf[N_Digital-1] >> (7 - i)) & 0x01));
		HAL_GPIO_WritePin(HC595_PORT,HC595_SCLK_PIN,GPIO_PIN_SET);
	}
}
void Write_Dot(uint32_t N_Digital){                      //写入小数点
	buf[N_Digital-1] += 0x80;
}
void Lighten_ShuMaGuan(){                                //点亮一个数码管
	HAL_GPIO_WritePin(HC595_PORT,HC595_LCLK_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(HC595_PORT,HC595_LCLK_PIN,GPIO_PIN_SET);
}
void Write_Buff(uint32_t position,uint32_t len,uint32_t dat){   //
	uint32_t temp = dat;
	for(uint32_t i = position; i < position + len; i++){
		buf[i] = DuanMa[temp % 10];
		temp /= 10;
	}
}
void Display_1(uint32_t N_Digit,uint32_t Number){         //数码管展示一位数据不带小数点
	Change_BUFF(N_Digit, 10);
	Write_Number_to_ST(N_Digit);
	Lighten_ShuMaGuan();
	ShuMaGuan_Enable(N_Digit);
	Change_BUFF(N_Digit, Number);
	Write_Number_to_ST(N_Digit);
	Lighten_ShuMaGuan();
}
void Display_1_Dot(uint32_t N_Digit,uint32_t Number){     //数码管展示一位数据带小数点
	ShuMaGuan_Enable(N_Digit);
	Change_BUFF(N_Digit, Number);
	Write_Dot(N_Digit);
	Write_Number_to_ST(N_Digit);
	Lighten_ShuMaGuan();
}
//void Display_More(uint32_t start,uint32_t end,uint32_t Num){
//	uint32_t data[8] = {0};
//	void Display_1(uint32_t N_Digit,uint32_t Number);
//	void decode(uint32_t number,uint32_t a[8]);
//	decode(21*100000+5*10000+59*10+4,data);
//	for(int i = 5;i <= 7;i ++){
//		Display_1(9-i, data[i-1]);
//		if(i == 6){
//			Display_1_Dot(9-i, data[i-1]);
//		}
//		for(int j = 0;j <= 10000;j++){}
//	}
//	for(int i = 1;i <= 3;i ++){
//		Display_1(9-i, data[i-1]);
//		if(i == 2){
//			Display_1_Dot(9-i, data[i-1]);
//		}
//		for(int j = 0;j <= 10000;j++){}
//	}
//}
void Display_Buff(){                                     //展示缓冲区数据
	for(int N_Digit = 0;N_Digit < 8;N_Digit ++){
		Write_Number_to_ST(N_Digit+1);
		ShuMaGuan_Enable(N_Digit+1);
		Lighten_ShuMaGuan();
		for(int i = 0;i <= 20000;i++){}
		if(N_Digit == 7){

		}
	}
}
//void CorrectDisplay_1(uint32_t N_Digital,uint32_t Number, uint32_t Dot_If){
//	Write_Number_to_ST(10,0);
//	Lighten_ShuMaGuan();
//	ShuMaGuan_Enable(N_Digital);
//	Write_Number_to_ST(Number,Dot_If);
//	Lighten_ShuMaGuan();
//	for(int j = 0;j <= 10000;j++){}
//	//HAL_Delay(1);
//}
//void Shuma_Put_Dot(uint32_t N_Digital){
////	Write_Number_to_ST(10,0);
////	Lighten_ShuMaGuan();
////	ShuMaGuan_Enable(N_Digital);
////	Write_Number_to_ST(10,1);
////	Lighten_ShuMaGuan();
////	HAL_Delay(1);
//
//}


void decode(uint32_t number,uint32_t buf[8]){
	uint32_t yushu = 0;
	for(int i = 0;i < 8;i ++){
		yushu = number % (uint32_t)pow(10,i+1);
		if(i == 0){
			buf[0] = yushu;
		}
		while(((number - yushu) > 0)){
			buf[i+1] = (number % (uint32_t)pow(10,i+2) - yushu)/(uint32_t)pow(10,i+1);
			break;
		}
	}
}
//同时点亮多个数码管函�??
//void Display_More(){
//	uint32_t buf[8] = {0};
//	void CorrectDisplay_1(uint32_t N_Digital,uint32_t Number, uint32_t Dot_If);
//	void decode(uint32_t number,uint32_t a[8]);
//	decode(21*100000+5*10000+59*10+4,buf);
//	for(int i = 5;i <= 7;i ++){
//		if(i == 68){
//			CorrectDisplay_1(9-i, buf[i-1], 1);
//		}else{
//			CorrectDisplay_1(9-i, buf[i-1], 0);
//		}
//	}
//	for(int i = 1;i <= 3;i ++){
//		if(i == 28){
//			CorrectDisplay_1(9-i, buf[i-1], 1);
//		}else{
//			CorrectDisplay_1(9-i, buf[i-1], 0);
//		}
//	}
//}
//void display_8(uint32_t Stu_Number){
//	uint32_t buff[8] = {0};
//	void CorrectDisplay_1(uint32_t N_Digital,uint32_t Number, uint32_t Dot_If);
//	void decode(uint32_t number,uint32_t a[8]);
//	decode(Stu_Number,buff);
//	for(int i = 1;i <= 8;i ++){
//		CorrectDisplay_1(9-i, buff[i-1], 0);
//	}
//}
//void display_Temp(uint8_t temp_int,uint8_t temp_deci){
//	uint8_t buf[8] = {0};
//	void CorrectDisplay_1(uint32_t N_Digital,uint32_t Number, uint32_t Dot_If);
//	void decode(uint32_t number,uint32_t a[8]);
////	decode(temp_int*100000+temp_deci*10000,buf);
//	decode(21003158,buf);
//	for(int i = 4;i <= 6;i ++){
//		CorrectDisplay_1(9-i, buf[i-1], 0);
////		Write_Number_to_ST(10,0);
////		Lighten_ShuMaGuan();
////		ShuMaGuan_Enable(9-i);
////		Write_Number_to_ST(buf[i-1],1);
////		Lighten_ShuMaGuan();
////		HAL_Delay(1);
//	}
//}

//void RTC_SetMy_Time(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *nowtime,uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm)
//{
//	RTC_TimeTypeDef sTime = {0};
//	RTC_DateTypeDef sDate = {0};
//
//	HAL_RTC_SetTime(&hrtc, &nowtime, RTC_FORMAT_BIN);
//	sTime.Hours = hour;
//	sTime.Minutes = min;
//	sTime.Seconds = sec;
//	sTime.TimeFormat=ampm;
//	sTime.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
//    sTime.StoreOperation=RTC_STOREOPERATION_RESET;
//}

HAL_StatusTypeDef RTC_SetMy_Time(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm)
{
	RTC_TimeTypeDef RTC_TimeStructure;

	RTC_TimeStructure.Hours=hour;
	RTC_TimeStructure.Minutes=min;
	RTC_TimeStructure.Seconds=sec;
	RTC_TimeStructure.TimeFormat=ampm;
	RTC_TimeStructure.DayLightSaving=RTC_DAYLIGHTSAVING_NONE;
    RTC_TimeStructure.StoreOperation=RTC_STOREOPERATION_RESET;
    return HAL_RTC_SetTime(&hrtc, &RTC_TimeStructure, RTC_FORMAT_BIN);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	int key0 = 0;
	int key1 = 0;
	int key2 = 0;
	int Flag = 0;
	int count = 0;
	int Warning_Delete = 0;
	int Temp_Humi_Flag = 0;
	int Change_Max_Flag = 0;
	int RTC_Clock_Flag = 0;
	int Change_Time_Flag = 0;
	int Select_Number = 0;
	uint8_t Max_Temp_int = 25;
	uint8_t Max_Temp_deci = 0;
	HAL_StatusTypeDef RTC_SetMy_Time(uint8_t hour,uint8_t min,uint8_t sec,uint8_t ampm);
	void ShuMaGuan_Enable(uint32_t numb);
	void Write_Number_to_ST(uint32_t number);
	void Lighten_ShuMaGuan();
	void Write_Buff();
	void Change_BUFF(uint32_t N_Digital,uint32_t Number);
	void Display_1(uint32_t N_Digit,uint32_t Number);
	void Display_More(uint32_t start,uint32_t end,uint32_t Num);
	void Display_Buff();

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  MX_TIM1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //HAL_TIM_Base_Start_IT(&htim2);
  //HAL_TIM_Base_Start_IT(&htim4);
  //HAL_UART_Receive_IT(&huart1, &temp, 2);
  //HAL_UART_Receive_IT(&huart1, &humi, 2);
  while(1)
  {
	HAL_RTC_GetTime(&hrtc, &Nowtime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Nowdate, RTC_FORMAT_BIN);
	Warning_Delete = 0;
	Display_Buff();
	if(!Flag){     //Flag = 0 时不断展示学号
		RTC_Clock_Flag = 0;
		Temp_Humi_Flag = 0;
		Write_Buff(0, 8, 85130012);
	}
	if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
		HAL_Delay(20);
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
			Flag = 1;       //按键key1控制学号显示结束
			key1 ++;
			for(uint32_t j = 0;j < 8;j ++){
				buf[j] = DuanMa[10];
			}
			while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
				count ++;     //count 变量用来判断长按还是短按
				for(uint32_t j = 0;j < 8;j ++){
					buf[j] = DuanMa[10];
					Change_BUFF(j+1, 18);
					Display_Buff();
					HAL_Delay(1);
				}
			}
			HAL_Delay(20);
		}
	}else if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_RESET){
			HAL_Delay(20);
			if(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_RESET){
			key2 ++;
			for(uint32_t j = 0;j < 8;j ++){
				buf[j] = DuanMa[10];
			}
			while(HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_2) == GPIO_PIN_RESET){
				for(uint32_t j = 0;j < 8;j ++){
					buf[j] = DuanMa[10];
					Change_BUFF(j+1, 18);
					Display_Buff();
					HAL_Delay(1);
				}
			}
			HAL_Delay(20);
		}
	}
	if(key1 > 0){
		if(count > 5){
			RTC_Clock_Flag = 1;       //时钟显示模式标识
			count = 0;
		}else{
			Temp_Humi_Flag = 1;       //温湿度传感报警模式标识
		}
	}


	//Temperature and Humidity Sensor While
	while(Temp_Humi_Flag){
		//Dispaly and Waring Mode
		if(DHT11_ReadData(&DHT11_Data)){
			printf("Temp:%d.%d   Humi:%d.%d   Temp_Warning:%d.%d\r\n",DHT11_Data.temp_int,
					DHT11_Data.temp_deci,DHT11_Data.humi_int,DHT11_Data.humi_deci,Max_Temp_int,Max_Temp_deci);
			for(uint32_t j = 0;j < 8;j ++){
				buf[j] = DuanMa[10];
			}
			Write_Buff(1, 3, (DHT11_Data.temp_deci*100 + (DHT11_Data.temp_int % 10)*10 + (DHT11_Data.temp_int-(DHT11_Data.temp_int % 10))/10));
			Write_Buff(5, 3, (DHT11_Data.humi_deci*100 + (DHT11_Data.humi_int % 10)*10 + (DHT11_Data.humi_int-(DHT11_Data.humi_int % 10))/10));
			Write_Dot(3);
			Write_Dot(7);
		}
		Change_BUFF(1, 16);
		Change_BUFF(5, 15);
		Display_Buff();
		//Change Max Mode Code
		while(Change_Max_Flag){
			//Debug With UART
			if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET){
				HAL_Delay(20);
				if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) == GPIO_PIN_RESET){
					if(Max_Temp_deci == 0){
						Max_Temp_deci = 9;
						Max_Temp_int --;
					}else{
						Max_Temp_deci --;
					}
					while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) == GPIO_PIN_RESET){
						for(uint32_t j = 0;j < 8;j ++){
							buf[j] = DuanMa[10];
							Display_Buff();
							HAL_Delay(1);
						}
					}
					HAL_Delay(20);
				}
			}
			if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2) == GPIO_PIN_RESET){
				HAL_Delay(20);
				if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2) == GPIO_PIN_RESET){
					if(Max_Temp_deci == 9){
						Max_Temp_deci = 0;
						Max_Temp_int ++;
					}else{
						Max_Temp_deci ++;
					}
					while(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2) == GPIO_PIN_RESET){
						for(uint32_t j = 0;j < 8;j ++){
							buf[j] = DuanMa[10];
							Display_Buff();
						}
					}
					HAL_Delay(20);
				}
			}
			//Display Now Max_Temperature
			for(uint32_t j = 0;j < 8;j ++){
				buf[j] = DuanMa[10];
			}
			Write_Buff(5,8,(Max_Temp_deci*100 + (Max_Temp_int % 10)*10 + (Max_Temp_int -(Max_Temp_int % 10))/10));
			Change_BUFF(4, 15);
			Change_BUFF(5, 16);
			Write_Dot(7);
			Display_Buff();
			//Return To "Display Temperature Humidity"
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
				 HAL_Delay(20);
				 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
					Flag = 1;
					Change_Max_Flag = 0;
					while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
						for(uint32_t j = 0;j < 8;j ++){
							Change_BUFF(j+1, 18);
							Display_Buff();
							HAL_Delay(1);
						}
					}
					HAL_Delay(20);
					break;
				 }
			}
		}
		//High Temperature Warning and Delete
		if(((DHT11_Data.temp_int >= Max_Temp_int)&(DHT11_Data.temp_deci >= Max_Temp_deci))&(!Warning_Delete)){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
				 HAL_Delay(20);
				 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
					 Warning_Delete = 1;
					 while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
							for(uint32_t j = 0;j < 8;j ++){
								Change_BUFF(j+1, 18);
								Display_Buff();
								HAL_Delay(1);
							}
					 }
					 HAL_Delay(20);
				 }
			}
		}else if(((DHT11_Data.temp_int)*10+(DHT11_Data.temp_deci) < (Max_Temp_int*10+Max_Temp_deci))|(Warning_Delete)){
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
		}
		//Key0: Go To Change Mode
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
			HAL_Delay(20);
			if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
				Change_Max_Flag = 1;
				while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
					for(uint32_t j = 0;j < 8;j ++){
						Change_BUFF(j+1, 18);
						Display_Buff();
						HAL_Delay(1);
					}
				}
				HAL_Delay(20);
			}
		}
		//WK-Up:Clear
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
			 HAL_Delay(20);
			 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
				 Temp_Humi_Flag = 0;
				 Flag = 0;
				 key1 = 0;
				 break;
			 }
		}
	}
	//RTC Clock While
	while(RTC_Clock_Flag){
		HAL_RTC_GetTime(&hrtc, &Nowtime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(&hrtc, &Nowdate, RTC_FORMAT_BIN);
		Time_Select[0] = Nowtime.Hours;
		Time_Select[1] = Nowtime.Minutes;
		Time_Select[2] = Nowtime.Seconds;
		//Change Time Mode Code
		while(Change_Time_Flag){
			if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
				HAL_Delay(20);
				if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9) == GPIO_PIN_RESET){
					Select_Number ++;
					Select_Number %= 3;
					while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_9) == GPIO_PIN_RESET){
						switch(Select_Number){
						case 0:
							for(uint32_t j = 3;j < 8;j ++){
								Change_BUFF(j+1, 18);
								Display_Buff();
								HAL_Delay(1);
							}break;
						case 1:
							for(uint32_t j = 0;j < 2;j ++){
								Change_BUFF(j+1, 18);
								Change_BUFF(j+7, 18);
								Display_Buff();
								HAL_Delay(1);
							}break;
						case 2:
							for(uint32_t j = 0;j < 5;j ++){
								Change_BUFF(j+1, 18);
								Display_Buff();
								HAL_Delay(1);
							}break;
						}
					}
					HAL_Delay(20);
				}
			}
			for(int i = 0;i < 3;i ++){
				if(Select_Number == i){
					if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET){
						HAL_Delay(20);
						if(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) == GPIO_PIN_RESET){
							if(i == 0){
								if(Time_Select[i] == 0){
									Time_Select[i] = 23;
								}else{
									Time_Select[i] --;
								}
							}else{
								if(Time_Select[i] == 0){
									Time_Select[i] = 59;
								}else{
									Time_Select[i] --;
								}
							}
							while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_8) == GPIO_PIN_RESET){
								for(uint32_t j = 3*i;j < 3*i+2;j ++){
									buf[j] = DuanMa[10];
									Change_BUFF(j+1, 10);
									Display_Buff();
									HAL_Delay(1);
								}
							}
							HAL_Delay(20);
						}
					}
					if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2) == GPIO_PIN_RESET){
						HAL_Delay(20);
						if(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2) == GPIO_PIN_RESET){
							if(i == 0){
								if(Time_Select[i] == 23){
									Time_Select[i] = 0;
								}else{
									Time_Select[i] ++;
								}
							}else{
								if(Time_Select[i] == 59){
									Time_Select[i] = 0;
								}else{
									Time_Select[i] ++;
								}
							}
							while(HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2) == GPIO_PIN_RESET){
								for(uint32_t j = 3*i;j < 3*i+2;j ++){
									buf[j] = DuanMa[10];
									Change_BUFF(j+1, 10);
									Display_Buff();
									HAL_Delay(1);
								}
							}
							HAL_Delay(20);
						}
					}
				}
			}

			//Display Now time
			for(uint32_t j = 0;j < 8;j ++){
				buf[j] = DuanMa[10];
			}
			for(int i = 0;i < 3;i ++){
				Write_Buff(3*i, 3*i+2, (Time_Select[i] % 10)*10 + (Time_Select[i] -(Time_Select[i] % 10))/10);
			}
			Change_BUFF(3, 18);
			Change_BUFF(6, 18);
			Display_Buff();
			//Return To "Display RTC"
			if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
				 HAL_Delay(20);
				 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
					Flag = 1;
					Change_Time_Flag = 0;
					RTC_SetMy_Time(Time_Select[0],Time_Select[1],Time_Select[2],RTC_FORMAT_BIN);
					while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
						for(uint32_t j = 0;j < 8;j ++){
							buf[j] = DuanMa[10];
							Change_BUFF(j+1, 18);
							Display_Buff();
							HAL_Delay(1);
						}
					}
					HAL_Delay(20);
					break;
				 }
			}
		}
		Write_Buff(0, 2, (Nowtime.Hours % 10)*10 + (Nowtime.Hours -(Nowtime.Hours % 10))/10);
		Write_Buff(3, 5, (Nowtime.Minutes % 10)*10 + (Nowtime.Minutes -(Nowtime.Minutes % 10))/10);
		Write_Buff(6, 8, (Nowtime.Seconds % 10)*10 + (Nowtime.Seconds -(Nowtime.Seconds % 10))/10);
		Change_BUFF(3, 18);
		Change_BUFF(6, 18);
		Display_Buff();
		//Key0: Go To Change Mode
		if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
			HAL_Delay(20);
			if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
				Change_Time_Flag = 1;
				while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9) == GPIO_PIN_RESET){
					for(uint32_t j = 0;j < 8;j ++){
						buf[j] = DuanMa[10];
						Change_BUFF(j+1, 18);
						Display_Buff();
						HAL_Delay(1);
					}
				}
				HAL_Delay(20);
			}
		}
		//WK-Up:Clear
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
			 HAL_Delay(20);
			 if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) != GPIO_PIN_RESET){
				 RTC_Clock_Flag = 0;
				 Flag = 0;
				 key1 = 0;
				 break;
			 }
		}
	}
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x16;
  sTime.Minutes = 0x36;
  sTime.Seconds = 0x30;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_FRIDAY;
  sDate.Month = RTC_MONTH_DECEMBER;
  sDate.Date = 0x17;
  sDate.Year = 0x21;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 100-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 10000-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 5000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 10000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 10000-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin
                          |LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
                          |GPO_HC138_A0_Pin|GPO_HC138_A1_Pin|GPO_HC138_A2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPO_HC595_DS_Pin|GPO_HC595_STCP_Pin|GPO_HC595_SHCP_Pin|BEEP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED0_Pin LED1_Pin LED2_Pin LED3_Pin
                           LED4_Pin LED5_Pin LED6_Pin LED7_Pin
                           GPO_HC138_A0_Pin GPO_HC138_A1_Pin GPO_HC138_A2_Pin */
  GPIO_InitStruct.Pin = LED0_Pin|LED1_Pin|LED2_Pin|LED3_Pin
                          |LED4_Pin|LED5_Pin|LED6_Pin|LED7_Pin
                          |GPO_HC138_A0_Pin|GPO_HC138_A1_Pin|GPO_HC138_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : WK_UP_Pin */
  GPIO_InitStruct.Pin = WK_UP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(WK_UP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : KEY0_Pin KEY1_Pin */
  GPIO_InitStruct.Pin = KEY0_Pin|KEY1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY2_Pin */
  GPIO_InitStruct.Pin = KEY2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPO_HC595_DS_Pin GPO_HC595_STCP_Pin GPO_HC595_SHCP_Pin BEEP_Pin
                           PB9 */
  GPIO_InitStruct.Pin = GPO_HC595_DS_Pin|GPO_HC595_STCP_Pin|GPO_HC595_SHCP_Pin|BEEP_Pin
                          |GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
__attribute__((weak)) int _write(int file, char *ptr, int len){
	if(HAL_UART_Transmit(&huart1,ptr,len,0xffff) != HAL_OK){
		Error_Handler();
	}
}
//定时器Tim2展示程序正在运行
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance == TIM2){
		static uint8_t status = 0;
		GPIOC->ODR = ~(0x01<<status);
		//HAL_GPIO_TogglePin(GPIOC, (0x01<<status));
		status ++;
		status %= 8;

	}
	if(htim->Instance == TIM4){
		printf("Now Time :%d-%d-%d  %d:%d:%d \r\n",Nowdate.Year,Nowdate.Month,Nowdate.Date,Nowtime.Hours,Nowtime.Minutes,Nowtime.Seconds);
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
