#include "core/price_tree.h"

void pt_init(price_tree_t *t) {
    // Initialize t->nil fields to safe defaults
    t->nil.left = &t->nil;
    t->nil.right = &t->nil;
    t->nil.parent = &t->nil;
    t->nil.color = PT_BLACK; // Assuming BLACK is defined for nil nodes
    t->root = &t->nil;
    t->size = 0;
}

price_level_t *pt_find(const price_tree_t *t, price_t price) {
    const price_node_t *nil = &t->nil;
    const price_node_t *x = t->root;

    while (x != nil) {
        if (price < x->key) {
            x = x->left;
        } else if (price > x->key) {
            x = x->right;
        } else {
            return x->level;
        }
    }

    return NULL;
}