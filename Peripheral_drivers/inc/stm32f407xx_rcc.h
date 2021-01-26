
#ifndef STM32F407XX_RCC_H
#define STM32F407XX_RCC_H

#include "stm32f407xx.h"
#include "stm32f407xx_common_macro.h"
#include "stm32f407xx_gpio.h"
#include <stdint.h>
#include <stdlib.h>

#define RCC_MCO2_SYSCLK 			0
#define RCC_MCO2_PLLI2S				1
#define RCC_MCO2_HSE				2
#define RCC_MCO2_PLL				3

#define RCC_MCO1_HSI 				0
#define RCC_MCO1_LSE				1
#define RCC_MCO1_HSE				2
#define RCC_MCO1_PLL				3

#define RCC_MCO_DIV2 				4
#define RCC_MCO_DIV3				5
#define RCC_MCO_DIV4				6
#define RCC_MCO_DIV5				7

#define RCC_APB_DIV2 				4
#define RCC_APB_DIV4 				5
#define RCC_APB_DIV8 				6
#define RCC_APB_DIV16 				7

#define RCC_AHB_DIV2 				8
#define RCC_AHB_DIV4 				9
#define RCC_AHB_DIV8 				10
#define RCC_AHB_DIV16 				11
#define RCC_AHB_DIV64 				12
#define RCC_AHB_DIV128 				13
#define RCC_AHB_DIV256 				14
#define RCC_AHB_DIV512 				15

/*
*@RCC_SYSCLK
*System clock source selection
*/
#define RCC_SYSCLK_HSI 				0
#define RCC_SYSCLK_HSE 				1
#define RCC_SYSCLK_PLL				2

#define RCC_PLL_150_VCO_INPUT_DIV	8
#define RCC_PLL_150_VCO_OUTPUT_MUL 	300
#define RCC_PLL_150_PLL_OUTPUT_DIV	0
#define RCC_PLL_150_MAIN_PLL_DIV	7

#define APB1 						0
#define APB2 						1

/***********************************************************************
RCC driver functions prototype
***********************************************************************/

/**
*@brief 		Configure MCO2 for clock measurement 
*@param 	Measure system clock or PLLI2S or HSE or PLL
*@param	Prescaler
*@return 	None
*/
void RCC_MCO2_config (uint8_t option, uint8_t prescaler);

/**
*@brief 		Configure MCO1 for clock measurement 
*@param 	Measure HSI or LSE or HSE or PLL
*@param	Prescaler
*@return 	None
*/
void RCC_MCO1_config (uint8_t option, uint8_t prescaler);

/**
*@brief 		Set system clock source as HSE
*@param 	None
*@return 	None
*/
void RCC_set_SYSCLK_HSE (void);

/**
*@brief 		Set system clock source as PLL 84 MHz
*@param 	None
*@return 	None
*/
void RCC_set_SYSCLK_PLL_150_MHz (void);

/**
*@brief 		Get system clock value
*@return 	-1:	PLL is configured as system clock source however configration is wrong
*								system clock value
*/
int32_t RCC_get_SYSCLK_value (void);

/**
*@brief 		Get APB bus clock value
*@param 	APB1 or APB2
*@return 	-1:	PLL is configured as system clock source however configration is wrong
*								APB1 or APB2 bus clock value
*/
int32_t RCC_get_PCLK_value(uint8_t APBx);

/***********************************************************************
Private function prototype
***********************************************************************/

/**
*@brief 	enable/disable HSI clock
*@param 	@EN/DI in stm32f407xx_common_macro.h
*@return 	None
*/
void	RCC_HSI_clock_ctrl (uint8_t enOrDis);

/**
*@brief 	Enable/disable HSE clock
*@param 	@EN/DI in stm32f407xx_common_macro.h
*@return 	None
*/
void	RCC_HSE_clock_ctrl (uint8_t enOrDis);

/**
*@brief 	enable/disable PLL clock
*@param 	@EN/DI in stm32f407xx_common_macro.h
*@return 	None
*/
void	RCC_PLL_clock_ctrl (uint8_t enOrDis);
void PWR_set_scale_mode (void);

/**
*@brief 	set Flash latency (in case of increasing system clock)
*@param 	None
*@return 	None
*/
void FLASH_set_latency (void);

/**
*@brief 	configure PLL as 150 MHz and turn PLL on
*@param 	None
*@return 	None
*/
void RCC_set_PLL_150_MHz (void);
int32_t RCC_get_PLL_output (void);
void RCC_delay( volatile uint32_t delay);


#endif
