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
  Move move = {
      .from = from,
      .to = to,
  };
  return move;
}

char* move_tostring(Move move)
{
  char* str = calloc(1, 4096);
  sprintf(str, "%02x-%02x", topos88(move.from), topos88(move.to));
  str = realloc(str, strlen(str) + 1);
  return str;
}

Move move_get_random(Board board, int flags)
{
  Move rv;
  int pos_64;
  bool whiteMove = flags & GetMovesWhite;
  for (;;)
  {
    pos_64 = randrange(0, 63);
    if (board.state[topos88(pos_64)] == ChessPieceNone)
      continue;
    if (whiteMove)
    {
      if (!(board.state[topos88(pos_64)] & ChessPieceIsWhite))
        continue;
    }
    else
    {
      if (board.state[topos88(pos_64)] & ChessPieceIsWhite)
        continue;
    }

    Move* moves;
    size_t nmoves = 0;
    board_get_moves(board, topos88(pos_64), &moves, &nmoves);
    if (nmoves == 0)
      continue;
    return moves[randrange(0, nmoves - 1)];
  }
}
