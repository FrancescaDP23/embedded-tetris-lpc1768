/****************************************Copyright (c)****************************************************
**
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h"
#include "TouchPanel/TouchPanel.h"
#include "button_EXINT/button.h"
#include "timer/timer.h"
#include "tetrominos/tetrominos.h"
#include "input/input.h"
#include "adc/adc.h"
#include "music/music.h"
#include <string.h>


#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif


int main(void)
{
  SystemInit();  														/* System Initialization (i.e., PLL)  */
	LCD_Initialization();

	Tetris_Init();

	ADC_init();

	music_init();

	init_timer(0, 0x17D7840);   // 1 secondo
	//init_timer(0, 0x005F5E10);   // velocità simulatore
	enable_timer(0);

	NVIC_SetPriority(RIT_IRQn, 0);       // RIT = Massima priorità
  NVIC_SetPriority(TIMER0_IRQn, 1);
  NVIC_SetPriority(TIMER1_IRQn, 2);

	init_RIT(50000);
	enable_RIT();

	while (1) {
		if(game_tick){
			game_tick = 0;
			updateGameLogic();
		}

		if(input_events){
			uint8_t ev = input_events;
			input_events = 0;

			if(ev & EV_LEFT){
				moveLeft();
			}

			if(ev & EV_RIGHT){
				moveRight();
			}

			if(ev & EV_ROTATE){
				rotatePiece();
			}

			if(ev & EV_HARDDROP){
				hardDrop();
			}

			if(ev & EV_KEY1){
				music_cmd = 1;
				handleKey1();
			}

		}
		if(slow_down_timer == 0){
			updateSystemSpeed();
		}
  }
}


/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
