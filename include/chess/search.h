#pragma once

#include "defs.h"

#include <stdbool.h>
#include <pthread.h>

extern bool g_player_moved;
extern bool g_cancel_search;

int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Node* out_node);
pthread_t minimax_sub_boards_async(Board board, size_t depth,
    bool maximising_player, Tree*** out_trees, size_t* nout_trees);
