#pragma once

#include "defs.h"

#include <stdbool.h>

int minimax(Board board, size_t depth, s64 alpha, s64 beta,
            bool maximising_player, Node* out_node);
Move search(Tree* tree);
