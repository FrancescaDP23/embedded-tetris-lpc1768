#ifndef TETROMINO_H
#define TETROMINO_H

#include "GLCD/GLCD.h"

#define PIECE_BLOCK_SIZE 4
#define ROWS 20
#define COLS 10


typedef enum{
	GAME_STOPPED = 0,
	GAME_RUNNING,
	GAME_PAUSED,
	GAME_OVER
}GameState;

typedef enum {
	I_PIECE = 0,
	O_PIECE,
	J_PIECE,
	L_PIECE,
	T_PIECE,
	S_PIECE,
	Z_PIECE	
} PieceType;


typedef struct {
	PieceType type;
	int rotation;
	int x, y;
} FallingPiece;

extern FallingPiece fallingPiece;
extern uint16_t field[ROWS][COLS];
extern uint16_t blockSize;
extern GameState gameState;
extern int score;
extern int high_score;
extern int lines_cleared;
extern volatile uint8_t game_tick;

void Tetris_Init(void);
void generateRandomTetromino(FallingPiece* fallingPiece);
void updateGameLogic(void);

void drawTetrominos(FallingPiece* fallingPiece);
void deleteTetromino(FallingPiece* fallingPiece);
void getPieceChars(char piece[PIECE_BLOCK_SIZE][PIECE_BLOCK_SIZE], FallingPiece* fallingPiece);

void placeTetromino(FallingPiece* fallingPiece);
void deleteTetrominos(FallingPiece* fallingPiece);

void moveLeft(void);
void moveRight(void);
void rotatePiece(void);
void hardDrop(void);
void softDrop(int active);
void handleKey1(void);
void GPIO_Input_Init(void);
#endif