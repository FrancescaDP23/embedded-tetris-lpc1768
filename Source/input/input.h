#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

// Eventi di input
typedef enum {
	EV_NONE = 0,
	EV_LEFT = 1 << 0,
	EV_RIGHT = 1 << 1,
	EV_ROTATE = 1 << 2,
	//EV_SOFTDROP = 1 << 3,
	EV_HARDDROP = 1 << 4,
	EV_KEY1 = 1 << 5
}input_event_t;

extern volatile uint8_t input_events;  // bitmask degli eventi attivi
extern volatile int joystick_down;    // stato del joystick ( 1 = premuto, 0 = rilasciato)
#endif 