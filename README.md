# Embedded Tetris on LPC1768

A bare-metal implementation of Tetris for the LandTiger development board, based on the NXP LPC1768 ARM Cortex-M3 microcontroller.

The project was developed as an extra-credit assignment for the **Computer Architectures** examination at the University of Salerno. It applies interrupt-driven programming and direct peripheral control to implement a complete interactive game on embedded hardware.

## Features

- Seven tetromino types with four rotation states
- Collision detection, line clearing, scoring, and high-score tracking
- Joystick movement, rotation, and soft drop
- Button-controlled hard drop, pause, restart, and game state transitions
- Adjustable falling speed through the ADC input
- LCD rendering on a 10 x 20 game board
- Music and sound effects generated through timers and the DAC
- Debounced input handling through the Repetitive Interrupt Timer
- Power-ups that clear part of the board or temporarily slow the game
- Malus rows added after a configured number of cleared lines

## Hardware and peripherals

| Component | Use in the project |
|---|---|
| LPC1768, ARM Cortex-M3 | Main microcontroller |
| GLCD | Game board, status, score, and line counter |
| Joystick | Horizontal movement, rotation, and soft drop |
| Push buttons | Pause, restart, and hard drop |
| ADC | Dynamic control of the falling speed |
| DAC | Music and placement sound effects |
| Timers 0-3 | Game ticks, tone generation, and note duration |
| RIT | Input polling, debouncing, and music sequencing |

## Software architecture

```text
hardware inputs
  joystick, buttons, ADC
          |
          v
interrupt handlers
  RIT, ADC, TIMER0-3
          |
          v
input event flags and game tick
          |
          v
main loop and Tetris state machine
          |
     +----+----+
     |         |
     v         v
GLCD rendering  DAC audio
```

The interrupt handlers perform time-sensitive peripheral work and publish compact event flags. The main loop consumes those events and updates the game state, keeping most game logic outside interrupt context.

## Repository structure

```text
.
├── Source/
│   ├── adc/             # ADC initialization and interrupt handler
│   ├── button_EXINT/    # Push-button support
│   ├── GLCD/            # Display driver and fonts
│   ├── input/           # Shared input-event flags
│   ├── joystick/        # Joystick support
│   ├── music/           # DAC audio and note playback
│   ├── RIT/             # Repetitive Interrupt Timer and debouncing
│   ├── tetrominos/      # Game state, pieces, collision, scoring, power-ups
│   ├── timer/           # Timer configuration and interrupt handlers
│   ├── sample.c         # Application entry point
│   ├── startup_LPC17xx.s
│   └── system_LPC17xx.c
├── RTE/                 # Keil runtime configuration
├── DebugConfig/         # LPC1768 debug configurations
├── sample.uvprojx       # Keil uVision project
└── sample.uvoptx        # Keil target options
```

Generated objects, listing files, build logs, and user-specific uVision settings are excluded from version control.

## Build and run

1. Install Keil MDK with support for the LPC1768 device family.
2. Open `sample.uvprojx` in Keil uVision.
3. Select the `LandTiger_LPC1768 (release)` target for the physical board or `SW_DEBUG` for the configured debug environment.
4. Build the project.
5. Flash the executable to the board and start a debug session.

The project depends on the LandTiger peripheral wiring and the included GLCD, timer, ADC, joystick, and audio support code. It has not been converted to a host application.

## Controls

| Input | Action |
|---|---|
| Joystick left/right | Move the current tetromino |
| Joystick up | Rotate clockwise |
| Joystick down | Soft drop |
| KEY1 | Start, pause, or restart |
| KEY2 | Hard drop |
| ADC input | Adjust the automatic falling speed |

## Academic context

This project demonstrates embedded C programming, memory-mapped I/O, interrupt prioritization, hardware timers, ADC and DAC integration, input debouncing, and state-machine design.
