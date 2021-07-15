#pragma once

#include "defs.h"

#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>

CHESS_EXPORT extern atomic_bool g_player_moved;
CHESS_EXPORT extern atomic_bool g_cancel_search;

int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Node* out_node);
pthread_t minimax_precompute_async(Board board, size_t depth,
    bool maximising_player, Tree*** out_trees, size_t* out_trees_count, u8 threads);
