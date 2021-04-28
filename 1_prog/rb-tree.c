#include "rb-tree.h"

#ifdef DEVCHECKS
#define CHECK_RET(x) do {if ((x) < 0) return (x);} while(0)
#else
#define CHECK_RET(x)
#endif

#ifdef  DEVCHECKS
#define NOT_NULL(x) do {assert((x) != NULL);} while(0)
#else
#define NOT_NULL(x)
#endif

#ifdef TEST

static void* mymalloc(size_t size)
{
    static int num_iter = 0;
    if (num_iter < 4){
        num_iter++;
        if (num_iter != 2){
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

static rb_node_t*   node_create     ();
static int          insert_fixup    (rb_tree_t* tree, rb_node_t* new_node);
static void         left_rotate     (rb_tree_t* tree, rb_node_t* node);
static void         right_rotate    (rb_tree_t* tree, rb_node_t* node);
static int          node_dump       (FILE* out, rb_node_t* node, rb_tree_t* tree, size_t* counter);
static int          delete_fixup    ();
static int          node_transplant (rb_tree_t* tree, rb_node_t* to, rb_node_t* who);
static int          subtree_distruct(rb_node_t* root, rb_node_t* nil, size_t* counter);
static int          call            (rb_node_t* node, rb_tree_t* tree, int (*func)(rb_tree_t*, rb_node_t*, void*), void* data, size_t counter);

static rb_node_t* node_create()
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

rb_tree_t* tree_ctor()
{
    rb_tree_t* tree = (rb_tree_t*) Malloc(sizeof(rb_tree_t));
    if (tree == NULL)
        return NULL;

    tree->num_nodes = 0;

    tree->nil = node_create();
    if (tree->nil == NULL)
    {
        free(tree);
        return NULL;
    }
    tree->nil->color = Black;

    tree->root = tree->nil;

    return tree;
}

rb_node_t* node_ctor(int node_key)
{
    rb_node_t* new_node = node_create();
    if (new_node != NULL)
        new_node->key = node_key;

    return new_node;
}

int RB_insert(rb_tree_t* tree, rb_node_t* new_node)
{
    if (tree == NULL || new_node == NULL)
        return BAD_ARGS;

    rb_node_t* Nil = tree->nil;
    int new_key = new_node->key;

    rb_node_t* cur_node = tree->root;
    rb_node_t* new_parent = Nil;

    while(cur_node != Nil)
    {
        new_parent = cur_node;

        if (new_key > cur_node->key)
            cur_node = cur_node->right;
        else
            cur_node = cur_node->left;
    }

    if (new_parent == Nil)
        tree->root = new_node;
    else if(new_key > new_parent->key)
        new_parent->right = new_node;
    else
        new_parent->left = new_node;

    new_node->parent = new_parent;
    new_node->color = Red;
    new_node->left = Nil;
    new_node->right = Nil;

    (tree->num_nodes)++;

    int ret_val = insert_fixup(tree, new_node);

    if (ret_val != 0)
        return BAD_TREE_CONDITION;

    return 0;
}

static int insert_fixup(rb_tree_t* tree, rb_node_t* new_node)
{
// Develop time checks
    NOT_NULL(tree);
    NOT_NULL(new_node);

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

int RB_delete(rb_tree_t* tree, rb_node_t* node)
{
    if (tree == NULL || node == NULL)
        return BAD_ARGS;

    rb_node_t* nil = tree->nil;
    rb_node_t* old = node;
    int old_orig_color = old->color;
    rb_node_t* replaced = NULL;

    if (node->right == nil)
    {
        replaced = node->left;
        int ret = node_transplant(tree, node, replaced);
        if (ret < 0)
            return ret;
    }
    else if (node->left == nil)
    {
        replaced = node->right;
        int ret = node_transplant(tree, node, replaced);
        if (ret < 0)
            return ret;
    }
    else
    {
        old = min_node(tree, node->right);
        if (old == NULL)
            return ERROR;
        //Develop time checks
        //if (old->left != nil)
            //return BAD_TREE_CONDITION;

        old_orig_color =  old->color;
        replaced = old->right;
        if (old->parent == node)
            replaced->parent = old; // it heps us if replaced == nil
        else
        {
            int ret = node_transplant(tree, old, old->right);
            if (ret < 0)
                return ret;
            old->right = node->right;
            old->right->parent = old;
        }
        node_transplant(tree, node, old);
        old->left = node->left;
        old->left->parent = old;
        old->color = node->color;
    }

    node_dtor(node);

    (tree->num_nodes)--;

    int ret = 0;
    if (old_orig_color == Black)
        ret = delete_fixup (tree, replaced);

    return ret;
}

static int node_transplant(rb_tree_t* tree, rb_node_t* to, rb_node_t* who)
{
    if (tree == NULL || to == NULL || who == NULL)
        return BAD_ARGS;

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

int node_dtor(rb_node_t* node)
{
    if (node == NULL)
        return BAD_ARGS;

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

    if (counter != 0)
    {
        tree->num_nodes = counter;
        return counter;
    }

    ret = node_dtor(tree->nil);

    tree->root = NULL;
    tree->nil = NULL;
    tree->num_nodes = 0;

    free(tree);

    return ret;
}

static int subtree_distruct(rb_node_t* root, rb_node_t* nil, size_t* counter)
{
    if (root == NULL || nil == NULL || counter == NULL)
        return BAD_ARGS;

    if (root == nil)
        return 0;

    int ret = 0;
    if (root->left != nil || root->left != NULL)
    {
        ret = subtree_distruct(root->left, nil, counter);
        if (ret != 0)
            return ret;
    }
    if (root->right != nil || root->right != NULL)
    {
        ret = subtree_distruct(root->right, nil, counter);
        if (ret != 0)
            return ret;
    }

    if (*counter == 0)
        return E_TOO_MUCH_ELEM;

    (*counter)--;

    ret = node_dtor(root);

    return ret;
}

static int delete_fixup(rb_tree_t* tree, rb_node_t* extra_black)
{
    if (tree == NULL || extra_black == NULL)
        return BAD_ARGS;

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

int foreach(rb_tree_t* tree, int (*func)(rb_tree_t*, rb_node_t*, void*), void* data)
{
    if (tree == NULL || func == NULL)
        return BAD_ARGS;

    int ret = call(tree->root, tree, func, data, tree->num_nodes);

    return ret;
}

static int call(rb_node_t* node, rb_tree_t* tree,
    int (*func)(rb_tree_t*, rb_node_t*, void*), void* data, size_t counter)
{
    if (node == NULL || tree == NULL || func == NULL)
        return BAD_ARGS;

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

    ret = func(tree, node, data);
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

rb_node_t* min_node(rb_tree_t* tree, rb_node_t* node)
{
    if (tree == NULL || node == NULL)
        return NULL;

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

rb_node_t* max_node(rb_tree_t* tree, rb_node_t* node)
{
    if (tree == NULL || node == NULL)
        return NULL;

    size_t counter = tree->num_nodes;
    rb_node_t* nil = tree->nil;
    rb_node_t* max = tree->root;

    for (; counter > 0; counter--)
    {
        if (max->right == nil)
            return max;
        max = max->right;
    }

    return NULL;
}

rb_node_t* RB_search(rb_tree_t* tree, int key)
{
    if (tree == NULL)
        return NULL;

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

    fprintf(out, "\t\tkey_nil [label = \"nil\", shape = \"diamond\","
                 " color = \"#FFFFFF\", fontcolor = \"#000000\"];\n");

    fprintf(out, "}\n");

    if (ret != 0)
        return BAD_TREE_CONDITION;

    return 0;
}

static int node_dump(FILE* out, rb_node_t* node, rb_tree_t* tree, size_t* counter) // recurcive!!!!!
{
    if (node == NULL || tree == NULL || counter == NULL)
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

    if (node->parent == NULL)
        return ERROR;

    if (node->parent == tree->nil)
        fprintf(out, "\t\tkey_%d -> root_nil", node->key);
    else
        fprintf(out, "\t\tkey_%d -> key_%d", node->key, node->parent->key);
    fprintf(out, "[label = \"parent\"];\n");

    if (node->left == NULL)
        return ERROR;

    if (node->left == tree->nil)
        fprintf(out, "\t\tkey_%d -> key_nil [label = \"left\"];\n", node->key);
    else
    {
        fprintf(out, "\t\tkey_%d -> key_%d [label = \"left\"];\n", node->key, node->left->key);
        int ret = node_dump(out, node->left, tree, counter);
        if (ret < 0)
            return ret;
    }

    if (node->right == NULL)
        return ERROR;

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
