#pragma once

#include "defs.h"

Node* node_new(Node* parent, Move move, bool isWhite);
int node_free(Node** node);
Move node_get_best_move(Node node);

Tree* tree_new(Node* node, Board board);
int tree_free(Tree** tree);
void tree_print_best_line(Tree tree);
void tree_traverse(Node root);
