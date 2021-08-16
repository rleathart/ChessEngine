#include <chess/move.h>
#include <chess/util.h>
#include <chess/piece.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// New move from 0x88 positions
Move move_new(int from, int to)
{
  // We need to use memset here so that the struct padding garbage doesn't break
  // move_equals
  Move move;
  memset(&move, 0, sizeof(move));
  move.from = from;
  move.to = to;
  move.promotion = ChessPieceNone;
  return move;
}

char* move_tostring(Move move)
{
  char* str = calloc(1, 4096);
  sprintf(str, "%02x-%02x", topos88(move.from), topos88(move.to));
  str = realloc(str, strlen(str) + 1);
  return str;
}

bool move_equals(Move move, Move other)
{
  return memcmp(&move, &other, sizeof(Move)) == 0;
}
