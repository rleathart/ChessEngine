#include <rgl/logging.h>

#include <chess/search.h>
#include <chess/board.h>
#include <chess/move.h>
#include <chess/evaluate.h>
#include <chess/tree.h>

#include <stdlib.h>
#include <math.h>

/// @param out_node non-null
int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Node* out_node)
{
  // We don't want to print anything inside minimax
  DebugLevel old_debug_level = t_debug_level_get();
  t_debug_level_set(DebugLevelNone);
  int best_eval = maximising_player ? -INT_MAX : INT_MAX;
  Board new_board = board;
  Array moves = board_get_moves_all(board,
                      maximising_player ? GetMovesWhite : GetMovesBlack);

  int rv = best_eval;

  if (moves.used == 0) // Either checkmate or stalemate
  {
    if (!is_in_check(board, maximising_player)) // Stalemate
      best_eval = 0;

    goto end;
  }

  if (depth == 0)
  {
    best_eval = evaluate_board(board);
    goto end;
  }

  int best_eval_i = 0;

  for (size_t i = 0; i < moves.capacity; i++)
  {
    if (!array_index_is_allocated(&moves, i))
      continue;
    Node* new_node = node_new(out_node, *(Move*)array_get(&moves, i), !maximising_player);

    board_update(&new_board, array_get(&moves, i));
    int eval =
        minimax(new_board, depth - 1, alpha, beta, !maximising_player, new_node);
    new_board = board; // Restore board state after trying a move

    int last_best_eval = best_eval;
    best_eval = maximising_player ? fmax(best_eval, eval) : fmin(best_eval, eval);

    if (best_eval != last_best_eval)
      best_eval_i = i;

    if (maximising_player)
      alpha = fmax(alpha, eval);
    else
      beta = fmin(beta, eval);

    if (beta <= alpha) // Prune
      break;
  }

end:
  array_free(&moves);

  out_node->value = best_eval;
  out_node->best_child = best_eval_i;

  t_debug_level_set(old_debug_level);
  return best_eval;
}

// @@Rework merging precomputation may cause this to break. Consider taking a
// struct as an argument to handle that. or _Thread_local :D
Move search(Tree* tree)
{
  static Node* checkmate_lines = NULL;
  static Node* current_node = NULL;
  static bool can_force_mate = false;
  Move best_move;

  if (!can_force_mate)
  {
    int value = minimax(tree->board, tree->depth, -INT_MAX, INT_MAX,
        tree->root->isWhite, tree->root);
    best_move = node_get_best_move(*tree->root);
    if (value == -INT_MAX)
    {
      can_force_mate = true;
      // deep copy tree with root at the chosen move
      Node* best_child = tree->root->children[tree->root->best_child];
      checkmate_lines = node_copy(*best_child);
      current_node = checkmate_lines;
    }
  }
  else
  {
    for (int i = 0; i < current_node->nchilds; i++)
    {
      if (move_equals(tree->root->move,
          current_node->children[i]->move))
        current_node = current_node->children[i];
    }
    current_node =
      current_node->children[current_node->best_child];
    best_move = current_node->move;
    if (current_node->nchilds <=0)
    {
      current_node = NULL;
      can_force_mate = false;
      node_free(&checkmate_lines);
    }
  }

  return best_move;
}
