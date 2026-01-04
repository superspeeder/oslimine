//
// Created by andy on 1/3/26.
//

#include "list.h"

void flist_append(flist_t *list, flist_node_t *node) {
    flist_node_t *pre = list->head;
    if (pre == nullptr) {
        list->head = node;
    } else {
        while (pre->next) {
            pre = pre->next;
        }
        pre->next = node;
    }
}

void dlist_append(dlist_t *list, dlist_node_t *node) {
    dlist_node_t* pre = list->tail;
    if (pre == nullptr) {
        list->head = node;
        list->tail = node;
        node->prev = nullptr;
    } else {
        pre->next = node;
        node->prev = pre;
        list->tail = node;
    }
    node->next = nullptr;
}

void flist_prepend(flist_t *list, flist_node_t *node) {
    if (list->head) {
        node->next = list->head;
    }
    list->head = node;
}

void dlist_prepend(dlist_t *list, dlist_node_t *node) {
    dlist_node_t* pre = list->head;
    if (pre == nullptr) {
        list->head = node;
        list->tail = node;
        node->next = nullptr;
    } else {
        pre->prev = node;
        node->next = pre;
        list->head = node;
    }
    node->prev = nullptr;
}

void flist_remove(flist_t *list, flist_node_t *node) {
    if (node == list->head) {
        list->head = node->next;
    } else { // not at the front, so we have to find the node before this one to relink things properly.
        flist_node_t* pre = list->head;
        for (; pre->next != node; pre = pre->next) {}
        pre->next = node->next;
    }

    node->next = nullptr;
}

void dlist_remove(dlist_t *list, dlist_node_t *node) {
    if (node == list->head) {
        list->head = node->next;
    }

    if (node == list->tail) {
        list->tail = node->prev;
    }

    if (node->prev) {
        node->prev->next = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
}

flist_node_t *flist_pop(flist_t *list) {
    flist_node_t* node = list->head;
    list->head = node->next;
    node->next = nullptr;
    return node;
}

dlist_node_t *dlist_pop_front(dlist_t *list) {
    dlist_node_t* node = list->head;
    list->head = node->next;
    if (node == list->tail) {
        list->tail = list->head;
    }

    node->next = nullptr;
    return node;
}

dlist_node_t *dlist_pop_end(dlist_t *list) {
    dlist_node_t* node = list->tail;
    list->tail = node->prev;
    if (node == list->head) {
        list->head = list->tail;
    }

    node->prev = nullptr;
    return node;
}

