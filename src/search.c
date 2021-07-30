#include <rgl/logging.h>

#include <chess/search.h>
#include <chess/board.h>
#include <chess/move.h>
#include <chess/evaluate.h>
#include <chess/tree.h>

#include <stdlib.h>
#include <inttypes.h>
#include <math.h>

// Selection sort
void node_order_children(Node* node)
{
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

/// @param out_node non-null
int minimax(Board board, u64 depth, u64 max_depth, s64 alpha, s64 beta,
            bool maximising_player, Node* out_node)
{
  // We don't want to print anything inside minimax
  DebugLevel old_debug_level = t_debug_level_get();
  t_debug_level_set(DebugLevelNone);
  int best_eval = maximising_player ? -INT_MAX : INT_MAX;
  Board new_board = board;

  if (out_node->nchilds == 0)
  {
    Array moves = board_get_moves_all(board,
        maximising_player ? GetMovesWhite : GetMovesBlack);
    for (int i = 0; i < moves.used; i++)
    {
      Move move = *(Move*)array_get(&moves, i);
      node_new(out_node, move, !maximising_player);
    }
    array_free(&moves);
  }
  else
  {
    node_order_children(out_node);
  }

  int rv = best_eval;
  int best_eval_i = 0;

  if (out_node->nchilds == 0) // Either checkmate or stalemate
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

  // If moves in tree for current depth, loop over tree moves

  for (size_t i = 0; i < out_node->nchilds; i++)
  {
    Node* current_node = out_node->children[i];
    Move move = current_node->move;

    board_update(&new_board, &move);
    int eval =
        minimax(new_board, depth - 1, max_depth, alpha, beta, !maximising_player, current_node);
    DebugLevel old_debug_level = t_debug_level_get();
    t_debug_level_set(DebugLevelInfo);
    if (depth == max_depth)
      for (int i = 0; i < current_node->nchilds; i++)
        ILOG("Nodes free: %d\n", node_free(&current_node->children[i]));
    t_debug_level_set(old_debug_level);
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

  // Do a search to depth 1
  // Save which moves we think are good
  // Do search with depth 2
  // Look at good moves from prev search first

  // Good moves are those which result in a good evaluation.

  if (!can_force_mate)
  {
    int depth = 5;
    int local_depth = 0;
    int value;
    while (local_depth < depth)
    {
      value = minimax(tree->board, ++local_depth, depth, -INT_MAX, INT_MAX,
          tree->root->isWhite, tree->root);
      /* u64 nnodes = 0; */
      /* free(tree_traverse(tree->root, tree_get_all_condition, &nnodes)); */
      /* ILOG("Nodes in tree: %" PRIu64 "\n", nnodes); */
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
