#ifndef PRICE_TREE_H
#define PRICE_TREE_H

#include "../common/types.h"
#include "level.h"
#include <stddef.h>

typedef enum
{
  PT_RED,
  PT_BLACK
} pt_color_t;

typedef struct price_node
{
  price_t key;
  price_level_t* level;
  pt_color_t color;
  struct price_node *left, *right, *parent;
} price_node_t;

// tree
typedef struct price_tree
{
  price_node_t* root;
  price_node_t nil;
  size_t size;
} price_tree_t;

void pt_init(price_tree_t* t);

price_level_t* pt_find(const price_tree_t* t, price_t price);

int pt_insert(price_tree_t* t, price_t price, price_level_t* level);

price_level_t* pt_min(const price_tree_t* t);

price_level_t* pt_max(const price_tree_t* t);

int pt_remove(price_tree_t* t, price_t price);

// Clear the tree in O(P). The tree frees its nodes.
// free_level(level) is called for each stored level pointer (may be NULL).
void pt_clear(price_tree_t* t, void (*free_level)(price_level_t* level));

#endif