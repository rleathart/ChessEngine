#include <rgl/logging.h>

#include <chess/board.h>
#include <chess/evaluate.h>
#include <chess/move.h>
#include <chess/search.h>
#include <chess/tree.h>

#include <inttypes.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// Selection sort
void node_order_children(Node* node)
{
  if (node->nchilds == 0)
    return;

  bool order_descending = node->isWhite;
  int i, j, next;
  for (i = 0; i < node->nchilds - 1; i++)
  {
    next = i;
    for (j = i + 1; j < node->nchilds; j++)
    {
      int a = order_descending ? j : next;
      int b = order_descending ? next : j;
      if (node->children[a]->value > node->children[b]->value)
        next = j;
    }
    Node* temp = node->children[i];
    node->children[i] = node->children[next];
    node->children[next] = temp;
    if (node->best_child == i)
      node->best_child = next;
    if (node->best_child == next)
      node->best_child = i;
  }
}

typedef struct
{
  s64 alpha, beta;
  u64 max_depth;
  bool prune;
} MinimaxArgs;

/// @param out_node non-null
int minimax(Board board, u64 depth, bool maximising_player, Node* out_node,
            MinimaxArgs args)
{
  // We don't want to print anything inside minimax
  DebugLevel old_debug_level = t_debug_level_get();
  t_debug_level_set(DebugLevelWarning);
  int best_eval = maximising_player ? -INT_MAX : INT_MAX;

  int rv = best_eval;
  int best_eval_i = 0;

  if (depth == 0)
  {
    best_eval = evaluate_board(board);
    goto end;
  }

  Array moves = board_get_moves_all(board, maximising_player ? GetMovesWhite
                                                             : GetMovesBlack);

  // foreach move in board_get_moves_all:
  //  if move not in out_node.moves:
  //    out_node.moves.append(move)
  // @@Speed Maybe profile this and see if we can speed it up
  for (int i = 0; i < moves.used; i++)
  {
    bool move_in_tree = false;
    for (int j = 0; j < out_node->nchilds; j++)
    {
      if (move_equals(*(Move*)array_get(&moves, i), out_node->children[j]->move))
      {
        move_in_tree = true;
        break;
      }
    }

    if (!move_in_tree)
      node_new(out_node, *(Move*)array_get(&moves, i), !maximising_player);
  }

  array_free(&moves);

  node_order_children(out_node);

  if (out_node->nchilds == 0) // Either checkmate or stalemate
  {
    if (!is_in_check(board, maximising_player)) // Stalemate
      best_eval = 0;

    goto end;
  }

  // If moves in tree for current depth, loop over tree moves

  u64 cached_nchilds = out_node->nchilds;
  for (size_t i = 0; out_node->nchilds > 0 && i < cached_nchilds; i++)
  {
    Node* current_node = out_node->children[0];
    if (!args.prune || depth == args.max_depth)
      current_node = out_node->children[i];

    Move move = current_node->move;

    Board new_board = board;
    board_update(&new_board, &move);
    int eval =
        minimax(new_board, depth - 1, !maximising_player, current_node, args);
    new_board = board; // Restore board state after trying a move

    if (args.prune && depth != args.max_depth)
      node_free(&current_node);

    int last_best_eval = best_eval;
    best_eval =
        maximising_player ? fmax(best_eval, eval) : fmin(best_eval, eval);

    if (best_eval != last_best_eval)
      best_eval_i = i;

    if (maximising_player)
      args.alpha = fmax(args.alpha, eval);
    else
      args.beta = fmin(args.beta, eval);

    if (args.beta <= args.alpha) // Prune
      break;
  }

end:
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
    int depth = 5;
    int local_depth = 1;
    int value;
    while (local_depth <= depth)
    {
      bool prune = false;
      if (local_depth == depth)
        prune = true;

      MinimaxArgs args = {
          .alpha = -INT_MAX,
          .beta = INT_MAX,
          .max_depth = depth,
          .prune = prune,
      };
      value = minimax(tree->board, local_depth++, tree->root->isWhite,
                      tree->root, args);
    }

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
      if (move_equals(tree->root->move, current_node->children[i]->move))
        current_node = current_node->children[i];
    }
    current_node = current_node->children[current_node->best_child];
    best_move = current_node->move;
    if (current_node->nchilds <= 0)
    {
      current_node = NULL;
      can_force_mate = false;
      node_free(&checkmate_lines);
    }
  }

  return best_move;
}
