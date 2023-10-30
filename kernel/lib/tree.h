#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct tree_s {
    struct tree_child {
        struct tree_child* next;
        struct tree_child* prev;
        struct tree_s* value;
    }* children;
    struct tree_s* parent;
    void* value;
} tree_t;

typedef uint8_t(*tree_comparator_t)(void* a, void* b);

tree_t* tree_create();
tree_t* tree_find(tree_t* tree, void* data, tree_comparator_t comparator);
void tree_free_nodes(tree_t* tree);
void tree_free_values(tree_t* tree);
void tree_free(tree_t* tree);
void tree_insert_node(tree_t* parent, tree_t* node);
tree_t* tree_insert_value(tree_t* parent, void* value);
void tree_remove_branch(tree_t* node);
void tree_destroy_branch(tree_t* node);
void tree_remove_and_merge(tree_t* node);
void tree_destroy_and_merge(tree_t* node);