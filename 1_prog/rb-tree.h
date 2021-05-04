#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

//enum of colors

enum {
    Red = 1, // not 0 for simpler search via warning
    Black = 2,
    Poison_color = 3
};

enum Poisons{
    Poison_key = -9999999
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

typedef struct rb_tree rb_tree_t;

rb_tree_t*  tree_ctor       ();
int         tree_dtor       (rb_tree_t* tree);

int         RB_insert       (rb_tree_t* tree, int key);
int         RB_delete       (rb_tree_t* tree, int key);

int         foreach         (rb_tree_t* tree, int (*func)(int, void*), void*);
int         min_key         (rb_tree_t* tree);
int         max_key         (rb_tree_t* tree);
int         RB_search       (rb_tree_t* tree, int key);
size_t*     RB_takeNodsNum  (rb_tree_t* tree);
int         tree_dump       (FILE* out, rb_tree_t* tree);

