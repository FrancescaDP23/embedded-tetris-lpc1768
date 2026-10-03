/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_adc.c
** Last modified Date:  20184-12-30
** Last Version:        V1.00
** Descriptions:        functions to manage A/D interrupts
** Correlated files:    adc.h
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "LPC17xx.h"
#include "adc.h"
#include "../timer/timer.h"
#include "tetrominos/tetrominos.h"

/*----------------------------------------------------------------------------
  A/D IRQ: Executed when A/D Conversion is ready (signal from ADC peripheral)
 *----------------------------------------------------------------------------*/


void ADC_IRQHandler(void) {
	
	uint32_t regVal;
	
	regVal = LPC_ADC->ADSTAT;
	
	if(regVal & (1<<5)){
		uint16_t val = (LPC_ADC->ADDR5 >> 4) & 0xFFF;
		
		current_adc_value = val;
	}
  		
}
