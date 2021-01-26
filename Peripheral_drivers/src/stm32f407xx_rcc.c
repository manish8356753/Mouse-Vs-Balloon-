

#include "stm32f407xx_rcc.h"
#include "stm32f407xx.h"

void RCC_MCO2_config (uint8_t option, uint8_t prescaler)
{
	//select clock signal
	RCC->CFGR &= ~(RCC_CFGR_MCO2);
	RCC->CFGR |= option << RCC_CFGR_MCO2_Pos;
	
	//configure prescaler
	RCC->CFGR &= ~(RCC_CFGR_MCO2PRE);
	RCC->CFGR |= prescaler << RCC_CFGR_MCO2PRE_Pos;
	
	//set up PC9 as MCO2 function
	GPIO_init_direct(GPIOC,GPIO_PIN_NO_9,GPIO_MODE_ALTFN,GPIO_OUTPUT_LOW_SPEED,GPIO_OUTPUT_TYPE_PP,GPIO_NO_PUPDR,0);
}

void RCC_MCO1_config (uint8_t option, uint8_t prescaler)
{
	//select clock signal
	RCC->CFGR &= ~(RCC_CFGR_MCO1);
	RCC->CFGR |= option << RCC_CFGR_MCO1_Pos;
	
	//configure prescaler
	RCC->CFGR &= ~(RCC_CFGR_MCO1PRE);
	RCC->CFGR |= prescaler << RCC_CFGR_MCO1PRE_Pos;
	
	//set up PA8 as MCO2 function
	GPIO_init_direct(GPIOA,GPIO_PIN_NO_8,GPIO_MODE_ALTFN,GPIO_OUTPUT_LOW_SPEED,GPIO_OUTPUT_TYPE_PP,GPIO_NO_PUPDR,0);
}

void RCC_set_SYSCLK_HSE (void)
{
	//turn on HSE
	RCC_HSE_clock_ctrl (ENABLE);
	
	//select HSE as system clock
	RCC->CFGR &= ~(RCC_CFGR_SW);
	RCC->CFGR |= RCC_SYSCLK_HSE << RCC_CFGR_SW_Pos;
	
	while (((RCC->CFGR & RCC_CFGR_SWS) >> RCC_CFGR_SWS_Pos) != RCC_SYSCLK_HSE);
	
	//turn of HSI
	RCC_HSI_clock_ctrl (DISABLE);
}

//Set system clock as PLL 150 MHz, APB2 = 75Mhz, APB1 = 37.5Mhz
void RCC_set_SYSCLK_PLL_150_MHz (void)
{	

	//set flash latency to 5 wait state
	FLASH_set_latency();

	//configure AHB division factor as 1, APB2 division factor as 2, APB1 division factor as 4
	RCC->CFGR &= ~(RCC_CFGR_HPRE);
	
	RCC->CFGR &= ~(RCC_CFGR_PPRE2);
	RCC->CFGR |= RCC_APB_DIV2 << RCC_CFGR_PPRE2_Pos;
	
	RCC->CFGR &= ~(RCC_CFGR_PPRE1);
	RCC->CFGR |= RCC_APB_DIV4 << RCC_CFGR_PPRE1_Pos;
	
	RCC_set_PLL_150_MHz();
		
	//select PLL as system clock
	RCC->CFGR &= ~(RCC_CFGR_SW);
	RCC->CFGR |= RCC_SYSCLK_PLL << RCC_CFGR_SW_Pos;
	
	//wait until system clock source is PLL
	while (((RCC->CFGR & RCC_CFGR_SWS) >> RCC_CFGR_SWS_Pos) != RCC_SYSCLK_PLL);
	
	RCC_HSI_clock_ctrl (DISABLE);
}

int32_t RCC_get_SYSCLK_value (void)
{
	int32_t sysClockVal = 0;
	uint8_t sysClockStatus =	(RCC->CFGR & RCC_CFGR_SWS) >> RCC_CFGR_SWS_Pos;
	
	if(sysClockStatus == 0){
		sysClockVal = 16000000;
	}else if(sysClockStatus == 1){
		sysClockVal = 8000000;
	}else if(sysClockStatus == 2){
		sysClockVal = RCC_get_PLL_output();
	}	
	
	return sysClockVal;
}

//Get APB bus clock value
int32_t RCC_get_PCLK_value(uint8_t APBx)
{
	int32_t sysClock = 0, PCLK = 0;
	uint8_t AHBdiv = 0, APBdiv = 0;
	uint16_t AHBdivArray[] = {2,4,8,16,64,128,256,512};
	uint8_t APBdivArray[]	=	{2,4,8,16};
	uint8_t AHBdivStatus = (RCC->CFGR >> RCC_CFGR_HPRE_Pos)	& 0x0F;
	uint8_t APBdivStatus;
	if(APBx == APB1){
		APBdivStatus = (RCC->CFGR >> RCC_CFGR_PPRE1_Pos)	& 0x07;
	}else{
		APBdivStatus = (RCC->CFGR >> RCC_CFGR_PPRE2_Pos)	& 0x07;
	}
		
	sysClock = RCC_get_SYSCLK_value();
	
	if(sysClock == -1){
		return -1;
	}
	
	//get AHB prescaler
	if(AHBdivStatus <= 7){
		AHBdiv = 1;
	}else{
		AHBdiv = AHBdivArray[AHBdivStatus-8];
	}
	
	//get APB1 prescaler
	if(APBdivStatus <= 3){
		APBdiv = 1;
	}else{
		APBdiv = APBdivArray[APBdivStatus-4];
	}
	
	//derive PCLK
	PCLK = (sysClock/AHBdiv)/APBdiv;
	return PCLK;
}

void RCC_HSI_clock_ctrl (uint8_t enOrDis)
{
	if (enOrDis == ENABLE){
		RCC->CR |= RCC_CR_HSION;
		while (!(RCC->CR & RCC_CR_HSIRDY));
	}else if (enOrDis == DISABLE){
		RCC->CR &= ~RCC_CR_HSION;
	}
}

void RCC_HSE_clock_ctrl (uint8_t enOrDis)
{
	if (enOrDis == ENABLE){
		RCC->CR |= RCC_CR_HSEON;
		while (!(RCC->CR & RCC_CR_HSERDY));
	}else if (enOrDis == DISABLE){
		RCC->CR &= ~RCC_CR_HSEON;
	}
}

void RCC_PLL_clock_ctrl (uint8_t enOrDis)
{
	if (enOrDis == ENABLE){
		RCC->CR |= RCC_CR_PLLON;
		while (!(RCC->CR & RCC_CR_PLLRDY));
	}else if (enOrDis == DISABLE){
		RCC->CR &= ~RCC_CR_PLLON;
	}
}

//set voltage scale mode to mode 1
void PWR_set_scale_mode (void)
{
	PWR->CR |= PWR_CR_VOS;
}

void FLASH_set_latency (void)
{
	FLASH->ACR &= ~(0x07 << FLASH_ACR_LATENCY_Pos);
	FLASH->ACR |= FLASH_ACR_LATENCY_5WS;

}

void RCC_set_PLL_150_MHz (void)
{
	//turn on HSE
	RCC_HSE_clock_ctrl (ENABLE);
	
	//select HSE as PLL input, turn off PLL, and program PLL 's division factors to achieve 150Mhz
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC;
	RCC->CR &= ~RCC_CR_PLLON;
	
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLM);
	RCC->PLLCFGR |= RCC_PLL_150_VCO_INPUT_DIV << RCC_PLLCFGR_PLLM_Pos;

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLN);
	RCC->PLLCFGR |= RCC_PLL_150_VCO_OUTPUT_MUL << RCC_PLLCFGR_PLLN_Pos;

	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLP);
	RCC->PLLCFGR |= RCC_PLL_150_PLL_OUTPUT_DIV << RCC_PLLCFGR_PLLP_Pos;
	
	RCC->PLLCFGR &= ~(RCC_PLLCFGR_PLLQ);
	RCC->PLLCFGR |= RCC_PLL_150_MAIN_PLL_DIV << RCC_PLLCFGR_PLLQ_Pos;
	
	//turn on PLL
	RCC_PLL_clock_ctrl (ENABLE);
}

//Private function:get PLL output value
//@note if return value = -1, PLL configuration is wrong
int32_t RCC_get_PLL_output (void)
{
	uint32_t VCOinput = 0;
	uint8_t VCOinputDiv = 0;
	uint16_t VCOoutputMul = 0;
	uint8_t PLLoutputDiv =	0;
	uint32_t PLLoutput = 0;
	
	//determine PLL input source
	uint8_t check = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> RCC_PLLCFGR_PLLSRC_Pos;
	if (check){
		VCOinput = 8000000;
	}else{
		VCOinput = 16000000;
	}
	
	//determine VCO input division factor
	//VCO input division factor possible value is [2,63]
	VCOinputDiv =  (RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos;
	if(VCOinputDiv <2 || VCOinputDiv > 63){
		return -1;
	}
	
	//determine VCO output multiplication factor
	//VCO input multiplication factor possible value is [50,432]
	VCOoutputMul =  (RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos;
	if(VCOoutputMul <50 || VCOoutputMul > 432){
		return -1;
	}	
	
	//determine PLL output division factor
	uint8_t temp =  (RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos;
	if(!temp){
		PLLoutputDiv = 2;
	}else if(temp == 1){
		PLLoutputDiv = 4;
	}else if(temp == 2){
		PLLoutputDiv = 6;
	}	else if(temp == 3){
		PLLoutputDiv = 8;
	}	
	
	PLLoutput = (((double)(VCOinput / VCOinputDiv))*VCOoutputMul)/PLLoutputDiv; 
	return PLLoutput;
}

//Private function:generate delay
void RCC_delay( volatile uint32_t delay)
{
	while(!delay){
		delay--;
	};
}
