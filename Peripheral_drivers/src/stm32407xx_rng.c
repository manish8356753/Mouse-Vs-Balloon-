/*
 *@note RNG 's clock (RNG_CLK) must satisfy the following condition: RNG_CLK >= HCLK/16
 *		RNG_CLK is derived from PLL clock: PLL_CLK/PLLQ (PLLQ is division factor which value can be program in RCC_PLLCFGR)
 *		In order for RNG to be correctly configured, user need to call RCC_set_SYSCLK_PLL_150_MHz before enabling RNG
 */

#include "stm32f407xx_rng.h"

uint32_t RNG_random_value = 0; 

void RNG_CLK_ctr(uint8_t enOrDis)
{
	if(enOrDis == ENABLE){
		RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;
	}else if (enOrDis == DISABLE){
		RCC->AHB2ENR &= ~RCC_AHB2ENR_RNGEN;
	}
}

void RNG_periph_ctr(uint8_t enOrDis)
{
	if(enOrDis == ENABLE){
		RNG->CR |= RNG_CR_RNGEN;
	}else if (enOrDis == DISABLE){
		RNG->CR &= ~RNG_CR_RNGEN;
	}	
}	

void RNG_init(void)
{
	RNG_CLK_ctr(ENABLE);
	RNG_periph_ctr(ENABLE);
}

void RNG_deinit(void)
{
	RCC->AHB2RSTR |= RCC_AHB2RSTR_RNGRST;
	RCC->AHB2RSTR &= ~RCC_AHB2RSTR_RNGRST;
};

uint32_t RNG_get(void)
{
	while(!(RNG->SR & RNG_SR_DRDY));
	return RNG->DR;
}

void RNG_intrpt_ctr(uint8_t enOrDis)
{
	if(enOrDis == ENABLE){
		RNG->CR |= RNG_CR_IE;
	}else if (enOrDis == ENABLE){
		RNG->CR &= ~RNG_CR_IE;	
	}
}

void RNG_intrpt_vector_ctr(uint8_t IRQnumber, uint8_t enOrDis)
{
		if(enOrDis == ENABLE){
			if(IRQnumber <= 31){
				NVIC->ISER[0] |= (1<<IRQnumber);
			}
			else if(IRQnumber > 31 && IRQnumber <= 63){
				NVIC->ISER[1] |= (1<<(IRQnumber%32));
			}
			else if(IRQnumber > 63 && IRQnumber <= 95){
				NVIC->ISER[2] |= (1<<(IRQnumber%64));
			}
		}else{
			if(IRQnumber <= 31){
				NVIC->ICER[0] |= (1<<IRQnumber);
			}
			else if(IRQnumber > 31 && IRQnumber <= 63){
				NVIC->ICER[1] |= (1<<(IRQnumber%32));
			}
			else if(IRQnumber > 63 && IRQnumber <= 95){
				NVIC->ICER[2] |= (1<<(IRQnumber%64));
			}
		}
}

void RNG_intrpt_priority_config(uint8_t IRQnumber, uint8_t priority)
{
	uint8_t registerNo = IRQnumber/4;
	uint8_t section = IRQnumber%4;
	
	NVIC->IP[registerNo] &= ~(0xFF << (8*section));
	NVIC->IP[registerNo] |= (priority << (8*section + NUM_OF_IPR_BIT_IMPLEMENTED));
}

void RNG_intrpt_handler (void)
{
	uint8_t check1 = (RNG->CR & RNG_CR_IE) >> RNG_CR_IE_Pos;
	uint8_t check2 = (RNG->SR & RNG_SR_DRDY) >> RNG_SR_DRDY_Pos;
	
	//case interrupt triggered due to data is ready
	if(check1 & check2){
		RNG_random_value = RNG->DR;
	}
	
	check1 = (RNG->CR & RNG_CR_IE) >> RNG_CR_IE_Pos;
	check2 = (RNG->SR & RNG_SR_CEIS) >> RNG_SR_CEIS_Pos;
	
	//case interrupt triggered due to clock is not correctly detected
	if(check1 & check2){
		RNG->SR &= ~RNG_SR_CEIS;
	}
}
