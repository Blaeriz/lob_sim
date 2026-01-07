#include "core/price_tree.h"
#include <stdlib.h>
#include <assert.h>

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

//MINIMUM IN PRICE TREE
price_level_t *pt_min(const price_tree_t *t) {
    const price_node_t *nil = &t->nil;
    const price_node_t *x = t->root;

    if (x == nil) {
        return NULL;
    }

    while (x->left != nil) {
        x = x->left;
    }

    return x->level;
}

//MAXIMUM IN PRICE TREE
price_level_t *pt_max(const price_tree_t *t) {
    const price_node_t *nil = &t->nil;
    const price_node_t *x = t->root;

    if (x == nil) {
        return NULL;
    }

    while (x->right != nil) {
        x = x->right;
    }

    return x->level;
}

//LEFT ROTATE TREE
static void left_rotate(price_tree_t *t, price_node_t *x) {
    price_node_t *nil = &t->nil;

    assert(x != nil);
    assert(x->right != nil);

    price_node_t *y = x->right;
    x->right = y->left;
    if (y->left != nil) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil) {
        t->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

//RIGHT ROTATE TREE
static void right_rotate(price_tree_t *t, price_node_t *x) {
    price_node_t *nil = &t->nil;

    assert(x != nil);
    assert(x->left != nil);

    price_node_t *y = x->left;
    x->left = y->right;
    if (y->right != nil) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == nil) {
        t->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->right = y;
    }
    y->right = x;
    x->parent = y;
}