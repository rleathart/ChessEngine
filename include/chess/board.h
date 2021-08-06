#pragma once

#include <rgl/array.h>

#include <chess/defs.h>
#include <chess/move.h>
#include <chess/piece.h>

#include <stdbool.h>

typedef enum
{
  ConsiderChecks = 1 << 0,
} GetMovesFlags;

typedef enum
{
  GetMovesBlack = 1 << 0,
  GetMovesWhite = 1 << 1,
} GetMovesAllFlags;

void board_new(Board* board, char* fen);
void board_new_from_string(Board* board, char* board_str);
void board_update(Board* board, Move* move);
bool board_can_move(ChessPiece piece, Board board, int pos88);
Array board_get_moves(Board _board, int pos, GetMovesFlags flags);
Array board_get_moves_all(Board board, GetMovesAllFlags flags);

char* board_tostring(Board board);
Move* board_calculate_line(Board board, int depth, bool maximising_player);

bool is_in_check(Board board, bool isWhite);
bool is_in_checkmate(Board board, bool isWhite);
bool is_in_stalemate(Board board, bool isWhite);
