#pragma once

#include "defs.h"

Node* node_new(Node* parent, Move move, bool isWhite);
int node_free(Node** node);
Move node_get_best_move(Node node);
size_t node_find_depth(Node* node);

Tree* tree_new(Node* node, Board board, size_t depth);
int tree_free(Tree** tree);
void tree_print_best_line(Tree tree);
Node** tree_traverse(Node* root, bool (*condition)(Node), size_t* node_count);
bool tree_get_leaves_condition(Node node);
bool tree_get_leaves_all(Node node);
