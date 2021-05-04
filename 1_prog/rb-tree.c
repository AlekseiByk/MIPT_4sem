#include "rb-tree.h"

#ifdef TEST

static void* mymalloc(size_t size)
{
    static int num_iter = 0;
    if (num_iter < 3){
        num_iter++;
        if (num_iter != 1){
            errno = ENOMEM;
            return NULL;
        }
    }
    return malloc(size);
}

#define Malloc(x) mymalloc(x)
#else
#define Malloc(x) malloc(x)
#endif

//node of red-black tree struct

typedef struct rb_node{
    struct rb_node* parent;
    struct rb_node* left;
    struct rb_node* right;
    int color;
    int key;
} rb_node_t;

//red-black tree struct

struct rb_tree{
    struct rb_node* root;
    struct rb_node* nil;
    size_t num_nodes;
};

static rb_node_t*   node_ctor       ();
static int          node_delete     (rb_node_t* node);
static int          insert_fixup    (rb_tree_t* tree, rb_node_t* new_node);
static void         left_rotate     (rb_tree_t* tree, rb_node_t* node);
static void         right_rotate    (rb_tree_t* tree, rb_node_t* node);
static int          node_dump       (FILE* out, rb_node_t* node, rb_tree_t* tree, size_t* counter);
static int          delete_fixup    ();
static int          node_transplant (rb_tree_t* tree, rb_node_t* to, rb_node_t* who);
static int          subtree_distruct(rb_node_t* root, rb_node_t* nil, size_t* counter);
static int          call            (rb_node_t* node, rb_tree_t* tree, int (*func)(int, void*), void* data, size_t counter);
static rb_node_t*   take_node       (rb_tree_t* tree, int key);
static rb_node_t*   take_min_node   (rb_tree_t* tree, rb_node_t* node);

rb_tree_t* tree_ctor()
{
    rb_tree_t* tree = (rb_tree_t*) Malloc(sizeof(rb_tree_t));
    if (tree == NULL)
        return NULL;

    tree->num_nodes = 0;

    tree->nil = node_ctor();
    if (tree->nil == NULL)
    {
        free(tree);
        return NULL;
    }
    tree->nil->color = Black;

    tree->root = tree->nil;

    return tree;
}

static rb_node_t* node_ctor()
{
    rb_node_t* node = (rb_node_t*) Malloc(sizeof(rb_node_t));
    if (node == NULL)
        return NULL;

    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;

    node->color = Poison_color;
    node->key = Poison_key;

    return node;
}

int RB_insert(rb_tree_t* tree, int key)
{
    if (tree == NULL)
        return BAD_ARGS;

    rb_node_t* Nil = tree->nil;

    rb_node_t* cur_node = tree->root;
    rb_node_t* new_parent = Nil;

    rb_node_t* new_node = node_ctor();
    new_node->key = key;

    while(cur_node != Nil)
    {
        new_parent = cur_node;

        if (key > cur_node->key)
            cur_node = cur_node->right;
        else
            cur_node = cur_node->left;
    }

    if (new_parent == Nil)
        tree->root = new_node;
    else if(key > new_parent->key)
        new_parent->right = new_node;
    else
        new_parent->left = new_node;

    new_node->parent = new_parent;
    new_node->color = Red;
    new_node->left = Nil;
    new_node->right = Nil;

    (tree->num_nodes)++;

    insert_fixup(tree, new_node);

    return 0;
}

static int insert_fixup(rb_tree_t* tree, rb_node_t* new_node)
{
    rb_node_t* cur_node = new_node;
    int ret_val = 0;

    while (cur_node->parent->color == Red)
    {
        if (cur_node->parent->parent->left == cur_node->parent) // left branch
        {
            rb_node_t* uncle = cur_node->parent->parent->right;
            if (uncle->color == Red)
            {
                cur_node->parent->color = Black;
                uncle->color = Black;

                cur_node = cur_node->parent->parent;
                cur_node->color = Red;
            }
            else
            {
                if(cur_node->parent->right == cur_node)
                {
                    cur_node = cur_node->parent;
                    left_rotate(tree, cur_node);
                }
                cur_node->parent->color = Black;
                cur_node->parent->parent->color = Red;

                right_rotate(tree, cur_node->parent->parent);
            }
        }
        else // right branch
        {
            rb_node_t* uncle = cur_node->parent->parent->left;
            if (uncle->color == Red)
            {
                cur_node->parent->color = Black;
                uncle->color = Black;

                cur_node = cur_node->parent->parent;
                cur_node->color = Red;
            }
            else
            {
                if (cur_node->parent->left == cur_node)
                {
                    cur_node = cur_node->parent;
                    right_rotate(tree, cur_node);
                }
                cur_node->parent->parent->color = Red;
                cur_node->parent->color = Black;
                left_rotate(tree, cur_node->parent->parent);
            }
        }
    }

    tree->root->color = Black;

    return 0;
}

static void left_rotate(rb_tree_t* tree, rb_node_t* node)
{
    rb_node_t* child = node->right;

    child->parent = node->parent;

    if (child->parent->left == node)
        child->parent->left = child;
    else if (child->parent->right == node)
        child->parent->right = child;

    node->parent = child;
    if (child->parent == tree->nil)
        tree->root = child;

    node->right = child->left;

    if (node->right != tree->nil)
        node->right->parent = node;

    child->left = node;

    return;
}

static void right_rotate(rb_tree_t* tree, rb_node_t* node)
{
    rb_node_t* child = node->left;

    child->parent = node->parent;

    if (child->parent->left == node)
        child->parent->left = child;
    else if (child->parent->right == node)
        child->parent->right = child;

    node->parent = child;
    if (child->parent == tree->nil)
        tree->root = child;

    node->left = child->right;

    if (node->left != tree->nil)
        node->left->parent = node;

    child->right = node;

    return;
}

int RB_delete(rb_tree_t* tree, int key)
{
    if (tree == NULL)
        return BAD_ARGS;
    rb_node_t* nil = tree->nil;
    rb_node_t* node = take_node(tree, key);
    if (node == NULL)
        return BAD_ARGS;
    rb_node_t* old = node;  
    int old_orig_color = old->color;
    rb_node_t* replaced = NULL;

    if (node->right == nil)
    {
        replaced = node->left;
        int ret = node_transplant(tree, node, replaced);
    }
    else if (node->left == nil)
    {
        replaced = node->right;
        int ret = node_transplant(tree, node, replaced);
    }
    else
    {
        old = take_min_node(tree, node->right);

        old_orig_color =  old->color;
        replaced = old->right;
        if (old->parent == node)
            replaced->parent = old;
        else
        {
            int ret = node_transplant(tree, old, old->right);
            old->right = node->right;
            old->right->parent = old;
        }
        node_transplant(tree, node, old);
        old->left = node->left;
        old->left->parent = old;
        old->color = node->color;
    }

    node_delete(node);

    (tree->num_nodes)--;

    int ret = 0;
    if (old_orig_color == Black)
        ret = delete_fixup (tree, replaced);

    return ret;
}

static int node_transplant(rb_tree_t* tree, rb_node_t* to, rb_node_t* who)
{
    rb_node_t* nil = tree->nil;

    if (to->parent == nil)
        tree->root = who;
    else if (to == to->parent->left)
        to->parent->left = who;
    else
        to->parent->right = who;

    who->parent = to->parent;

    return 0;
}

static int node_delete(rb_node_t* node)
{
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->color = Poison_color;
    node->key = Poison_key;

    free(node);

    return 0;
}

int tree_dtor(rb_tree_t* tree)
{
    if (tree == NULL)
        return BAD_ARGS;

    size_t counter = tree->num_nodes;

    int ret = subtree_distruct(tree->root, tree->nil, &counter);
    if (ret < 0)
        return ret;

    ret = node_delete(tree->nil);

    tree->root = NULL;
    tree->nil = NULL;
    tree->num_nodes = 0;

    free(tree);

    return ret;
}

static int subtree_distruct(rb_node_t* root, rb_node_t* nil, size_t* counter)
{
    if (root == nil)
        return 0;

    int ret = 0;
    if (root->left != nil || root->left != NULL)
    {
        ret = subtree_distruct(root->left, nil, counter);
    }
    if (root->right != nil || root->right != NULL)
    {
        ret = subtree_distruct(root->right, nil, counter);
    }

    if (*counter == 0)
        return E_TOO_MUCH_ELEM;

    (*counter)--;

    ret = node_delete(root);

    return ret;
}

static int delete_fixup(rb_tree_t* tree, rb_node_t* extra_black)
{
    rb_node_t* root = tree->root;
    while(extra_black->color == Black && extra_black != root)
    {
        if (extra_black->parent->left == extra_black)
        {
            rb_node_t* bro = extra_black->parent->right;
            if (bro->color == Red)
            {
                extra_black->parent->color = Red;
                bro->color = Black;

                left_rotate(tree, extra_black->parent);

                bro = extra_black->parent->right;
            }
            if (bro->left->color == Black && bro->right->color == Black)
            {
                bro->color = Red;
                extra_black = extra_black->parent;
            }
            else
            {
                if (bro->right->color == Black)
                {
                    bro->color = Red;
                    bro->left->color = Black;

                    right_rotate(tree, bro);

                    bro = extra_black->parent->right;
                }

                bro->color = extra_black->parent->color;
                extra_black->parent->color = Black;
                bro->right->color = Black;

                left_rotate(tree, extra_black->parent);

                extra_black = root;
            }
        }
        else
        {
            rb_node_t* bro = extra_black->parent->left;

            if (bro->color == Red)
            {
                bro->color = Black;
                extra_black->parent->color = Red;

                right_rotate(tree, extra_black->parent);

                bro = extra_black->parent->left;
            }
            if (bro->left->color == Black && bro->right->color == Black)
            {
                bro->color = Red;
                extra_black = extra_black->parent;
            }
            else
            {
                if (bro->left->color == Black)
                {
                    bro->right->color = Black;
                    bro->color = Red;

                    left_rotate(tree, bro);

                    bro = extra_black->parent->left;
                }

                bro->color = extra_black->parent->color;
                extra_black->parent->color = Black;
                bro->left->color = Black;

                right_rotate(tree, extra_black->parent);

                extra_black = root;
            }
        }
    }
    extra_black->color = Black;

    return 0;
}

int foreach(rb_tree_t* tree, int (*func)(int, void*), void* data)
{
    if (tree == NULL || func == NULL)
        return BAD_ARGS;

    int ret = call(tree->root, tree, func, data, tree->num_nodes);

    return ret;
}

static int call(rb_node_t* node, rb_tree_t* tree,
    int (*func)(int, void*), void* data, size_t counter)
{
    rb_node_t* nil = tree->nil;
    if (node == nil)
        return EMPTY_TREE;

    int ret = 0;

    if (counter <= 0)
        return E_TOO_MUCH_ELEM;

    if (node->left != nil)
    {
        ret = call(node->left, tree, func, data, counter - 1);
        if (ret < 0)
            return ret;
    }

    ret = func(node -> key, data);
    if (ret < 0)
        return ret;

    if (node->right != nil)
    {
        ret = call(node->right, tree, func, data, counter - 1);
        if (ret < 0)
            return ret;
    }

    return 0;
}

static rb_node_t* take_min_node(rb_tree_t* tree, rb_node_t* node)
{
    size_t counter = tree->num_nodes;
    rb_node_t* nil = tree->nil;
    rb_node_t* min = node;

    for(; counter > 0; counter--)
    {
        if (min->left == nil)
            return min;
        min = min->left;
    }

    return NULL;
}

int min_key(rb_tree_t* tree)
{
    if (tree == NULL)
        return Poison_key;

    size_t counter = tree->num_nodes;
    rb_node_t* nil = tree->nil;
    rb_node_t* min = tree->root;

    for(; counter > 0; counter--)
    {
        if (min->left == nil)
            return min -> key;
        min = min->left;
    }

    return Poison_key;
}

int max_key(rb_tree_t* tree)
{
    if (tree == NULL)
        return Poison_key;

    size_t counter = tree->num_nodes;
    rb_node_t* nil = tree->nil;
    rb_node_t* max = tree->root;

    for (; counter > 0; counter--)
    {
        if (max->right == nil)
            return max->key;
        max = max->right;
    }

    return Poison_key;
}

int RB_search(rb_tree_t* tree, int key)
{
    if (tree == NULL)
        return BAD_ARGS;

    rb_node_t* cur_node = tree->root;
    size_t counter = tree->num_nodes;

    for(; counter > 0; counter--)
    {
        if (cur_node == tree->nil)
            return 0;
        if (key > cur_node->key)
            cur_node = cur_node->right;
        else if (key < cur_node->key)
            cur_node = cur_node->left;
        else
            return 1;
    }

    return 0;
}

static rb_node_t* take_node(rb_tree_t* tree, int key)
{
    rb_node_t* cur_node = tree->root;
    size_t counter = tree->num_nodes;

    for(; counter > 0; counter--)
    {
        if (cur_node == tree->nil)
            return NULL;
        if (key > cur_node->key)
            cur_node = cur_node->right;
        else if (key < cur_node->key)
            cur_node = cur_node->left;
        else
            return cur_node;
    }

    return NULL;
}

int tree_dump(FILE* out, rb_tree_t* tree)
{
    if (out == NULL || tree == NULL)
        return BAD_ARGS;

    fprintf(out, "digraph dump\n{\n");

    fprintf(out, "\tnode [color = \"#000000\", shape = \"box\", fontsize = 20];\n"
                 "\tedge [color = \"#000000\", fontsize = 20];\n\n");

    fprintf(out, "\t\troot_nil [label = \"root_nil\", shape = \"diamond\","
                 " color = \"#FFFFFF\", fontcolor = \"#000000\"];\n");


    if (tree->num_nodes == 0)
    {
        fprintf(out, "}\n");
        return 0;
    }

    size_t counter = tree->num_nodes;
    int ret = node_dump(out, tree->root, tree, &counter);

    if (ret != 0)
        return BAD_TREE_CONDITION;

    fprintf(out, "\t\tkey_nil [label = \"nil\", shape = \"diamond\","
                 " color = \"#FFFFFF\", fontcolor = \"#000000\"];\n");

    fprintf(out, "}\n"); 

    return 0;
}

static int node_dump(FILE* out, rb_node_t* node, rb_tree_t* tree, size_t* counter) // recurcive!!!!!
{
    //if (node == NULL || tree == NULL || counter == NULL)\
        return BAD_ARGS;

    if (*counter == 0)
        return ERROR;
    (*counter)--;

    fprintf(out, "\t\tkey_%d [label = \"%d\", ", node->key, node->key);
    if (node->color == Red)
        fprintf(out, "color = \"#FF0000\", style = \"filled\", "
                     "fillcolor = \"#FF0000\", fontcolor = \"#FFFFFF\"");
    else
        fprintf(out, "color = \"#000000\", style = \"filled\", "
                     "fillcolor = \"#000000\", fontcolor = \"#FFFFFF\"");
    fprintf(out, "];\n");

    if (node->parent == tree->nil)
        fprintf(out, "\t\tkey_%d -> root_nil", node->key);
    else
        fprintf(out, "\t\tkey_%d -> key_%d", node->key, node->parent->key);
    fprintf(out, "[label = \"parent\"];\n");

    if (node->left == tree->nil)
        fprintf(out, "\t\tkey_%d -> key_nil [label = \"left\"];\n", node->key);
    else
    {
        fprintf(out, "\t\tkey_%d -> key_%d [label = \"left\"];\n", node->key, node->left->key);
        int ret = node_dump(out, node->left, tree, counter);
        if (ret < 0)
            return ret;
    }

    if (node->right == tree->nil)
        fprintf(out, "\t\tkey_%d -> key_nil [label = \"right\"];\n", node->key);
    else
    {
        fprintf(out, "\t\tkey_%d -> key_%d [label = \"right\"];\n", node->key, node->right->key);

        int ret = node_dump(out, node->right, tree, counter);
        if (ret < 0)
            return ret;
    }

    return 0;
}

size_t* RB_takeNodsNum (rb_tree_t* tree){
    if (tree != NULL)
        return &(tree->num_nodes);

    return NULL;
}
