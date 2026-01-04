//
// Created by andy on 1/3/26.
//

#pragma once

/**
 * @brief Forward linked list node
 */
typedef struct flist_node_t {
    struct flist_node_t *next;
    void                *value;
} flist_node_t;

/**
 * @brief Doubly linked list node
 */
typedef struct dlist_node_t {
    struct dlist_node_t *next;
    struct dlist_node_t *prev;
    void                *value;
} dlist_node_t;

/**
 * @brief Forward linked list
 */
typedef struct flist_t {
    flist_node_t *head;
} flist_t;

/**
 * @brief Doubly linked list
 */
typedef struct dlist_t {
    dlist_node_t *head;
    dlist_node_t *tail;
} dlist_t;

/**
 * @brief This operation is O(n). Only do this when you really don't have to worry about it.
 *
 * If you need to do this alot, just use a doubly linked list
 * @param list
 * @param node
 */
void flist_append(flist_t *list, flist_node_t *node);

// O(1)
void dlist_append(dlist_t *list, dlist_node_t *node);

// O(1)
void flist_prepend(flist_t *list, flist_node_t *node);

// O(1)
void dlist_prepend(dlist_t *list, dlist_node_t *node);

/**
 * @brief Remove a node from the forward-linked-list
 *
 * It should be noted that doing this to any node that isnt the head of the list will have to iterate over the list to find it's predecessor, making this operation worst case and
 * general case O(n).
 *
 * If you need to do this alot at random positions, use a doubly linked list.
 *
 * @param list
 * @param node
 */
void flist_remove(flist_t *list, flist_node_t *node);

// O(1)
void dlist_remove(dlist_t *list, dlist_node_t *node);

flist_node_t *flist_pop(flist_t *list);

dlist_node_t *dlist_pop_front(dlist_t *list);
dlist_node_t *dlist_pop_end(dlist_t *list);

/*
List Comparisons

Doubly linked list: Append, prepend, remove are all O(1)
Forward linked list: Prepend is O(1), append is O(n), and remove has best case O(1), average case O(n)

Both have linear memory complexity, but the forward linked list uses 2n and the doubly linked list uses 3n.
*/
