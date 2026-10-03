#include "LPC17xx.h"
#include "RIT.h"
#include "tetrominos/tetrominos.h"
#include "GLCD/GLCD.h"
#include "input/input.h"
#include "adc/adc.h"
#include "music/music.h"

#define DEBOUNCE_TICKS 10
#define UPTICKS 1

NOTE tetris_theme[] = {
    {e5, time_semiminima}, {b4, time_croma}, {c5, time_croma}, {d5, time_semiminima},
    {c5, time_croma}, {b4, time_croma},
    {a4, time_semiminima}, {a4, time_croma}, {c5, time_croma}, {e5, time_semiminima},
    {d5, time_croma}, {c5, time_croma},
    {b4, time_semiminima}, {b4, time_croma}, {c5, time_croma}, {d5, time_semiminima},
    {e5, time_semiminima},
    {c5, time_semiminima}, {a4, time_semiminima}, {a4, time_semiminima},
    {pause, time_croma},
    {d5, time_semiminima}, {f5, time_croma}, {a5, time_semiminima}, {g5, time_croma}, {f5, time_croma},
    {e5, time_semiminima}, {e5, time_croma}, {c5, time_croma}, {e5, time_semiminima}, {d5, time_croma}, {c5, time_croma},
    {b4, time_semiminima}, {b4, time_croma}, {c5, time_croma}, {d5, time_semiminima}, {e5, time_semiminima},
    {c5, time_semiminima}, {a4, time_semiminima}, {a4, time_semiminima},

    {pause, time_semiminima}
};

NOTE game_over_theme[] = {
    {e5, time_croma}, {pause, time_semicroma},
    {d5, time_croma}, {pause, time_semicroma},
    {c5, time_croma}, {pause, time_semicroma},
    {b4, time_semibreve}
};

volatile int joystick_down = 0;

void RIT_IRQHandler (void)
{
    static int currentNote = 0;
    static int ticks = 0;
    static int last_cmd = -1;

    // Gestione cambio stato musica
    if(music_cmd != last_cmd) {
        currentNote = 0;
        ticks = 0;
        last_cmd = music_cmd;

        if(music_cmd == 0 || music_cmd == 3){
             disable_timer(2);
             disable_timer(3);
        }
    }
    if(music_cmd == 1 || music_cmd == 2)
    {

        if(!isNotePlaying())
        {
            ++ticks;
            if(ticks >= UPTICKS)
            {
                ticks = 0;

                if(music_cmd == 1) // TETRIS
                {
                    playNote(tetris_theme[currentNote++]);
                    if(currentNote >= (sizeof(tetris_theme)/sizeof(NOTE))) {
                        currentNote = 0; // Loop infinito
                    }
                }
                else if(music_cmd == 2) // GAME OVER
                {
                    if(currentNote < (sizeof(game_over_theme)/sizeof(NOTE))) {
                        playNote(game_over_theme[currentNote++]);
                    }
                }
            }
        }
    }

    static int left = 0, left_cnt = 0;
    static int right = 0, right_cnt = 0;
    static int up = 0, up_cnt = 0;
    static int KEY1_down = 0, key1_cnt = 0;
    static int KEY2_down = 0, key2_cnt = 0;

    // Left
    if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){
        if (left_cnt < DEBOUNCE_TICKS) left_cnt++;
        if (left_cnt == DEBOUNCE_TICKS && left == 0){
            left = 1;
            input_events |= EV_LEFT;
        }
    } else { left=0; left_cnt=0; }

    // Right
    if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){
        if (right_cnt < DEBOUNCE_TICKS) right_cnt++;
        if (right_cnt == DEBOUNCE_TICKS && right == 0){
            right = 1;
            input_events |= EV_RIGHT;
        }
    } else { right=0; right_cnt=0; }

    // Joystick Down - Soft Drop
    if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){
        joystick_down = 1;
    } else {
        joystick_down = 0;
    }

    // Up (Rotate)
    if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){
        if (up_cnt < DEBOUNCE_TICKS) up_cnt++;
        if (up_cnt == DEBOUNCE_TICKS && up == 0){
            up = 1;
            input_events |= EV_ROTATE;
        }
    } else { up=0; up_cnt=0; }

    // KEY1 (Pause/Start)
    if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){
        if(key1_cnt < DEBOUNCE_TICKS) key1_cnt++;
        if(key1_cnt == DEBOUNCE_TICKS && KEY1_down == 0){
            KEY1_down = 1;
            input_events |= EV_KEY1;
        }
    } else { KEY1_down=0; key1_cnt=0; }

    // KEY2 (Hard Drop)
    if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){
        if(key2_cnt < DEBOUNCE_TICKS) key2_cnt++;
        if(key2_cnt == DEBOUNCE_TICKS && KEY2_down == 0){
            KEY2_down = 1;
            input_events |= EV_HARDDROP;
        }
    } else { KEY2_down = 0; key2_cnt = 0; }

    // Timer rallentamento (Powerup)
    if(slow_down_timer > 0){
        slow_down_timer--;
    }

    LPC_RIT->RICTRL |= 0x1;    /* clear interrupt flag */
}
