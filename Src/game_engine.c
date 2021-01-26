/*
 * game_engine.c
 *
 *  Created on: June 8, 2020
 *      Author: manish
 *
 */

#include "game_engine.h"
#include "bitmap_byte_array.h"

uint8_t frameUpdate = CLEAR;
int16_t score = 0;
int16_t scorePrevious = 0;
char displayScore[15];

uint8_t shootButtonFirstTimeFlag = MVB_FIRST_TIME_TRUE;

Object_t Playermouse;
Object_t balloon[MVB_balloon_BUFFER_SIZE] ;
Object_t needle[MVB_needle_BUFFER_SIZE];

vector balloonVect;
vector needleVect;

uint8_t currentWave = 0;
uint8_t numOfballoonInWave[MVB_NUM_OF_WAVE] = {1,2,3,4,5};

void MVB_init (void)
{
	//Clock configuration: HCLK = 150Mhz, APB2 = 75Mhz, APB1 = 37.5Mhz
	RCC_set_SYSCLK_PLL_150_MHz();
	
	RNG_init();
	
	ILI9341_init();
	ILI9341_rotate(ILI9341_orientation_landscape_2);
 	ILI9341_fill_display(ILI9341_BLACK);
	
	joystick_init(JOYSTICK_ADC,JOYSTICK_X_ADC_CHANNEL,JOYSTICK_Y_ADC_CHANNEL);

	button_init(SHOOT_BUTTON_PORT,SHOOT_BUTTON_PIN,GPIO_PU);
	button_init(THRUST_BUTTON_PORT,THRUST_BUTTON_PIN,GPIO_PU);
	
	led_init(PROTOBOARD_GREEN_LED_PORT,PROTOBOARD_GREEN_LED_PIN);
	led_init(PROTOBOARD_BLUE_LED_PORT,PROTOBOARD_BLUE_LED_PIN);
	led_init(PROTOBOARD_RED_LED_PORT,PROTOBOARD_RED_LED_PIN);
	
	//configure timer 6 to generate periodic interrupt of 33ms (screen refresh rate 30Hz)
	TIM_init_direct(TIM6,1875,1319);
	TIM_intrpt_vector_ctr(IRQ_TIM6_DAC,ENABLE);
	TIM_interrupt_ctr(TIM6,ENABLE);
	
	//configure timer 3 to generate 20ms counter (timer  3 is used for button debouncing)
	TIM_init_direct(TIM3,1875,799);

	//make timer 3 have lower priority than timer 6
	TIM_intrpt_priority_config(IRQ_TIM3, 55);

	vector_init(&balloonVect);
	vector_init(&needleVect);

}

//Starts timer 6 to generate periodic interrupt of 33ms to update frame
void MVB_start_update_frame (void)
{
	TIM_ctr(TIM6,START);
}

void MVB_create_player_mouse (Object_t *PlayermousePtr)
{
	PlayermousePtr->Object_Property.x = MVB_random_x();
	PlayermousePtr->Object_Property.y = MVB_random_y();
	
	PlayermousePtr->Object_Property.dx = 0;
	PlayermousePtr->Object_Property.dy = 0;
	
	PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_N;
	
	PlayermousePtr->Object_Property.aliveFlag = MVB_ALIVE_TRUE;
	PlayermousePtr->Object_Property.lifeSpan = 0;
	
	PlayermousePtr->Object_Image.image = player_mouse_north_bmp;
	PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H1;
	PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W1;
}

void MVB_create_balloon (vector *balloonVectPtr,Object_t *balloonPtr, uint8_t numberToCreate, Object_t *PlayermousePtr)
{
	Object_t *PreviousballoonPtr = NULL;
	uint8_t Collision_with_mouse = MVB_COLLISION_FALSE, Collision_with_other_balloons = MVB_COLLISION_FALSE;

	for (uint8_t count = 0; count < numberToCreate; count++,balloonPtr++){

		//store address of balloon to be created in balloon vector
		vector_add(balloonVectPtr,(void*)balloonPtr);

		balloonPtr->Object_Image.image = balloon_bmp;
		balloonPtr->Object_Image.imageHeight = MVB_balloon_BMP_H;
		balloonPtr->Object_Image.imageWidth = MVB_balloon_BMP_W;
		balloonPtr->Object_Image.clearWhenDead = MVB_DEAD_OBJECT_UNCLEARED;

		//keep randomizing balloon 's position until balloon being generated is not colliding with player and also with other balloons
		do{
			Collision_with_mouse = MVB_COLLISION_FALSE;
			Collision_with_other_balloons = MVB_COLLISION_FALSE;

			balloonPtr->Object_Property.x = MVB_random_x();
			balloonPtr->Object_Property.y = MVB_random_y();

			Collision_with_mouse = MVB_collision_detect(balloonPtr,PlayermousePtr);

			if(Collision_with_mouse == MVB_COLLISION_TRUE){
				continue;
			}

			//if first balloon is being created, no need to check for collision with other balloon
			if(count == 0){
				break;
			}

			//for generation of next balloon, collision need to be checked with previous balloons
			PreviousballoonPtr = balloonPtr-1;

			for(uint8_t i = 0; i< count;i++,PreviousballoonPtr--){
				Collision_with_other_balloons = MVB_collision_detect(balloonPtr,PreviousballoonPtr);

				if(Collision_with_other_balloons == MVB_COLLISION_TRUE){
					break;
				}
			}

		}while((Collision_with_mouse == MVB_COLLISION_TRUE) || (Collision_with_other_balloons == MVB_COLLISION_TRUE ));

		/* Give random speed and direction for balloon*/
		balloonPtr->Object_Property.dx = MVB_random_sign()*MVB_balloon_BASE_SPEED;
		balloonPtr->Object_Property.dy = MVB_random_sign()*MVB_balloon_BASE_SPEED;

		balloonPtr->Object_Property.aliveFlag = MVB_ALIVE_TRUE;
		balloonPtr->Object_Property.lifeSpan = 0;
		balloonPtr->Object_Property.balloonSize = MVB_balloon_SIZE_L;
	}
}

void MVB_create_needle (vector *needleVectPtr, Object_t *needlePtr, Object_t *PlayermousePtr)
{
	if (!SHOOT_BUTTON_READ){
		//wait till TIM3 counts to 20ms for button debounce
		TIM_ctr(TIM3,START);
		while(TIM_counter_status(TIM3));

		PROTOBOARD_RED_LED_ON;

		scorePrevious = score;
		score--;

		Object_t *head = needlePtr;

		//do not create more needle if there is already 3 active needles
		while((needlePtr->Object_Property.aliveFlag != MVB_ALIVE_FALSE)	&& (needlePtr->Object_Property.aliveFlag != MVB_ALIVE_UNSET)){
			needlePtr ++;
			if(needlePtr - head > (MVB_needle_BUFFER_SIZE-1)){
				return;
			}
		}

		vector_add(needleVectPtr,needlePtr);

		needlePtr->Object_Property.aliveFlag = MVB_ALIVE_TRUE;
		needlePtr->Object_Property.lifeSpan = MVB_needle_LIFESPAN;
		needlePtr->Object_Image.clearWhenDead = MVB_DEAD_OBJECT_UNCLEARED;

		if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_N){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_N_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_N_START_Y;

			needlePtr->Object_Property.dx = 0;
			needlePtr->Object_Property.dy = -MVB_needle_BASE_SPEED;

			needlePtr->Object_Image.image = needle_north_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_W1;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_H1;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_S){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_S_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_S_START_Y;

			needlePtr->Object_Property.dx =	0;
			needlePtr->Object_Property.dy = MVB_needle_BASE_SPEED;

			needlePtr->Object_Image.image = needle_south_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_W1;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_H1;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_E){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_E_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_E_START_Y;

			needlePtr->Object_Property.dx = MVB_needle_BASE_SPEED;
			needlePtr->Object_Property.dy = 0;

			needlePtr->Object_Image.image = needle_east_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_H1;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_W1;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_W){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_W_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_W_START_Y;

			needlePtr->Object_Property.dx = -MVB_needle_BASE_SPEED;
			needlePtr->Object_Property.dy = 0;

			needlePtr->Object_Image.image = needle_west_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_H1;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_W1;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_NE){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_NE_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_NE_START_Y;

			needlePtr->Object_Property.dx = MVB_needle_BASE_SPEED;
			needlePtr->Object_Property.dy = -MVB_needle_BASE_SPEED;

			needlePtr->Object_Image.image = needle_north_east_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_W2;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_H2;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_SE){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_SE_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_SE_START_Y;

			needlePtr->Object_Property.dx = MVB_needle_BASE_SPEED;
			needlePtr->Object_Property.dy = MVB_needle_BASE_SPEED;

			needlePtr->Object_Image.image = needle_south_east_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_W2;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_H2;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_NW){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_NW_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_NW_START_Y;

			needlePtr->Object_Property.dx = -MVB_needle_BASE_SPEED;
			needlePtr->Object_Property.dy = -MVB_needle_BASE_SPEED;

			needlePtr->Object_Image.image = needle_north_west_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_W2;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_H2;

		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_SW){

			needlePtr->Object_Property.x = PlayermousePtr->Object_Property.x + MVB_needle_SW_START_X;
			needlePtr->Object_Property.y = PlayermousePtr->Object_Property.y + MVB_needle_SW_START_Y;

			needlePtr->Object_Property.dx = -MVB_needle_BASE_SPEED;
			needlePtr->Object_Property.dy = MVB_needle_BASE_SPEED;

			needlePtr->Object_Image.image = needle_south_west_bmp;
			needlePtr->Object_Image.imageWidth = MVB_needle_BMP_W2;
			needlePtr->Object_Image.imageHeight = MVB_needle_BMP_H2;

		}

	}
	else{
		PROTOBOARD_RED_LED_OFF;
	}
}

void MVB_draw_player_mouse (Object_t *PlayermousePtr)
{
	ILI9341_draw_bitmap_w_background(PlayermousePtr->Object_Property.x,PlayermousePtr->Object_Property.y,
	PlayermousePtr->Object_Image.image,PlayermousePtr->Object_Image.imageWidth,
	PlayermousePtr->Object_Image.imageHeight,ILI9341_LIGHTGREY,ILI9341_BLACK);
}

void MVB_draw_balloon (vector *balloonVect)
{
	Object_t *balloonPtr = NULL;
	for(uint8_t count = 0;count < balloonVect->total;count++){
		balloonPtr = vector_get(balloonVect,count);
		ILI9341_draw_bitmap_w_background(balloonPtr->Object_Property.x,balloonPtr->Object_Property.y,
		balloonPtr->Object_Image.image,balloonPtr->Object_Image.imageWidth,
		balloonPtr->Object_Image.imageHeight,0xB3E7,ILI9341_BLACK);
	}
}

void MVB_draw_needle (vector *needleVectPtr)
{
	Object_t  *needlePtr = NULL;
	for (uint8_t count = 0;count < needleVectPtr->total;count++){
		needlePtr = vector_get(needleVectPtr,count);
		ILI9341_draw_bitmap_w_background(needlePtr->Object_Property.x,needlePtr->Object_Property.y,
		needlePtr->Object_Image.image,needlePtr->Object_Image.imageWidth,
		needlePtr->Object_Image.imageHeight,ILI9341_LIGHTGREY,ILI9341_BLACK);
	}
}

void MVB_update_player_mouse (Object_t *PlayermousePtr)
{
	MVB_update_player_mouse_direction (PlayermousePtr);
	MVB_update_player_mouse_position	(PlayermousePtr);
}

void MVB_update_balloon (vector *balloonVectPtr, Object_t *PlayermousePtr)
{
	Object_t *balloonPtr = NULL;
	Object_t *OtherballoonPtr = NULL;

	for(uint8_t count = 0;count < balloonVectPtr->total;count++){

		balloonPtr = vector_get(balloonVectPtr,count);

		//update current balloon 's position
		balloonPtr->Object_Property.x += balloonPtr->Object_Property.dx;
		balloonPtr->Object_Property.y += balloonPtr->Object_Property.dy;
		MVB_wrap_cordinate(&balloonPtr->Object_Property.x,&balloonPtr->Object_Property.y);

		//check for collision with other balloons if there is more than one balloon
		if(balloonVectPtr->total > 1){

			for (uint8_t i = count + 1;i < balloonVectPtr->total;i++){

				OtherballoonPtr = vector_get(balloonVectPtr,i);

				if(OtherballoonPtr->Object_Property.aliveFlag == MVB_ALIVE_TRUE){
					if(MVB_collision_detect(balloonPtr,OtherballoonPtr) == MVB_COLLISION_TRUE){

						//if collided make both balloon travel in opposite direction
						balloonPtr->Object_Property.dx *= -1;
						balloonPtr->Object_Property.dy *= -1;
						OtherballoonPtr->Object_Property.dx *= -1;
						OtherballoonPtr->Object_Property.dy *= -1;

					}
				}
			}
		}

		//check for collision with mouse
		if(MVB_collision_detect(balloonPtr,PlayermousePtr) == MVB_COLLISION_TRUE){

			//if collided mark player mouse as dead and return to main loop
			PlayermousePtr->Object_Property.aliveFlag = MVB_ALIVE_FALSE;

			return;
		}
	}
}

void MVB_update_needle (vector *needleVectPtr, vector *balloonVectPtr)
{
	Object_t *needlePtr = NULL;
	Object_t *balloonPtr = NULL;

	for(int8_t count = 0; count < needleVectPtr->total; count++){

		needlePtr = vector_get(needleVectPtr,count);

		needlePtr->Object_Property.lifeSpan--;

		needlePtr->Object_Property.x += needlePtr->Object_Property.dx;
		needlePtr->Object_Property.y += needlePtr->Object_Property.dy;
		MVB_wrap_cordinate(&needlePtr->Object_Property.x,&needlePtr->Object_Property.y);

		//if life span of needle ran out, mark needle as dead and free from needle vector
		if(!(needlePtr->Object_Property.lifeSpan)){
			needlePtr->Object_Property.aliveFlag = MVB_ALIVE_FALSE;
			MVB_delete_dead_needle(needlePtr);
			vector_delete(needleVectPtr,count);
			count--;
		}

		//if needle hit a balloon, mark both needle and balloon as dead, free current needle from needle vector, balloon from balloon vector
		for(int8_t i = 0;i < balloonVectPtr->total; i++){
			balloonPtr = (Object_t*)vector_get(balloonVectPtr,i);

			if(MVB_collision_detect(needlePtr,balloonPtr) == MVB_COLLISION_TRUE){

				scorePrevious = score;
				score += 10;

				needlePtr->Object_Property.aliveFlag = MVB_ALIVE_FALSE;
				MVB_delete_dead_needle(needlePtr);
				vector_delete(needleVectPtr,count);
				count--;
				balloonPtr->Object_Property.aliveFlag = MVB_ALIVE_FALSE;
				MVB_delete_dead_balloon(balloonPtr);
				vector_delete(balloonVectPtr,i);
				i--;

			}

		}
	}
}

void MVB_display_score (void)
{
	char displayScore[15];
	sprintf(displayScore,"Score: %d",scorePrevious);

	//if score is updated, clear old score before printing new score to screen
	if(scorePrevious |= score){
		ILI9341_put_string(240,0,displayScore,&TM_Font_7x10,ILI9341_BLACK);
		sprintf(displayScore,"Score: %d",score);
		ILI9341_put_string(240,0,displayScore,&TM_Font_7x10,ILI9341_YELLOW);
	}else{
		ILI9341_put_string(240,0,displayScore,&TM_Font_7x10,ILI9341_YELLOW);
	}
}

void MVB_display_start_screen(void)
{
	MVB_display_black_background();
	ILI9341_put_string(20,120,"Press shoot button to start",&TM_Font_11x18,ILI9341_RED);
}

void MVB_display_game_over_screen(void)
{
	MVB_display_black_background();
	ILI9341_draw_bitmap(48,0,dead_mouse_bmp,225,225,ILI9341_YELLOW);
	ILI9341_put_string(10,200,"        Mouse died!               Want to try again?",&TM_Font_11x18,ILI9341_RED);
}

void MVB_reset_game(void)
{
	score = 0;
	frameUpdate = CLEAR;
	RNG_init();
	TIM_ctr(TIM6,STOP);
	currentWave = 0;

	for(uint8_t count = 0; count < balloonVect.total;){
		vector_delete(&balloonVect,count);
	}

	for(uint8_t count = 0; count < needleVect.total;){
		vector_delete(&needleVect,count);
	}

	for(uint8_t count = 0;count < MVB_balloon_BUFFER_SIZE;count++){
		balloon[count].Object_Property.aliveFlag = MVB_ALIVE_FALSE;
	}

	for(uint8_t count = 0;count < MVB_needle_BUFFER_SIZE;count++){
		needle[count].Object_Property.aliveFlag = MVB_ALIVE_FALSE;
	}
}

//if space object is out of bound, it is wraped around the display
void MVB_wrap_cordinate (int16_t *xPtr, int16_t *yPtr)
{
	if (*xPtr < -40){
		*xPtr += ILI9341_config.width;
	}
	
	if (*xPtr >=  ILI9341_config.width){
		*xPtr -= ILI9341_config.width;
	}

	if (*yPtr < -40){
		*yPtr += ILI9341_config.height;
	}
	
	if (*yPtr >=  ILI9341_config.height){
		*yPtr -= ILI9341_config.height;
	}
}

int16_t MVB_random_x (void)
{
	return (RNG_get() & 0x1FF) % (ILI9341_config.width +1);
}

int16_t MVB_random_y (void)
{
	return (RNG_get() & 0xFF) % (ILI9341_config.height +1);
}

int8_t MVB_random_sign (void)
{
	uint8_t temp = (RNG_get() & 0x0F) % 9;
	if(temp < 5){
		return -1;
	}
	return 1;
}

void MVB_update_player_mouse_direction (Object_t *PlayermousePtr)
{
	uint8_t direction = 0;
	direction = joystick_read_direction (JOYSTICK_ADC,JOYSTICK_X_ADC_CHANNEL, JOYSTICK_Y_ADC_CHANNEL);
	
	if (direction == JS_DIR_CENTERED){
		return;
	}else if (direction == JS_DIR_UP){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_N;
		
		PlayermousePtr->Object_Image.image = player_mouse_north_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W1;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H1;
		return;
	}else if (direction == JS_DIR_DOWN){
				
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_S;
		
		PlayermousePtr->Object_Image.image = player_mouse_south_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W1;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H1;
		return;
		
	}else if (direction == JS_DIR_RIGHT){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_E;
		
		PlayermousePtr->Object_Image.image = player_mouse_east_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W1;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H1;
		return;
		
	}else if (direction == JS_DIR_LEFT){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_W;
		
		PlayermousePtr->Object_Image.image = player_mouse_west_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W1;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H1;
		return;
		
	}else if (direction == JS_DIR_RIGHT_UP){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_NE;
		
		PlayermousePtr->Object_Image.image = player_mouse_north_east_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W2;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H2;
		return;
		
	}else if (direction == JS_DIR_RIGHT_DOWN){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_SE;
		
		PlayermousePtr->Object_Image.image = player_mouse_south_east_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W2;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H2;
		return;
		
	}else if (direction == JS_DIR_LEFT_UP){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_NW;
		
		PlayermousePtr->Object_Image.image = player_mouse_north_west_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W2;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H2;
		return;
		
	}else if (direction == JS_DIR_LEFT_DOWN){
		
		PlayermousePtr->Object_Property.headingDir = MVB_HEADING_DIR_SW;
		
		PlayermousePtr->Object_Image.image = player_mouse_south_west_bmp;
		PlayermousePtr->Object_Image.imageWidth = MVB_PLAYER_mouse_BMP_W2;
		PlayermousePtr->Object_Image.imageHeight = MVB_PLAYER_mouse_BMP_H2;
		return;
		
	}
}

void MVB_update_player_mouse_position (Object_t *PlayermousePtr)
{
	//if thrust button is pressed, start incrementing speed. Once released, decrement speed.
	if(!THRUST_BUTTON_READ){
		
		int8_t ddx = 0;
		int8_t ddy = 0;

		PROTOBOARD_BLUE_LED_ON;
		
		if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_N){
			ddx	= 0;
			ddy = -MVB_PLAYER_BASE_ACCELERATION;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_S){
			ddx += 0;
			ddy += MVB_PLAYER_BASE_ACCELERATION;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_E){
			ddx += MVB_PLAYER_BASE_ACCELERATION;
			ddy += 0;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_W){
			ddx += -MVB_PLAYER_BASE_ACCELERATION;
			ddy += 0;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_NE){
			ddx += MVB_PLAYER_BASE_ACCELERATION;
			ddy += -MVB_PLAYER_BASE_ACCELERATION;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_NW){
			ddx += -MVB_PLAYER_BASE_ACCELERATION;
			ddy += -MVB_PLAYER_BASE_ACCELERATION;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_SE){
			ddx += MVB_PLAYER_BASE_ACCELERATION;
			ddy += MVB_PLAYER_BASE_ACCELERATION;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}else if(PlayermousePtr->Object_Property.headingDir == MVB_HEADING_DIR_SW){
			ddx += -MVB_PLAYER_BASE_ACCELERATION;
			ddy += MVB_PLAYER_BASE_ACCELERATION;
			MVB_accelerate_player_mouse(PlayermousePtr,ddx,ddy);
		}
	
	}else{
		
		PROTOBOARD_BLUE_LED_OFF;

		PlayermousePtr->Object_Property.x += PlayermousePtr->Object_Property.dx;
		PlayermousePtr->Object_Property.y += PlayermousePtr->Object_Property.dy;
		MVB_wrap_cordinate(&PlayermousePtr->Object_Property.x,&PlayermousePtr->Object_Property.y);
		
		if(fabs(PlayermousePtr->Object_Property.dx) > MVB_PLAYER_BASE_DECELERATION){
			
			if(PlayermousePtr->Object_Property.dx > 0){
				PlayermousePtr->Object_Property.dx -= MVB_PLAYER_BASE_DECELERATION;
			}else if(PlayermousePtr->Object_Property.dx < 0){
				PlayermousePtr->Object_Property.dx += MVB_PLAYER_BASE_DECELERATION;
			}
		}else{
			PlayermousePtr->Object_Property.dx = 0;
		}
		
		if(fabs(PlayermousePtr->Object_Property.dy) > MVB_PLAYER_BASE_DECELERATION){
						
			if(PlayermousePtr->Object_Property.dy > 0){
				PlayermousePtr->Object_Property.dy -= MVB_PLAYER_BASE_DECELERATION;
			}else if(PlayermousePtr->Object_Property.dy < 0){
				PlayermousePtr->Object_Property.dy += MVB_PLAYER_BASE_DECELERATION;
			}
		}else{
			PlayermousePtr->Object_Property.dy = 0;
		}
	
	}
	
}

// Detect collision between 2 objects using AABB algorithm
uint8_t MVB_collision_detect (Object_t *Object1Ptr, Object_t *Object2Ptr)
{
	int16_t Obj1BottomRight_X = Object1Ptr->Object_Property.x + (Object1Ptr->Object_Image.imageWidth - 10);
	int16_t Obj1BottomRight_Y = Object1Ptr->Object_Property.y + (Object1Ptr->Object_Image.imageHeight - 10);
	
	int16_t Obj2BottomRight_X = Object2Ptr->Object_Property.x + (Object2Ptr->Object_Image.imageWidth - 10);
	int16_t Obj2BottomRight_Y = Object2Ptr->Object_Property.y + (Object2Ptr->Object_Image.imageHeight - 10);
	
	if (Object1Ptr->Object_Property.x < Obj2BottomRight_X 
		&& Object2Ptr->Object_Property.x < Obj1BottomRight_X
		&& Object1Ptr->Object_Property.y < Obj2BottomRight_Y
		&& Object2Ptr->Object_Property.y < Obj1BottomRight_Y){
						
		return MVB_COLLISION_TRUE;
	}	

	return MVB_COLLISION_FALSE;
}

// draw black image over object to be deleted
void MVB_delete_dead_needle (Object_t *needlePtr)
{
		if((needlePtr->Object_Property.aliveFlag == MVB_ALIVE_FALSE) && (needlePtr->Object_Image.clearWhenDead == MVB_DEAD_OBJECT_UNCLEARED)){
			ILI9341_draw_bitmap_w_background(needlePtr->Object_Property.x,needlePtr->Object_Property.y,
			needlePtr->Object_Image.image,needlePtr->Object_Image.imageWidth,
			needlePtr->Object_Image.imageHeight,ILI9341_BLACK,ILI9341_BLACK);
			needlePtr->Object_Image.clearWhenDead = MVB_DEAD_OBJECT_CLEARED;
		}
}

// draw black image over object to be deleted
void MVB_delete_dead_balloon (Object_t *balloonPtr)
{
		if((balloonPtr->Object_Property.aliveFlag == MVB_ALIVE_FALSE) && (balloonPtr->Object_Image.clearWhenDead == MVB_DEAD_OBJECT_UNCLEARED)){
			ILI9341_draw_bitmap_w_background(balloonPtr->Object_Property.x,balloonPtr->Object_Property.y,
			balloonPtr->Object_Image.image,balloonPtr->Object_Image.imageWidth,
			balloonPtr->Object_Image.imageHeight,ILI9341_BLACK,ILI9341_BLACK);
			balloonPtr->Object_Image.clearWhenDead = MVB_DEAD_OBJECT_CLEARED;
		}
}

void MVB_accelerate_player_mouse (Object_t *PlayermousePtr, int8_t ddx, int8_t ddy)
{
	if(fabs(PlayermousePtr->Object_Property.dx) < MVB_PLAYER_MAX_SPEED){
		PlayermousePtr->Object_Property.dx += ddx;
	}

	if(fabs(PlayermousePtr->Object_Property.dy) < MVB_PLAYER_MAX_SPEED){
		PlayermousePtr->Object_Property.dy += ddy;
	}
}

void MVB_display_black_background(void)
{
	ILI9341_fill_display(ILI9341_BLACK);
}

void TIM6_DAC_IRQHandler (void)
{
	TIM_intrpt_handler(TIM6);
	frameUpdate = SET;
}
