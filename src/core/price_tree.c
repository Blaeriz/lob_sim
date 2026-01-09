#include "core/price_tree.h"
#include <stdlib.h>
#include <assert.h>

// Forward declarations for internal helpers (needed because pt_insert calls insert_fixup)
static void left_rotate(price_tree_t *t, price_node_t *x);
static void right_rotate(price_tree_t *t, price_node_t *x);
static void insert_fixup(price_tree_t *t, price_node_t *z);

static void transplant(price_tree_t *t, price_node_t *u, price_node_t *v);
static price_node_t *tree_min_node(price_tree_t *t, price_node_t *x);
static void delete_fixup(price_tree_t *t, price_node_t *x);

void pt_init(price_tree_t *t) {
    // Initialize t->nil fields to safe defaults
    t->nil.left = &t->nil;
    t->nil.right = &t->nil;
    t->nil.parent = &t->nil;
    t->nil.color = PT_BLACK; // Assuming BLACK is defined for nil nodes
    t->root = &t->nil;
    t->size = 0;
    t->nil.key = 0;
    t->nil.level = NULL;
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

    insert_fixup(t, z);

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
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

//RB TREE FIXUP
static void insert_fixup(price_tree_t *t, price_node_t *z) {
    while (z->parent->color == PT_RED) {
        if (z->parent == z->parent->parent->left) {
            price_node_t *u = z->parent->parent->right;

            if (u->color == PT_RED) {
                z->parent->color = PT_BLACK;
                u->color = PT_BLACK;
                z->parent->parent->color = PT_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(t, z);
                }
                z->parent->color = PT_BLACK;
                z->parent->parent->color = PT_RED;
                right_rotate(t, z->parent->parent);
            }
        } else {
            price_node_t *u = z->parent->parent->left;

            if (u->color == PT_RED) {
                z->parent->color = PT_BLACK;
                u->color = PT_BLACK;
                z->parent->parent->color = PT_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(t, z);
                }
                z->parent->color = PT_BLACK;
                z->parent->parent->color = PT_RED;
                left_rotate(t, z->parent->parent);
            }
        }
    }
    t->root->color = PT_BLACK;
}

// ---- DELETE SUPPORT (RB remove) ----

static void transplant(price_tree_t *t, price_node_t *u, price_node_t *v) {
    price_node_t *nil = &t->nil;
    if (u->parent == nil) {
        t->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

static price_node_t *tree_min_node(price_tree_t *t, price_node_t *x) {
    price_node_t *nil = &t->nil;
    while (x->left != nil) x = x->left;
    return x;
}

static void delete_fixup(price_tree_t *t, price_node_t *x) {
    price_node_t *nil = &t->nil;

    while (x != t->root && x->color == PT_BLACK) {
        if (x == x->parent->left) {
            price_node_t *w = x->parent->right;

            if (w->color == PT_RED) {
                w->color = PT_BLACK;
                x->parent->color = PT_RED;
                left_rotate(t, x->parent);
                w = x->parent->right;
            }

            if (w->left->color == PT_BLACK && w->right->color == PT_BLACK) {
                w->color = PT_RED;
                x = x->parent;
            } else {
                if (w->right->color == PT_BLACK) {
                    w->left->color = PT_BLACK;
                    w->color = PT_RED;
                    right_rotate(t, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = PT_BLACK;
                w->right->color = PT_BLACK;
                left_rotate(t, x->parent);
                x = t->root;
            }
        } else {
            price_node_t *w = x->parent->left;

            if (w->color == PT_RED) {
                w->color = PT_BLACK;
                x->parent->color = PT_RED;
                right_rotate(t, x->parent);
                w = x->parent->left;
            }

            if (w->right->color == PT_BLACK && w->left->color == PT_BLACK) {
                w->color = PT_RED;
                x = x->parent;
            } else {
                if (w->left->color == PT_BLACK) {
                    w->right->color = PT_BLACK;
                    w->color = PT_RED;
                    left_rotate(t, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = PT_BLACK;
                w->left->color = PT_BLACK;
                right_rotate(t, x->parent);
                x = t->root;
            }
        }
    }

    x->color = PT_BLACK;
}

// REMOVE NODE BY PRICE (RB delete)
// Return: 1 removed, 0 not found
int pt_remove(price_tree_t *t, price_t price) {
    price_node_t *nil = &t->nil;

    // Find node z with key == price
    price_node_t *z = t->root;
    while (z != nil) {
        if (price < z->key) {
            z = z->left;
        } else if (price > z->key) {
            z = z->right;
        } else {
            break;
        }
    }
    if (z == nil) return 0;

    price_node_t *y = z;
    pt_color_t y_original_color = y->color;
    price_node_t *x;

    if (z->left == nil) {
        x = z->right;
        transplant(t, z, z->right);
    } else if (z->right == nil) {
        x = z->left;
        transplant(t, z, z->left);
    } else {
        y = tree_min_node(t, z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(t, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(t, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);
    if (t->size > 0) t->size--;

    if (y_original_color == PT_BLACK) {
        delete_fixup(t, x);
    }

    // Ensure root is black; also covers empty-tree root == nil
    t->root->color = PT_BLACK;

    // Keep NIL parent self-linked (hygiene)
    nil->parent = nil;

    return 1;
}