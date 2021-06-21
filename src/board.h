#pragma once

#include "defs.h"
#include "move.h"
#include "piece.h"

#include <stdbool.h>

void board_new(Board* board, char* fen);
void board_update(Board* board, Move* move);
bool board_can_move(ChessPiece piece, Board board, int pos88);
void board_get_moves(Board _board, int pos, Move** moves, size_t* nmoves);
void board_get_moves_all(Board board, Move** moves, size_t* nmoves, int flags);

char* board_tostring(Board board);