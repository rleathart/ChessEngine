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

bool tree_get_leaves_condition(Node node)
{
  if (node.nchilds > 0)
    return false;
  return true;
}
bool tree_get_all_condition(Node node)
{ return true; }

//@@ FIXME: Might be a small memory leak here.
// free the returned pointer
Node** tree_traverse(Node* root, bool (*condition)(Node), size_t* length)
{
  Node** node = NULL;
  size_t child_length = 0;
  *length = 0;
  for (int i = 0; i < root->nchilds; i++)
  {
    Node** child_nodes = tree_traverse(root->children[i], condition, &child_length);
    int orig_length = *length;
    *length = *length + child_length;
    if (child_length > 0)
    {
      node = realloc(node, (*length) * sizeof(Node*));
      for (int i = *length - child_length; i < *length; i++)
        node[i] = child_nodes[i - orig_length];
    }
    free(child_nodes);
  }

  if (condition(*root))
  {
    *length = *length + 1;
    node = realloc(node, (*length) * sizeof(Node*));
    node[*length - 1] = root;
  }
  return node;
}

size_t node_find_depth(Node* node)
{
  int depth = 0;
  if (node->parent)
    depth = node_find_depth(node->parent) + 1;
  return depth;
}

Tree* tree_new(Node* node, Board board, size_t depth)
{
  Tree* tree = malloc(sizeof(Tree));
  tree->root = node;
  tree->board = board;
  tree->depth = depth;
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

//deep copies a node and its children, returning the root
Node* node_copy_private(Node* parent, Node original)
{
  Node* copy = malloc(sizeof(Node));
  copy->children = malloc(original.nchilds * sizeof(Node*));
  copy->parent = parent;
  copy->nchilds = original.nchilds;
  copy->best_child = original.best_child;
  copy->move = original.move;
  copy->isWhite = original.isWhite;
  copy->value = original.value;
  for (int i = 0; i < copy->nchilds; i++)
  {
    copy->children[i] = node_copy_private(copy, *original.children[i]);
  }
  return copy;
}
//wrapper for node_copy_private
Node* node_copy(Node original)
{
  return node_copy_private(NULL, original);
}

Move node_get_best_move(Node node)
{
  if (node.children && node.best_child >= 0)
    return node.children[node.best_child]->move;
  return move_new(-1, -1);
}

// Free returned pointer by calling tree_free. Note: this will recursively free
// all children of root.
int tree_free(Tree** tree)
{
  int nodes_freed = node_free(&((*tree)->root));
  free(*tree);
  *tree = NULL;
  return nodes_freed;
}

// Free returned pointer by calling node_free. Note: calling this function
// on an ancestor or related tree will also free this node.
int node_free(Node** node)
{
  int children_to_free = (*node)->nchilds;
  int children_freed = 0;
  for (int i = 0; i < children_to_free; i++)
    children_freed += node_free(&((*node)->children[i]));
  if ((*node)->parent) // @@FIXME Actually remove children from parent
    (*node)->parent->nchilds--;
  free((*node)->children);
  free(*node);
  *node = NULL;
  return ++children_freed;
}
