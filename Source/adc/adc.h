#ifndef ADC_H
#define ADC_H
#include "LPC17xx.h"
#include <string.h>

extern volatile uint16_t ADC_Value;
extern volatile float volume_scale;

/* lib_adc.c */
void ADC_init (void);
void ADC_start_conversion (void);

/* IRQ_adc.c */
void ADC_IRQHandler(void);

#endif