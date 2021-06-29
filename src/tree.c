#include <chess/tree.h>
#include <chess/move.h>

#include <stdlib.h>

void tree_print_best_line(Tree tree)
{
  printf("Printing best line:\n");
  Board board = tree.board;
  Node* current_node = tree.root;
  printf("value: %d\n", current_node->value);
  printf("isWhite: %d\n", current_node->isWhite);
  printf("%s\n\n", board_tostring(board));
  while (current_node->nchilds > 0)
  {
    current_node = current_node->children[current_node->best_child];
    board_update(&board, &current_node->move);
    printf("isWhite: %d\n", current_node->isWhite);
    printf("move: %s\n", move_tostring(current_node->move));
    printf("%s\n\n", board_tostring(board));
  }
  printf("Done\n\n");
}
void tree_traverse(Node root)
{
  for (int i = 0; i < root.nchilds; i++)
  {
    tree_traverse(*(root.children[i]));
    if (root.nchilds - 1 == i)
      printf("\n\nlayer\n\n");
  }
  printf("root.move: %s\n", move_tostring(root.move));
  printf("root.value: %d\n", root.value);
}

Tree* tree_new(Node* node, Board board)
{
  Tree* tree = malloc(sizeof(Tree));
  tree->root = node;
  tree->board = board;
  return tree;
}

Node* node_new(Node* parent, Move move, bool isWhite)
{
  Node* node = malloc(sizeof(Node));
  node->parent = parent;
  node->children = NULL;
  node->nchilds = 0;
  node->best_child = -1;
  node->move = move;
  node->isWhite = isWhite;
  node->value = -INT_MAX;

  if (parent)
  {
    parent->nchilds++;
    parent->children = realloc(parent->children, parent->nchilds * sizeof(Node*));
    parent->children[parent->nchilds - 1] = node;
    if (parent->best_child < 0)
      parent->best_child = 0;
  }
  return node;
}

Move node_get_best_move(Node node)
{
  return node.children[node.best_child]->move;
}

int tree_free(Tree* tree)
{
  int nodes_freed = node_free(tree->root);
  free(tree);
  return nodes_freed;
}

int node_free(Node* node)
{
  int children_to_kill = node->nchilds;
  int children_killed = 0;
  for (int i = 0; i < children_to_kill; i++)
    children_killed += node_free(node->children[i]);
  if (node->parent)
    node->parent->nchilds--;
  free(node->children);
  free(node);
  return ++children_killed;
}
