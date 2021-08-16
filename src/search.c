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
  t_debug_level_push(DebugLevelDebug);
    Node* temp = node->children[i];
    node->children[i] = node->children[next];
    node->children[next] = temp;
    if (node->best_child == i)
    {
      node->best_child = next;
      DLOG("BEST_CHILD: %p %d\n", node, node->best_child);
    }
    if (node->best_child == next)
    {
      node->best_child = i;
      DLOG("BEST_CHILD: %p %d\n", node, node->best_child);
    }
  }
  DLOG("NODE_BEST_CHILD: %p %d\n", node, node->best_child);
  t_debug_level_pop();
}

// @@Rework Change the rest of search/minimax to use MinimaxOutput rather than
// node
typedef struct
{
  Node* info_node;
} MinimaxOutput;

typedef struct
{
  s64 alpha, beta;
  u64 max_depth;
  bool prune;
} MinimaxArgs;

/// @param node non-null
int minimax(Board board, u64 depth, bool maximising_player, Node* node,
            MinimaxArgs args, MinimaxOutput* output)
{
  // We don't want to print anything inside minimax
  t_debug_level_push(DebugLevelWarning);
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
    for (int j = 0; j < node->nchilds; j++)
    {
      if (move_equals(*(Move*)array_get(&moves, i), node->children[j]->move))
      {
        move_in_tree = true;
        break;
      }
    }

    if (!move_in_tree)
      node_new(node, *(Move*)array_get(&moves, i), !maximising_player);
  }

  array_free(&moves);

  node_order_children(node);

  if (node->nchilds == 0) // Either checkmate or stalemate
  {
    if (!is_in_check(board, maximising_player)) // Stalemate
      best_eval = 0;

    Node* tmp = node;
    while (tmp->parent->parent)
      tmp = tmp->parent;

    output->info_node = node_copy(*tmp);

    goto end;
  }

  // If moves in tree for current depth, loop over tree moves

  u64 cached_nchilds = node->nchilds;
  for (size_t i = 0; node->nchilds > 0 && i < cached_nchilds; i++)
  {
    Node* current_node = node->children[0];
    if (!args.prune || depth == args.max_depth)
      current_node = node->children[i];

    Move move = current_node->move;

    Board new_board = board;
    board_update(&new_board, &move);
    int eval =
        minimax(new_board, depth - 1, !maximising_player, current_node, args, output);
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
  node->value = best_eval;
  node->best_child = best_eval_i;
  DLOG("BEST_CHILD: %p %d\n", node, node->best_child);

  t_debug_level_pop();
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

  MinimaxOutput output = {};
  if (!can_force_mate)
  {
    int depth = tree->depth;
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

      // @@Rework If we want to play as black, this 'false' needs to change to
      // something else
      output.info_node = node_new(NULL, move_new(-1, -1), false);

      value = minimax(tree->board, local_depth++, tree->root->isWhite,
                      tree->root, args, &output);
    }

    best_move = node_get_best_move(*tree->root);
    if (value == -INT_MAX)
    {
      can_force_mate = true;
      // deep copy tree with root at the chosen move
      /* Node* best_child = tree->root->children[tree->root->best_child]; */
      /* checkmate_lines = node_copy(*best_child); */
      checkmate_lines = output.info_node;
      current_node = checkmate_lines;
    }
    else
    {
      node_free(&output.info_node);
    }
  }
  else
  {
    bool player_move_found = false;
    for (int i = 0; i < current_node->nchilds; i++)
    {
      DLOG("tree->root->move: %s\n", move_tostring(tree->root->move));
      if (move_equals(tree->root->move, current_node->children[i]->move))
        player_move_found = true, current_node = current_node->children[i];
    }

    if (!player_move_found)
    {
      ELOG("Failed to find player move!\n");
      getchar();
    }

    DLOG("current_node: %p\n", current_node);
    DLOG("current_node->nchilds: %lld\n", current_node->nchilds);
    DLOG("current_node->best_child: %d\n", current_node->best_child);
    DLOG("current_node->move: %s\n", move_tostring(current_node->move));
    DLOG("current_node->children[0]: %p\n", current_node->children[0]);
    DLOG("current_node->children[0]->move: %s\n", move_tostring(current_node->children[0]->move));
    DLOG("current_node->children[0]->value: %d\n", current_node->children[0]->value);
    if (current_node->nchilds == 1)
      current_node = current_node->children[0];
    else
      current_node = current_node->children[current_node->best_child];
    DLOG("current_node: %p\n", current_node);
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
