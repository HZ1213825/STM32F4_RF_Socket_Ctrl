#ifndef _RF_H
#define _RF_H

#include "stm32f4xx.h" // 注意更改这个文件的#define STM32F401xx为自己的芯片型号
#include "Delay.h"
//接收
// GPIO
#define RF_Read_GPIO_RCC RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE)
#define RF_Read_GPIOx GPIOB
#define RF_Read_GPIO_Pin GPIO_Pin_0

//外部中断
#define RF_Read_EXIT_Link SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource0)
#define RF_Read_EXIT_Pin EXTI_Line0
#define RF_Read_EXIT_IRQn EXTI0_IRQn
#define RF_Read_EXIT_Priority_1 3
#define RF_Read_EXIT_Priority_2 2
#define RF_Read_EXTI_IRQHandler EXTI0_IRQHandler

//定时器
#define RF_Read_TIM_RCC RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE)
#define RF_Read_TIM_TIMx TIM3
#define RF_Read_TIM_IRQn TIM3_IRQn
#define RF_Read_TIM_Priority_1 2
#define RF_Read_TIM_Priority_2 2
#define RF_Read_TIM_IRQHandler TIM3_IRQHandler

//发送
#define RF_Send_GPIO_RCC RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
#define RF_Send_GPIOx GPIOA
#define RF_Send_GPIO_Pin GPIO_Pin_0

#define RF_Rean_Len 3
extern u8 RF_READ_OK;                //解码成功标志
extern u8 RF_READ_data[RF_Rean_Len]; //发送的数据

void RF_Read_Init(void);
void RF_Send(u8 *Dat, u8 Len);
void RF_Send_init(void);
#endif
