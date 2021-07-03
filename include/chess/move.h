#pragma once

#include "defs.h"
#include "board.h"

// New move from 0x88 positions
Move move_new(int from, int to);
char* move_tostring(Move move);
bool move_equals(Move move, Move other);
Move move_get_random(Board board, int flags);
