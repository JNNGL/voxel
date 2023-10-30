#include "tree.h"

#include <lib/string.h>
#include <lib/alloc.h>

tree_t* tree_create() {
    tree_t* tree = malloc(sizeof(tree_t));
    memset(tree, 0, sizeof(tree_t));
    return tree;
}

tree_t* tree_find(tree_t* tree, void* data, tree_comparator_t comparator) {
    if (!tree || !comparator) {
        return 0;
    }

    if (comparator(tree->value, data)) {
        return tree;
    }

    for (struct tree_child* child = tree->children; child; child = child->next) {
        tree_t* child_result = tree_find(child->value, data, comparator);
        if (child_result) {
            return child_result;
        }
    }

    return 0;
}

void tree_free_nodes(tree_t* tree) {
    if (!tree) {
        return;
    }

    for (struct tree_child* child = tree->children; child;) {
        struct tree_child* next = child->next;
        tree_free_nodes(child->value);
        free(child);
        child = next;
    }

    free(tree);
}

void tree_free_values(tree_t* tree) {
    if (!tree) {
        return;
    }

    for (struct tree_child* child = tree->children; child; child = child->next) {
        tree_free_values(tree);
    }

    free(tree->value);
}

void tree_free(tree_t* tree) {
    if (!tree) {
        return;
    }

    tree_free_values(tree);
    tree_free_nodes(tree);
}

void tree_insert_node(tree_t* parent, tree_t* node) {
    if (!parent || !node) {
        return;
    }

    node->parent = parent;

    struct tree_child* child = malloc(sizeof(struct tree_child));
    memset(child, 0, sizeof(struct tree_child));

    struct tree_child* children = parent->children;
    if (children) {
        children->prev = child;
    }

    child->next = children;
    parent->children = child;
}

tree_t* tree_insert_value(tree_t* parent, void* value) {
    if (!value) {
        return 0;
    }

    tree_t* tree = tree_create();
    tree->value = value;
    tree_insert_node(parent, tree);
    return tree;
}

void tree_remove_branch(tree_t* node) {
    if (!node) {
        return;
    }

    if (node->parent) {
        struct tree_child* children = node->parent->children;
        struct tree_child* entry;
        for (entry = children; entry; entry = entry->next) {
            if (entry->value == node) {
                break;
            }
        }

        if (entry) {
            if (children == entry) {
                node->parent->children = entry->next;
            }

            if (entry->prev) {
                entry->prev->next = entry->next;
            }

            if (entry->next) {
                entry->next->prev = entry->prev;
            }

            free(entry);
        }
    }

    tree_free_nodes(node);
}

void tree_destroy_branch(tree_t* node) {
    if (!node) {
        return;
    }

    tree_free_values(node);
    tree_remove_branch(node);
}

void tree_remove_and_merge(tree_t* node) {
    if (!node) {
        return;
    }

    if (node->parent) {
        struct tree_child* child;
        for (child = node->parent->children; child->next; child = child->next);
        child->next = node->children;
        free(node);
    } else {
        tree_remove_branch(node);
    }
}

void tree_destroy_and_merge(tree_t* node) {
    if (!node) {
        return;
    }

    if (node->parent) {
        free(node->value);
        tree_remove_branch(node);
    } else {
        tree_destroy_branch(node);
    }
}