
#ifndef STM32F407XX_TIMER_H
#define STM32F407XX_TIMER_H

#include "stm32f407xx.h"
#include "stm32f407xx_common_macro.h"
#include <stdint.h>
#include <stdlib.h>


//Timer structure definition
typedef struct{
	uint16_t reloadVal;	 
	uint16_t prescaler;	
}TIM_Config_t;

typedef struct{
	TIM_TypeDef *TIMxPtr;
	TIM_Config_t *TIMxConfigPtr;
}TIM_Handle_t;

/**
*@brief Timer peripheral clock enable/disable
*@param Pointer to base address of timer 
*@param Enable or disable action
*@return none
*/
void TIM_CLK_ctr(TIM_TypeDef *TIMxPtr, uint8_t enOrDis);

/**
*@brief Timer start/stop 
*
*Set or clear CEN bit in CR register to start/stop timer. 
*
*@param Pointer to base address of timer
*@param Start or Stop action
*@return none
*/
void TIM_ctr(TIM_TypeDef *TIMxPtr, uint8_t startOrStop);

/**
*@brief Initialize timer
*@param Pointer to timer handle struct
*@return none
*/
void TIM_init(TIM_Handle_t *TIMxHandlePtr);

/**
*@brief Initialize timer directly using given parameter
*@param Pointer to timer x peripheral
*@param Reload value
*@param Prescaler
*@return none
*/
void TIM_init_direct(TIM_TypeDef *TIMxPtr,uint16_t reloadVal,uint16_t preScaler);

/**
*@brief Deinitialize timer
*@param Pointer to base address of timer
*@return none
*/
void TIM_deinit(TIM_TypeDef *TIMxPtr);

/**
*@brief 	Reset timer 's counter
*@param 	Pointer to base address of timer
*@return 	None
*/
void TIM_reset_counter(TIM_TypeDef *TIMxPtr);

/**
*@brief Set timer 's reload value
*@param Pointer to base address of timer
*@param Reload value
*@return none
*/
void TIM_set_reload_val(TIM_TypeDef *TIMxPtr, uint16_t reloadVal);

/**
*@brief Set timer 's prescaler
*@param Pointer to base address of timer
*@param Prescaler
*@return none
*/
void TIM_set_prescaler(TIM_TypeDef *TIMxPtr, uint16_t prescaler);

/**
*@brief Enable or disable interrupt for update event of timer
*@param Pointer to base address of timer
*@param Enable or disable action
*@return none
*/
void TIM_interrupt_ctr(TIM_TypeDef *TIMxPtr, uint8_t enOrDis);

/**
*@brief Enable or disable timer 's interrupt vector in NVIC 
*@param IRQ number
*@param Enable or disable action
*@return none
*/
void TIM_intrpt_vector_ctr (uint8_t IRQnumber, uint8_t enOrDis);

/**
*@brief Config priority for timer 's interrupt 
*@param IRQ number
*@param Priority
*@return none
*/
void TIM_intrpt_priority_config(uint8_t IRQnumber, uint8_t priority);

/**
*@brief Timer interrupt handler
*@param Pointer to base address of timer
*@return none
*/
void TIM_intrpt_handler (TIM_TypeDef *TIMxPtr);

/**
*@brief Configure timer trigger output (TRGO) on update event
*@param Pointer to base address of timer
*@return none
*/
void TIM_update_event_TRGO (TIM_TypeDef *TIMxPtr);

uint8_t TIM_counter_status(TIM_TypeDef *TIMxPtr);
#endif
