/*
 * game_engine.h
 *
 *  Created on: June 8, 2020
 *      Author: manish
 */

#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include "stm32f407xx_timer.h"
#include "stm32f407xx_rcc.h"
#include "stm32f407xx_rng.h"
#include "ili9341.h"					//LCD Driver
#include "joystick.h"
#include "button.h"
#include "led.h"
#include <math.h>
#include <stdio.h>
#include "vector.h"


#define SHOOT_BUTTON_PORT				GPIOC
#define	SHOOT_BUTTON_PIN				GPIO_PIN_NO_1
#define	SHOOT_BUTTON_READ				button_read(SHOOT_BUTTON_PORT,SHOOT_BUTTON_PIN)

#define THRUST_BUTTON_PORT				GPIOC
#define	THRUST_BUTTON_PIN				GPIO_PIN_NO_3
#define	THRUST_BUTTON_READ				button_read(THRUST_BUTTON_PORT,THRUST_BUTTON_PIN)

#define PROTOBOARD_GREEN_LED_PORT		GPIOC
#define PROTOBOARD_GREEN_LED_PIN		GPIO_PIN_NO_2
#define PROTOBOARD_GREEN_LED_ON			led_on(PROTOBOARD_GREEN_LED_PORT,PROTOBOARD_GREEN_LED_PIN)
#define PROTOBOARD_GREEN_LED_OFF		led_off(PROTOBOARD_GREEN_LED_PORT,PROTOBOARD_GREEN_LED_PIN)

#define PROTOBOARD_BLUE_LED_PORT		GPIOA
#define PROTOBOARD_BLUE_LED_PIN			GPIO_PIN_NO_0
#define PROTOBOARD_BLUE_LED_ON			led_on(PROTOBOARD_BLUE_LED_PORT,PROTOBOARD_BLUE_LED_PIN)
#define PROTOBOARD_BLUE_LED_OFF			led_off(PROTOBOARD_BLUE_LED_PORT,PROTOBOARD_BLUE_LED_PIN)

#define PROTOBOARD_RED_LED_PORT			GPIOA
#define PROTOBOARD_RED_LED_PIN			GPIO_PIN_NO_2
#define PROTOBOARD_RED_LED_ON			led_on(PROTOBOARD_RED_LED_PORT,PROTOBOARD_RED_LED_PIN)
#define PROTOBOARD_RED_LED_OFF			led_off(PROTOBOARD_RED_LED_PORT,PROTOBOARD_RED_LED_PIN)

/*
*@MVM_ACCELERATION
*speed control
*/
#define MVB_PLAYER_INITIAL_SPEED		0
#define MVB_PLAYER_BASE_ACCELERATION	1
#define MVB_PLAYER_BASE_DECELERATION	0.1
#define MVB_PLAYER_MAX_SPEED			5
#define MVB_balloon_BASE_SPEED			1
#define MVB_needle_BASE_SPEED			3

#define MVB_needle_LIFESPAN				30

#define MVB_balloon_SIZE_L				0
#define MVB_balloon_SIZE_M				1

#define MVB_HEADING_DIR_N				1
#define MVB_HEADING_DIR_S				2
#define MVB_HEADING_DIR_E				3
#define MVB_HEADING_DIR_W				4
#define MVB_HEADING_DIR_NE				5
#define MVB_HEADING_DIR_NW				6
#define MVB_HEADING_DIR_SE				7
#define MVB_HEADING_DIR_SW				8

#define MVB_balloon_BMP_W	 			49
#define MVB_balloon_BMP_H				41
#define MVB_balloon_MEDIUM_BMP_W		27
#define MVB_balloon_MEDIUM_BMP_H		25

#define MVB_PLAYER_mouse_BMP_W1			39
#define MVB_PLAYER_mouse_BMP_H1			39

#define MVB_PLAYER_mouse_BMP_W2			41
#define MVB_PLAYER_mouse_BMP_H2			41

#define MVB_needle_BMP_W1				13
#define MVB_needle_BMP_H1				22

#define MVB_needle_BMP_W2				22
#define MVB_needle_BMP_H2				22

#define MVB_needle_N_START_X			(MVB_PLAYER_mouse_BMP_W1)/2 - 5
#define MVB_needle_N_START_Y			-MVB_needle_BMP_H1
#define MVB_needle_S_START_X			(MVB_PLAYER_mouse_BMP_W1)/2 	- 5
#define MVB_needle_S_START_Y			MVB_PLAYER_mouse_BMP_H1
#define MVB_needle_E_START_X			MVB_PLAYER_mouse_BMP_W1
#define MVB_needle_E_START_Y			(MVB_PLAYER_mouse_BMP_H1)/2 - 5
#define MVB_needle_W_START_X			-MVB_needle_BMP_H1
#define MVB_needle_W_START_Y			(MVB_PLAYER_mouse_BMP_H1)/2 - 5

#define MVB_needle_NE_START_X			MVB_PLAYER_mouse_BMP_W2
#define MVB_needle_NE_START_Y			-MVB_needle_BMP_H2
#define MVB_needle_SE_START_X			MVB_PLAYER_mouse_BMP_W2
#define MVB_needle_SE_START_Y			MVB_PLAYER_mouse_BMP_H2
#define MVB_needle_NW_START_X			-MVB_needle_BMP_W2
#define MVB_needle_NW_START_Y			-MVB_needle_BMP_H2
#define MVB_needle_SW_START_X			-MVB_needle_BMP_W2
#define MVB_needle_SW_START_Y			MVB_PLAYER_mouse_BMP_H2

#define MVB_ALIVE_FALSE					2
#define MVB_ALIVE_TRUE 					1
#define MVB_ALIVE_UNSET 				0

#define MVB_DEAD_OBJECT_UNCLEARED		0
#define MVB_DEAD_OBJECT_CLEARED			1

/*
*@MVM_COLLISION
*Collision return bool
*/
#define MVB_COLLISION_TRUE 				1
#define MVB_COLLISION_FALSE 			0

#define MVB_FIRST_TIME_TRUE 			1
#define MVB_FIRST_TIME_FALSE			0

#define MVB_balloon_BUFFER_SIZE 		15
#define MVB_needle_BUFFER_SIZE			3

#define MVB_MARGIN 						0

#define MVB_NUM_OF_WAVE					5

typedef struct{
	int16_t x;
	int16_t y;
	double dx;
	double dy;
	uint8_t headingDir;
	uint8_t aliveFlag;
	uint8_t lifeSpan;
	uint8_t balloonSize;
}Object_Property_t;

typedef struct{
	uint8_t imageWidth;
	uint8_t imageHeight;
	const uint8_t *image;
	uint8_t clearWhenDead;
}Object_Image_t;

typedef struct{
	Object_Property_t Object_Property;
	Object_Image_t Object_Image;
}Object_t;

/***********************************************************************
Private function prototype
***********************************************************************/

/**
*@brief 	If space object is out of bound, this function wraps around the position of the space object
*@param 	Current x position of the object
*@param 	Current y position of the object
*@return 	None
*/
void MVB_wrap_cordinate (int16_t *xPtr, int16_t *yPtr);

/**
*@brief 	Updates player mouse's direction in space
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_update_player_mouse_direction (Object_t *PlayermousePtr);

/**
*@brief 	Updates player mouse's position in space
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_update_player_mouse_position (Object_t *PlayermousePtr);

/**
*@brief 	Delete needle that has collided with balloon
*@param 	needle Object_t structure pointer
*@return 	None
*/
void MVB_delete_dead_needle (Object_t *needlePtr);

/**
*@brief 	Delete balloon that has collided with needle
*@param 	Balloon Object_t structure pointer
*@return 	None
*/
void MVB_delete_dead_balloon (Object_t *balloonPtr);

/**
*@brief 	Detect collision of objects using AABB algorithm
*@param 	Object 1 Object_t structure pointer
*@param 	Object 2 Object_t structure pointer
*@return 	@MVM_COLLISION
*/
uint8_t MVB_collision_detect (Object_t *Object1Ptr, Object_t *Object2Ptr);

/**
*@brief 	Accelerate player mouse
*@param 	Acceleration in x axis @MVM_ACCELERATION
*@param 	Acceleration in y axis @MVM_ACCELERATION
*@return 	None
*/
void MVB_accelerate_player_mouse (Object_t *PlayermousePtr, int8_t ddx, int8_t ddy);

/**
*@brief 	Generates random number between 0 and width of screen
*@param 	None
*@return 	None
*/
int16_t MVB_random_x (void);

/**
*@brief 	Generates random number between 0 and height of screen
*@param 	None
*@return 	None
*/
int16_t MVB_random_y (void);

/**
*@brief 	Generates random number to generate -1 or 1
*@param 	None
*@return 	None
*/
int8_t MVB_random_sign (void);

/***********************************************************************
Function prototype
***********************************************************************/
/**
*@brief 	Game engine initialization (clock, timer, adc, vector and gpio)
*@param 	None
*@return 	None
*/
void MVB_init (void);

/**
*@brief 	Starts timer 6 to update frame
*@param 	None
*@return 	None
*/
void MVB_start_update_frame (void);

/**
*@brief 	Create player mouse (Fill data into player mouse structure)
*@param 	Object_t pointer
*@return 	None
*/
void MVB_create_player_mouse (Object_t *PlayermousePtr);

/**
*@brief 	Create balloon (Fill data into elements in array of balloon structure, then add to balloon vector)
*@param 	Balloon vector structure pointer
*@param 	Balloon Object_t structure pointer
*@param 	Number of balloons to create
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_create_balloon (vector *balloonVectPtr,Object_t *balloonPtr, uint8_t numberToCreate, Object_t *PlayermousePtr);

/**
*@brief 	Create needle (Fill data into elements in array of needle structure, then add to needle vector)
*@param 	Needle vector structure pointer
*@param 	Needle Object_t structure pointer
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_create_needle (vector *needleVectPtr, Object_t *needlePtr, Object_t *PlayermousePtr);

/**
*@brief 	Draw player mouse
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_draw_player_mouse (Object_t *PlayermousePtr);

/**
*@brief 	Draw balloon
*@param 	Balloon vector structure pointer
*@return 	None
*/
void MVB_draw_balloon (vector *balloonVectPtr);

/**
*@brief 	Draw needle
*@param 	Needle vector structure pointer
*@return 	None
*/
void MVB_draw_needle (vector *needleVectPtr);

/**
*@brief 	Update player mouse's data
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_update_player_mouse (Object_t *PlayermousePtr);

/**
*@brief 	Update balloon's data
*@param 	Balloon vector structure pointer
*@param 	Mouse Object_t structure pointer
*@return 	None
*/
void MVB_update_balloon (vector *balloonVectPtr, Object_t *PlayermousePtr);

/**
*@brief 	Update needle's data
*@param 	Needle vector structure pointer
*@param 	Balloon vector structure pointer
*@return 	None
*/
void MVB_update_needle (vector *needleVectPtr, vector *balloonVectPtr);

/**
*@brief 	Fill display with black color
*@param 	None
*@return 	None
*/
void MVB_display_black_background(void);

/**
*@brief 	Display start screen
*@param 	None
*@return 	None
*/
void MVB_display_start_screen(void);

/**
*@brief 	Display player score
*@param 	None
*@return 	None
*/
void MVB_display_score(void);

/**
*@brief 	Display game over screen
*@param 	None
*@return 	None
*/
void MVB_display_game_over_screen(void);

/**
*@brief 	Reset game
*@param 	None
*@return 	None
*/
void MVB_reset_game(void);

/***********************************************************************
External function prototype from ili9341.c
***********************************************************************/
extern ILI9341_Config_t ILI9341_config;
extern void ILI9341_send_command (uint8_t cmd);
extern void ILI9341_send_parameter_16_bits (uint16_t param);
extern void ILI9341_set_active_area (uint16_t startColum, uint16_t startPage, uint16_t endColumn, uint16_t endPage);



#endif
