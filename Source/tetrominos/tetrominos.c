#include "tetrominos.h"
#include "GLCD/GLCD.h"
#include "timer/timer.h"
#include "LPC17xx.h"
#include "input/input.h"
#include "music/music.h"
#include <string.h>
#include <stdio.h>

#define NUM_PIECES 7
#define NUM_ROTATIONS 4

#define MIN_SPEED_MR0 25000000
#define MAX_SPEED_MR0 2500000

uint16_t x = 10;
uint16_t y = 30;
uint16_t gridWidth = 10;
uint16_t gridHeight = 20;

uint16_t blockSize = 12;
FallingPiece fallingPiece;
uint16_t field[ROWS][COLS] = {0};
GameState gameState = GAME_STOPPED;

int score = 0;
int high_score = 0;
int lines_cleared = 0;
int lines_since_malus = 0;

volatile int slow_down_timer = 0; // Contatore per i 15 secondi
volatile uint16_t current_adc_value = 0;
volatile uint8_t game_tick = 0;

char PIECES[NUM_PIECES][NUM_ROTATIONS][PIECE_BLOCK_SIZE][PIECE_BLOCK_SIZE] = {
	// I
	{
			{{0,0,0,0},  {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, 	// Rotazione 0 (partenza)
			 {{0,0,1,0}, {0,0,1,0}, {0,0,1,0}, {0,0,1,0}}, 	// Rotazione 1
			 {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, 	// Rotazione 2
			 {{0,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,1,0,0}} 	// Rotazione 4
	},

	// O
	{
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}
    },
    // J
    {
				{{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,1,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {1,1,0,0}, {0,0,0,0}}
    },
    // L
    {
				{{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,1,0}, {1,0,0,0}, {0,0,0,0}},
        {{1,1,0,0}, {0,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // T
    {
				{{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
				{{0,0,0,0}, {1,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // S
    {
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,1,0,0}, {0,1,1,0}, {0,0,1,0}, {0,0,0,0}},
        {{0,0,0,0}, {0,1,1,0}, {1,1,0,0}, {0,0,0,0}},
        {{1,0,0,0}, {1,1,0,0}, {0,1,0,0}, {0,0,0,0}}
    },
    // Z
    {
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
        {{0,0,1,0}, {0,1,1,0}, {0,1,0,0}, {0,0,0,0}},
        {{0,0,0,0}, {1,1,0,0}, {0,1,1,0}, {0,0,0,0}},
        {{0,1,0,0}, {1,1,0,0}, {1,0,0,0}, {0,0,0,0}}
    }
};

int random_int(int max) {
    return rand() % max;
}

void getPieceChars(char piece[PIECE_BLOCK_SIZE][PIECE_BLOCK_SIZE], FallingPiece* fp) {
    int i, j;
    for(i=0; i<PIECE_BLOCK_SIZE; i++) {
        for(j=0; j<PIECE_BLOCK_SIZE; j++) {
            piece[i][j] = PIECES[fp->type][fp->rotation][i][j];
        }
    }
}

uint16_t getTetrominoColor(int type){
	switch(type){
		case I_PIECE:
			return Cyan;
		case O_PIECE:
			return Yellow;
		case T_PIECE:
			return Magenta;
		case J_PIECE:
			return Blue;
		case L_PIECE:
			return Orange;
		case S_PIECE:
			return Green;
		case Z_PIECE:
			return Red;
		case BLOCK_POWERUP_O:
			return White;
		case BLOCK_POWERUP_SLOW:
			return Grey;
		case BLOCK_MALUS:
			return Purple;
		default:
			return White;
	}
}

void drawPieceSpecific(FallingPiece* fallingPiece, uint16_t color) {
    char shape[PIECE_BLOCK_SIZE][PIECE_BLOCK_SIZE];
    int r, c, blockCount = 0;

		int isDrawing = (color != Black);

    getPieceChars(shape, fallingPiece);

    for(r=0; r<PIECE_BLOCK_SIZE; r++) {
        for(c=0; c<PIECE_BLOCK_SIZE; c++) {
            if(shape[r][c] != 0) {
                int screenX = x + (fallingPiece->x + c) * blockSize;
                int screenY = y + (fallingPiece->y + r) * blockSize;

								uint16_t blockColor = color;

								if(isDrawing && fallingPiece->powerupType != 0 && blockCount == fallingPiece->powerupBlockIndex){
									blockColor = getTetrominoColor(fallingPiece->powerupType);
								}

                if((fallingPiece->y + r) >= 0) {
                    LCD_DrawCell(1, screenX, screenY, blockSize, blockColor);
                }
								blockCount++;
            }
        }
    }
}

void drawTetrominos(FallingPiece* fallingPiece){
	uint16_t tetrominoColor = getTetrominoColor(fallingPiece->type);
	drawPieceSpecific(fallingPiece, tetrominoColor);
}

void deleteTetromino(FallingPiece* fallingPiece){
	drawPieceSpecific(fallingPiece, Black);
}

void RedrawField() {
    int r, c;

    for(r = 0; r < ROWS; r++) {
        for(c = 0; c < COLS; c++) {
            int screenX = x + c * blockSize;
            int screenY = y + r * blockSize;
            if(field[r][c] != 0) {
								uint16_t color;
								if(field[r][c] >= 10){
									color = getTetrominoColor(field[r][c]);
								}else{
									color = getTetrominoColor((PieceType)(field[r][c] - 1));
								}
                LCD_DrawCell(1, screenX, screenY, blockSize, color);
            } else {
                LCD_DrawCell(0, screenX, screenY, blockSize, Black);
            }
        }
    }
}

// GIOCO
void updateScores(void){
	char buffer[20];

	sprintf(buffer, "%d", high_score);
	GUI_Text(167, 68, (uint8_t *)"    ", White, Black);
	GUI_Text(167, 68, (uint8_t *)buffer, White, Black);

	sprintf(buffer, "%d", score);
	GUI_Text(167, 135, (uint8_t *)"    ", White, Black);
	GUI_Text(167, 135, (uint8_t *)buffer, White, Black);

	sprintf(buffer, "%d", lines_cleared);
	GUI_Text(167, 215, (uint8_t *)"    ", White, Black);
	GUI_Text(167, 215, (uint8_t *)buffer, White, Black);

}

void drawStatusMessages(void){
	GUI_Text(20, 290, (uint8_t *)"                              ", White, Black);

    switch(gameState){
        case GAME_PAUSED:
            GUI_Text(20, 290, (uint8_t *)"PAUSED  | KEY1 to Start ", Magenta, Black);
            break;

        case GAME_RUNNING:
            GUI_Text(20, 290, (uint8_t *)"PLAYING | KEY1 to Pause", Magenta, Black);
            break;

        case GAME_OVER:
            GUI_Text(20, 290, (uint8_t *)"GAME OVER | KEY1 to Restart", Red, Black);
            break;

        default:
            break;
    }
}

void Tetris_Init(void){
	int i, j;

	for(i= 0; i<ROWS; i++){
		for(j =0; j<COLS; j++){
			field[i][j] = 0;
		}
	}
		LCD_Clear(Black);
		GUI_Text(90, 7, (uint8_t *)"TETRIS", Magenta, Black);
		gameState = GAME_PAUSED;
		score = 0;
		lines_cleared = 0;
		//LCD_DrawBorder(x, y, blockSize, gridWidth, gridHeight);
		uint16_t widthpx = (COLS * blockSize) + 2;
		uint16_t heightpx = (ROWS * blockSize) + 2;

		LCD_DrawLine(x - 1, y - 1, x - 1 + widthpx, y - 1, White);
		LCD_DrawLine(x - 1, y - 1 + heightpx, x - 1 + widthpx, y - 1 + heightpx, White);
		LCD_DrawLine(x - 1, y - 1, x - 1, y - 1 + heightpx, White);
		LCD_DrawLine(x - 1 + widthpx, y - 1, x - 1 + widthpx, y - 1 + heightpx, White);

		RedrawField();

		LCD_DrawRect(160, 30, 75, 60, White);
		LCD_DrawRect(160, 105, 75, 60, White);
		LCD_DrawRect(160, 180, 75, 60, White);

		GUI_Text(171, 35 , (uint8_t *)"Highest", White, Black);
		GUI_Text(171, 50 , (uint8_t *)"Score", White, Black);
		GUI_Text(171, 115, (uint8_t *)"Score", White, Black);
		GUI_Text(171, 195, (uint8_t *)"Lines", White, Black);

		updateScores();
		// reset input e timer
		joystick_down = 0;
		input_events = 0;

		generateRandomTetromino(&fallingPiece);

		drawStatusMessages();
	}


void generateRandomTetromino(FallingPiece* fallingPiece){
	int randomType = random_int(7);

	fallingPiece->type = randomType;
	fallingPiece->rotation = 0;
	fallingPiece->x = 4;
	fallingPiece->y = 0;

	int chance = random_int(100);
    if(chance < 10) {
        fallingPiece->powerupType = BLOCK_POWERUP_O;
        fallingPiece->powerupBlockIndex = random_int(4); // Uno dei 4 blocchi è il powerup
    } else if (chance < 20) {
        fallingPiece->powerupType = BLOCK_POWERUP_SLOW;
        fallingPiece->powerupBlockIndex = random_int(4);
    } else {
        fallingPiece->powerupType = 0;
        fallingPiece->powerupBlockIndex = -1;
    }
}

int checkCollision(FallingPiece* fp) {
    char shape[PIECE_BLOCK_SIZE][PIECE_BLOCK_SIZE];
    int r, c;
    getPieceChars(shape, fp);

    for(r=0; r<PIECE_BLOCK_SIZE; r++) {
        for(c=0; c<PIECE_BLOCK_SIZE; c++) {
            if(shape[r][c] != 0) {
                int boardX = fp->x + c;
                int boardY = fp->y + r;

                // Collisione bordi
                if (boardX < 0 || boardX >= COLS || boardY >= ROWS) return 1;

                // Collisione con blocchi esistenti (ignora se sopra il campo)
                if (boardY >= 0 && field[boardY][boardX] != 0) return 1;
            }
        }
    }
    return 0;
}

void checkLinesAndScore() {
    int r, c, k, j;
    int lines_found = 0;
	  int powerup_o_triggered = 0;
	  int powerup_slow_triggered = 0;

    for(r = ROWS - 1; r >= 0; r--) {
        int full = 1;
			  int has_powerup_o = 0;
			  int has_powerup_slow = 0;

        for(c = 0; c < COLS; c++) {
            if(field[r][c] == 0) {
							full = 0;
						  break;
						}
						if(field[r][c] == BLOCK_POWERUP_O) has_powerup_o = 1;
            if(field[r][c] == BLOCK_POWERUP_SLOW) has_powerup_slow = 1;
        }

        if(full) {
					  if(has_powerup_o) powerup_o_triggered = 1;
            if(has_powerup_slow) powerup_slow_triggered = 1;

            lines_found++;

            for(k = r; k > 0; k--) {
                for(j = 0; j < COLS; j++) {
									field[k][j] = field[k-1][j];
								}
            }
            for(j = 0; j < COLS; j++) {
							field[0][j] = 0;
            }
            r++;
        }
    }

    if(lines_found > 0) {
        lines_cleared += lines_found;
			  lines_since_malus += lines_found; // aggiorna il contatore malus

			  if(lines_found >= 4){
					score += 600;
				}else{
					score += (lines_found * 100);
						if(score > high_score){
							high_score = score;
						}
				}
        RedrawField();

				// attivazione Powerups
        if(powerup_o_triggered) {
            activateHalfClear();
            updateScores();
        }

        if(powerup_slow_triggered) {
            slow_down_timer = 300;
            // Forza velocit� normale
            LPC_TIM0->MR0 = TIMER_NORMAL_SPEED;
            LPC_TIM0->TC = 0;
        }

        // attivazione Malus
        while(lines_since_malus >= 10) {
            activateMalus();
            lines_since_malus -= 10;
        }
    }
				updateScores();
}

void placeTetromino(FallingPiece* fallingPiece){
	play_tick_sound();

	char tShape[PIECE_BLOCK_SIZE][PIECE_BLOCK_SIZE];
	int row, col, blockCount = 0;
	getPieceChars(tShape, fallingPiece);

	for(row = 0; row < PIECE_BLOCK_SIZE; row++){
		for(col = 0; col< PIECE_BLOCK_SIZE; col++){
			if(tShape[row][col] != 0){
				int boardX = fallingPiece->x + col;
				int boardY = fallingPiece->y + row;

				if(boardY >= 0 && boardY < ROWS && boardX >= 0 && boardX < COLS){
					if(fallingPiece->powerupType != 0 && blockCount == fallingPiece->powerupBlockIndex){
						field[boardY][boardX] = fallingPiece->powerupType;
					}else{
						field[boardY][boardX] = fallingPiece->type + 1;
					}
				}
				blockCount++;
			}
		}
	}
	score = score + 10;
	if(score > high_score){
		high_score = score;
	}
	checkLinesAndScore();
}

void updateGameLogic(void){
	if(gameState != GAME_RUNNING) return;

	deleteTetromino(&fallingPiece);
	fallingPiece.y++;

	if(checkCollision(&fallingPiece)){
		fallingPiece.y--;
		drawTetrominos(&fallingPiece);
		placeTetromino(&fallingPiece);

		generateRandomTetromino(&fallingPiece);

		if(checkCollision(&fallingPiece)){
			gameState = GAME_OVER;
			music_cmd = 2;
			drawStatusMessages();
		}else{
			drawTetrominos(&fallingPiece);
		}
	}else{
		drawTetrominos(&fallingPiece);
}
}


void moveLeft(void){
	if(gameState != GAME_RUNNING){
		return;
	}
	deleteTetromino(&fallingPiece);
	fallingPiece.x--;
	if(checkCollision(&fallingPiece)){
		fallingPiece.x++;
	}
	drawTetrominos(&fallingPiece);
}


void moveRight(void){
	if(gameState != GAME_RUNNING){
		return;
	}
	deleteTetromino(&fallingPiece);
	fallingPiece.x++;
	if(checkCollision(&fallingPiece)){
		fallingPiece.x--;
	}
	drawTetrominos(&fallingPiece);
}


void rotatePiece(void){
	if(gameState != GAME_RUNNING){
		return;
	}
	deleteTetromino(&fallingPiece);
	int old = fallingPiece.rotation;
	fallingPiece.rotation = (fallingPiece.rotation + 1) % 4;
	if(checkCollision(&fallingPiece)){
		fallingPiece.rotation = old;
	}
	drawTetrominos(&fallingPiece);
}

void hardDrop(void){
	if(gameState != GAME_RUNNING) return;

	deleteTetromino(&fallingPiece);
	do{
		fallingPiece.y++;
	}while(checkCollision(&fallingPiece) != 1);

	fallingPiece.y--;

	drawTetrominos(&fallingPiece);
	placeTetromino(&fallingPiece);

	generateRandomTetromino(&fallingPiece);

	if(checkCollision(&fallingPiece)){
		gameState = GAME_OVER;
		music_cmd = 2;
		drawStatusMessages();
	}else{
		drawTetrominos(&fallingPiece);
	}
}

void handleKey1(){
    static int last_state = -1;

    if(gameState == last_state) return;
    last_state = gameState;

    if(gameState == GAME_RUNNING){
        gameState = GAME_PAUSED;
    } else if(gameState == GAME_PAUSED){
        gameState = GAME_RUNNING;
        drawTetrominos(&fallingPiece);
    } else {
        Tetris_Init();
        return;
    }

    drawStatusMessages();

		if(gameState == GAME_PAUSED){
			music_cmd = 3;
		}else if(gameState == GAME_RUNNING){
			music_cmd = 1;
			enable_timer(2);
			enable_timer(3);
		}
}


void GPIO_Input_Init(void){
    // Joystick - GPIO1
    LPC_PINCON->PINMODE3 &= ~(3 << 20); // P1.26 DOWN
    LPC_PINCON->PINMODE3 &= ~(3 << 22); // P1.27 LEFT
    LPC_PINCON->PINMODE3 &= ~(3 << 24); // P1.28 RIGHT
    LPC_PINCON->PINMODE3 &= ~(3 << 26); // P1.29 UP

    // Buttons - GPIO2
    LPC_PINCON->PINMODE4 &= ~(3 << 22); // P2.11 KEY1
    LPC_PINCON->PINMODE4 &= ~(3 << 24); // P2.12 KEY2
}


void activateHalfClear(){
    int r, c;
    int start_row = ROWS / 2;
    int lines_removed = 0;

    // Cancella le righe dalla metà in giù
    for(r = ROWS - 1; r >= start_row; r--) {
        int has_blocks = 0;
        for(c=0; c<COLS; c++) {
            if(field[r][c] != 0) has_blocks = 1;
            field[r][c] = 0;
        }
        if(has_blocks) lines_removed++;
    }

    if(lines_removed > 0){
        if(lines_removed <= 4){
            score += (lines_removed * 100);
        } else {
            int groups = lines_removed / 4;
            int remainder = lines_removed % 4;
            score += (groups * 600);
            score += (remainder * 100);
        }
        if(score > high_score) high_score = score;
    }

    // Spostare tutto giù di (ROWS - start_row)
    int shift_amount = ROWS - start_row;
    for(r = start_row - 1; r >= 0; r--) {
        for(c = 0; c < COLS; c++) {
            field[r + shift_amount][c] = field[r][c];
            field[r][c] = 0;
        }
    }
    RedrawField();
}


void activateMalus(){
    int r, c;
    // Controllo Game Over (se la riga 0 ha blocchi)
    for(c=0; c<COLS; c++) {
        if(field[0][c] != 0) {
            gameState = GAME_OVER;
            drawStatusMessages();
            return;
        }
    }

    for(r = 0; r < ROWS - 1; r++) {
        for(c = 0; c < COLS; c++) {
            field[r][c] = field[r+1][c];
        }
    }

    // Genera riga Malus in fondo
    for(c = 0; c < COLS; c++) field[ROWS-1][c] = 0; // Reset

    // 7 blocchi random
    int blocks_placed = 0;
    while(blocks_placed < 7) {
        int pos = random_int(COLS);
        if(field[ROWS-1][pos] == 0) {
            field[ROWS-1][pos] = BLOCK_MALUS; // Nuovo ID per colore viola
            blocks_placed++;
        }
    }
    RedrawField();
}

void updateSystemSpeed(void){
    uint32_t new_MR0;

    if(joystick_down){
        new_MR0 = 0x002625A0;
    }
    else {
        if(slow_down_timer > 0){
             // power-up rallentamento attivo
             new_MR0 = 0x17D7840; // 1 Secondo (25MHz)
        }
        else {
             // lettura ADC
             uint32_t reduction = (uint32_t)current_adc_value * 4884;

             if (reduction > 20000000) reduction = 20000000;
             
             new_MR0 = 25000000 - reduction;

             // Limite di sicurezza
             if(new_MR0 < 1000000) new_MR0 = 1000000;
        }
    }

    if(LPC_TIM0->MR0 != new_MR0){
        LPC_TIM0->MR0 = new_MR0;

        if(LPC_TIM0->TC > new_MR0){
            LPC_TIM0->TC = 0;
        }
    }
}
