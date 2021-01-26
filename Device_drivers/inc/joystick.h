/*
 * provides functions for interfacing with joystick
 * The joystick communicate its position (in X axis & Y axis) with the MCU through 2 ADC channel.
 */

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "stm32f407xx.h"
#include "stm32f407xx_gpio.h"
#include "stm32f407xx_adc.h"

/***********************************************************************
Macro definition
***********************************************************************/

/*
*X axis and Y axis position threshold (in 12 bits digital number)
*/
#define X_POS_THRES_L 				15
#define X_POS_THRES_H 				4060
#define Y_POS_THRES_L 				15
#define Y_POS_THRES_H 				4060

/*
*X axis and Y axis direction
*/
#define X_DIR_LEFT 					-1
#define X_DIR_RIGHT 				1
#define X_DIR_CENTER 				0
#define Y_DIR_UP 					1
#define Y_DIR_DOWN 					-1
#define Y_DIR_CENTER 				0
/*
*Joystick direction
*/
#define JS_DIR_LEFT_UP 				1
#define JS_DIR_LEFT_DOWN 			2
#define JS_DIR_LEFT 				3
#define JS_DIR_RIGHT_UP 			4
#define JS_DIR_RIGHT_DOWN 			5
#define JS_DIR_RIGHT 				6
#define JS_DIR_UP 					7
#define JS_DIR_DOWN 				8
#define JS_DIR_CENTERED 			0

#define JOYSTICK_ADC					ADC1
#define JOYSTICK_X_ADC_CHANNEL			ADC_CHANNEL_5
#define JOYSTICK_Y_ADC_CHANNEL			ADC_CHANNEL_7

/***********************************************************************
Function prototype
***********************************************************************/

/**
*@brief Initilize joystick
*@param Pointer to ADCx peripheral (x = 1,2,3)
*@param X axis ADC channel
*@param Y axis ADC channel
*@return none
*/
void joystick_init(ADC_TypeDef *ADCxPtr, uint8_t X_axis_ADC_channel, uint8_t Y_axis_ADC_channel);

/**
*@brief Deinitilize joystick
*@param Pointer to ADCx peripheral (x = 1,2,3)
*@return none
*/
void joystick_deinit(ADC_TypeDef *ADCxPtr);

/**
*@brief Read joystick direction
*
*This read 12 bits digital number from X & Y axis ADC channel then determine the direction of joystick.
*
*@param Pointer to ADCx peripheral (x = 1,2,3)
*@param X axis ADC channel
*@param Y axis ADC channel
*@return Indicator of joystick direction
*/
uint8_t joystick_read_direction(ADC_TypeDef *ADCxPtr, uint8_t X_axis_ADC_channel, uint8_t Y_axis_ADC_channel);
#endif
