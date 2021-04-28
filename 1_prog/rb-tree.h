#pragma once

#include "stdio.h"
#include "stdlib.h"
#include "errno.h"

//node of red-black tree struct

struct rb_node{
    struct rb_node* parent;
    struct rb_node* left;
    struct rb_node* right;
    int color;
    int key;
};

//red-black tree struct

typedef struct rb_tree{
    struct rb_node* nil;
    struct rb_node* root;
    size_t num_nodes;
};

//enum of colors

enum {
    Red = 1, // not 0 for simpler search via warning
    Black = 2,
    Poison_color = 3
};

enum Poisons{
    Poison_key
};

//enum of errors

enum errors{
    BAD_ARGS = -1,
    ERROR = -2,
    BAD_TREE_CONDITION = -3,
    WRONG_OPERATION = -4,
    E_TOO_MUCH_ELEM = -5,
    EMPTY_TREE = -6
};

typedef struct rb_node rb_node_t;
typedef struct rb_tree rb_tree_t;

rb_tree_t*  tree_ctor       ();
rb_node_t*  node_ctor       (int node_key);
int         tree_dtor       (rb_tree_t* tree);
int         node_dtor       (rb_node_t* node);
int         RB_insert       (rb_tree_t* tree, rb_node_t* new_node);
int         RB_delete       (rb_tree_t* tree, rb_node_t* node);
int         foreach         (rb_tree_t* tree, int (*func)(rb_tree_t*, rb_node_t*, void*), void*);
rb_node_t*  min_node        (rb_tree_t* tree, rb_node_t* root);
rb_node_t*  max_node        (rb_tree_t* tree, rb_node_t* root);
rb_node_t*  RB_search       (rb_tree_t* tree, int key);
int         RB_takeKey      (rb_node_t* node);
int         RB_takeNodsNum  (rb_node_t* tree);
rb_node_t*  RB_takeRoot     (rb_node_t* tree);
int         tree_dump       (FILE* out, rb_tree_t* tree);
