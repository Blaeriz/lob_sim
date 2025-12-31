#include "core/level_ops.h"
#include <stdlib.h>

void level_init(price_level_t *level, price_t price) {
    level->price = price;
    level->total_qty = 0;
    level->head = NULL;
    level->tail = NULL;
}

int level_is_empty(const price_level_t *level) {
    return level->head == NULL;
}

order_t *level_peek(const price_level_t *level) {
    if (!level->head) return NULL;
    return level->head->order;
}

int level_push(price_level_t *level, order_t *order){
    order_node_t *new_node = malloc(sizeof *new_node);
    if (!new_node) {
        return 0; // Handle memory allocation failure
    }
    new_node->order = order;
    new_node->next = NULL;
    if(level->tail) {
        level->tail->next = new_node;
        level->tail = new_node;
        level->total_qty += order->qty;
        return 1;
    } else {
        level->head = new_node;
        level->tail = new_node;
        level->total_qty += order->qty;
        return 1;
    }
}

order_t *level_pop(price_level_t *level) {
    if(level->head){
        order_node_t *temp = level->head;
        order_t *o = temp->order;
        level->head = level->head->next;
        if(!(level->head)){
            level->tail = NULL;
        }
        level->total_qty -= o->qty;

        free(temp);
        return o;
    } else {
        return NULL;
    }
}