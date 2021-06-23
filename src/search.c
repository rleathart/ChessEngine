#include "search.h"
#include "board.h"
#include "evaluate.h"

#include <stdlib.h>

int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Move* out_move)
{
  int best_eval = maximising_player ? -INT_MAX : INT_MAX;
  Board new_board = board;
  Move* moves;
  size_t nmoves = 0;
  board_get_moves_all(board, &moves, &nmoves,
                      maximising_player ? GetMovesWhite : GetMovesBlack);
  if (nmoves == 0)
    return best_eval;

  if (depth == 0)
    return evaluate_board(board);

  for (size_t i = 0; i < nmoves; i++)
  {
    board_update(&new_board, &(moves[i]));
    int eval =
        minimax(new_board, depth - 1, alpha, beta, !maximising_player, NULL);
    new_board = board; // Restore board state after trying a move
    int last_best_eval = best_eval;
    best_eval = maximising_player ? max(best_eval, eval) : min(best_eval, eval);
    if (maximising_player)
    {
      alpha = max(alpha, eval);
      if (best_eval > last_best_eval)
        if (out_move)
          *out_move = moves[i];
    }
    else
    {
      beta = min(beta, eval);
      if (best_eval < last_best_eval)
        if (out_move)
          *out_move = moves[i];
    }
    if (beta <= alpha)
      break;
  }
  free(moves);
  return best_eval;
}
