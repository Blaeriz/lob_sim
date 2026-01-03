#include "core/price_tree.h"
#include "stdlib.h"

void pt_init(price_tree_t *t) {
    // Initialize t->nil fields to safe defaults
    t->nil.left = &t->nil;
    t->nil.right = &t->nil;
    t->nil.parent = &t->nil;
    t->nil.color = PT_BLACK; // Assuming BLACK is defined for nil nodes
    t->root = &t->nil;
    t->size = 0;
}


//FIND PRICE LEVEL
price_level_t *pt_find(const price_tree_t *t, price_t price) {
    const price_node_t *nil = &t->nil;
    price_node_t *x = t->root;

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

//INSERT NODE
int pt_insert(price_tree_t *t, price_t price, price_level_t *level) {
    price_node_t *nil = &t->nil;
    price_node_t *x = t->root;
    price_node_t *parent = nil;

    while (x != nil) {
        parent = x;
        if (price < x->key) {
            x = x->left;
        } else if (price > x->key) {
            x = x->right;
        }else {
            return 0;
        }
    }

    price_node_t *z = malloc(sizeof(price_node_t));
    if (!z) return -1;

    z->key = price;
    z->level = level;
    z->color = PT_RED;      
    z->left = nil;
    z->right = nil;
    z->parent = parent;

    if (parent == nil) {
        t->root = z;
    } else if (price < parent->key) {
        parent->left = z;
    } else {
        parent->right = z;
    }

    t->size++;
    return 1;
}