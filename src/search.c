#include <chess/search.h>
#include <chess/board.h>
#include <chess/evaluate.h>
#include <chess/tree.h>

#include <stdlib.h>

int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Move* out_move, Node* out_node)
{
  int best_eval = maximising_player ? -INT_MAX : INT_MAX;
  Board new_board = board;
  Move* moves;
  size_t nmoves = 0;
  board_get_moves_all(board, &moves, &nmoves,
                      maximising_player ? GetMovesWhite : GetMovesBlack);
  if (nmoves == 0)
  {
    if (out_node)
      out_node->value = best_eval;
    free(moves);
    return best_eval;
  }

  if (depth == 0)
  {
    best_eval = evaluate_board(board);
    if (out_node)
      out_node->value = best_eval;
    free(moves);
    return best_eval;
  }

  int best_eval_i = 0;

  for (size_t i = 0; i < nmoves; i++)
  {
    Node* new_node = NULL;
    if (out_node)
      new_node = node_new(out_node, moves[i], !maximising_player);

    board_update(&new_board, &(moves[i]));
    int eval =
        minimax(new_board, depth - 1, alpha, beta, !maximising_player, NULL, new_node);
    new_board = board; // Restore board state after trying a move
    int last_best_eval = best_eval;
    best_eval = maximising_player ? max(best_eval, eval) : min(best_eval, eval);
    if (best_eval != last_best_eval)
    {
      best_eval_i = i;
      if (out_move)
        *out_move = moves[i];
    }
    if (maximising_player)
    {
      alpha = max(alpha, eval);
    }
    else
      beta = min(beta, eval);
    if (beta <= alpha)
      break;
  }
  free(moves);
  if (out_node)
  {
    out_node->value = best_eval;
    out_node->best_child = best_eval_i;
  }
  return best_eval;
}
