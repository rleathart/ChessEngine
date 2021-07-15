#include "chess/defs.h"
#include "chess/move.h"
#include <chess/search.h>
#include <chess/board.h>
#include <chess/evaluate.h>
#include <chess/tree.h>

#include <stdlib.h>
#include <math.h>
#include <vcruntime.h>
#include <pthread.h>

atomic_bool g_player_moved = false;
atomic_bool g_cancel_search = false;

/// @param out_node non-null
int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Node* out_node)
{
  int best_eval = maximising_player ? -INT_MAX : INT_MAX;
  Board new_board = board;
  Move* moves;
  size_t nmoves = 0;
  board_get_moves_all(board, &moves, &nmoves,
                      maximising_player ? GetMovesWhite : GetMovesBlack);

  // @@Implement Right now, unlikely for AI to deliver checkmate since it
  // doesn't pick the line that checkmates in the fewest moves.
  if (nmoves == 0) // Either checkmate or stalemate
  {
    if (!is_in_check(board, maximising_player)) // Stalemate
      best_eval = 0;

    out_node->value = best_eval;
    free(moves);
    return best_eval;
  }

  if (depth == 0)
  {
    best_eval = evaluate_board(board);
    out_node->value = best_eval;
    free(moves);
    return best_eval;
  }

  int best_eval_i = 0;

  for (size_t i = 0; i < nmoves; i++)
  {
    if (g_cancel_search)
    {
      free(moves);
      return 0;
    }
    Node* new_node = node_new(out_node, moves[i], !maximising_player);

    board_update(&new_board, &(moves[i]));
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
  free(moves);

  out_node->value = best_eval;
  out_node->best_child = best_eval_i;

  return best_eval;
}

typedef struct MinimaxSubBoardsArgs
{
  Tree** trees;
  size_t ntrees;
  pthread_t thread;
} MinimaxSubBoardsArgs;

// @@Implement. Call minimax on current board and order the sub-boards based
// on the evaluation from highest to lowest. Then sequentially call minimax on
// subboards.
void* minimax_sub_boards(void* void_args)
{
  MinimaxSubBoardsArgs* args = (MinimaxSubBoardsArgs*)void_args;

  printf("args->ntrees: %lld\n", args->ntrees);
  for (int i = 0; i < args->ntrees; i++)
  {
    Tree* tree = args->trees[i];
    tree->is_searching = true;
    printf("analysing this move: %s\n", move_tostring(tree->root->move));
    clock_t start_time = clock();
    printf("board:\n%s\n", board_tostring(tree->board));
    printf("depth: %lld\n", tree->depth);
    minimax(tree->board, tree->depth, -INT_MAX, INT_MAX,
        tree->root->isWhite, tree->root);
    clock_t end_time = clock();
    double total_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Time taken to analyse move %s: %f\n\n",
        move_tostring(tree->root->move), total_time);
    tree->is_searching = false;
    if (!g_cancel_search)
      tree->search_complete = true;
    if (g_player_moved)
      break;
  }
  return NULL;
}

// Have to call minimax on each sub-board instead of the actual board because
// we don't want to prune sub-boards. If the player was to make a move that
// we pruned then our precalculated response would be inaccurate.
//
// This function can be optimised by ordering the sub-boards from best to worst
// and then calling minimax on the best boards first. This means that if the
// player makes one of the best moves then it is more likely that the AI will
// have a precalculated response.
//
// Perhaps a better way to optimise it would be
// to spawn the minimax of each sub-board in its own thread. That way, each
// board will be partly through the search by the time the player has moved.
// When the player moves, we cancel the search for irrelevant sub-boards and
// continue the single search that is relevant.
//
// INCORPORATE BOTH THE ABOVE OPTIMISATIONS INTO ONE SOLUTION:
// have a thread for each logical core -> run multiple searches on each
// thread, one after the other. Run the first optimisation and then run
// the best moves first. For a CPU with 8 logical cores, this means we
// run a minimax for the 8 best sub-boards first. once these minimax searches
// complete, we run the next best 8 sub-boards.
typedef struct MinimaxSubBoardsAsyncArgs
{
  Board board;
  size_t depth;
  bool maximising_player;
  Tree*** out_trees;
  size_t* out_trees_count;
  u8 threads;
} MinimaxSubBoardsAsyncArgs;

void* minimax_sub_boards_async(void* void_args)
{
  MinimaxSubBoardsAsyncArgs args = *(MinimaxSubBoardsAsyncArgs*)void_args;
  Board board = args.board;
  size_t depth = args.depth;
  bool maximising_player = args.maximising_player;
  Tree*** out_trees = args.out_trees;
  size_t* out_trees_count = args.out_trees_count;
  const u8 THREADS = args.threads;

  Node* root_player = node_new(NULL, move_new(-1,-1), maximising_player);
  Tree* tree_player = tree_new(root_player, board, depth);
  minimax(board, depth, -INT_MAX, INT_MAX, maximising_player, root_player);
  node_order_children(root_player);

  *out_trees = malloc(root_player->nchilds * sizeof(Tree*));
  *out_trees_count = root_player->nchilds;
  Tree** trees = *out_trees;
  printf("root_player->nchilds: %lld\n", root_player->nchilds);
  for (int i = 0; i < root_player->nchilds; i++)
  {
    Board new_board = board;
    board_update(&new_board, &root_player->children[i]->move);
    Node* root = node_new(NULL, root_player->children[i]->move, !maximising_player);
    trees[i] = tree_new(root, new_board, depth);
  }

  MinimaxSubBoardsArgs** msb_args_arr = malloc(THREADS * sizeof(MinimaxSubBoardsArgs*));
  pthread_t* threads = malloc(THREADS * sizeof(*threads));
  int trees_count = *out_trees_count;
  printf("trees_count: %d\n", trees_count);
  for (int i = 0; i < THREADS; i++)
  {
    msb_args_arr[i] = malloc(sizeof(MinimaxSubBoardsArgs));
    size_t msb_trees_count;
    if (i >= trees_count % THREADS)
      msb_trees_count = trees_count / THREADS;
    else
      msb_trees_count = trees_count / THREADS + 1;
    printf("msb_trees_count: %lld\n", msb_trees_count);

    Tree** msb_trees = malloc(msb_trees_count * sizeof(Tree*)); // @@FIXME mem leak here
    for (int j = 0; j < msb_trees_count; j++)
    {
      msb_trees[j] = trees[i + j * THREADS];
      printf("i + j * THREADS: %d\n", i + j * THREADS);
    }

    msb_args_arr[i]->trees = msb_trees;
    msb_args_arr[i]->ntrees = msb_trees_count;
    MinimaxSubBoardsArgs* test_args = (MinimaxSubBoardsArgs*)(void*)&msb_args_arr[i];
    pthread_create(&threads[i], NULL, minimax_sub_boards, (void*)msb_args_arr[i]);
  }

  for (int i = 0; i < THREADS; i++)
  {
    pthread_join(threads[i], NULL);
    free(msb_args_arr[i]);
  }
  tree_free(&tree_player);
  free(msb_args_arr);
  free(threads);
  return NULL;
}

pthread_t minimax_precompute_async(Board board, size_t depth,
    bool maximising_player, Tree*** out_trees, size_t* out_trees_count, u8 threads)
{
  MinimaxSubBoardsAsyncArgs* args = malloc(sizeof(MinimaxSubBoardsAsyncArgs));
  args->board = board;
  args->depth = depth;
  args->maximising_player = maximising_player;
  args->out_trees = out_trees;
  args->out_trees_count = out_trees_count;
  args->threads = threads;

  pthread_t thread;
  pthread_create(&thread, NULL, minimax_sub_boards_async, args);
  return thread;
}
