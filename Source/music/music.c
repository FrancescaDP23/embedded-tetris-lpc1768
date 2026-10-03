#include "music.h"
#include "../timer/timer.h"
#include "LPC17xx.h"

volatile int music_cmd = 0;

void music_init(void){
	// P0.26 come AOUT(DAC)
	LPC_PINCON->PINSEL1 &= ~(3 << 20);
	LPC_PINCON->PINSEL1 |= (2 << 20);
	
	LPC_SC->PCONP |= (1 << 22);
	LPC_SC->PCONP |= (1 << 23);
	LPC_SC->PCONP |= (1 << 2);
}

void playNote(NOTE note)
{
	// TIMER2 per la frequenza
	if(note.freq != pause)
	{
		uint32_t ticks_frequenza = 25000000 / (2 * note.freq);
		
		reset_timer(2);
		init_timer(2, ticks_frequenza);
		enable_timer(2);
	}
	else
	{
		disable_timer(2);
	}
	// TIMER3 per la durata
	reset_timer(3);
	init_timer(3, note.duration);
	enable_timer(3);
}

BOOL isNotePlaying(void)
{
	// controlla se il Timer 3 sta ancora contando
	return (LPC_TIM3->TCR & 1);
}

void play_tick_sound(void){
	uint32_t ticks = 25000000 / (2*200);
	reset_timer(1);
	init_timer(1, ticks);
	enable_timer(1);
}