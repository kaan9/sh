/* 
 * linkedlist.c  --- linkedlist.h implementation ---  
 * Kaan B Erdogmus, Belinda Liu CIS 380, kaanberk
 */
#include "linkedlist.h"

DEQUE * make_list(void * val, int (*deleter)(void *)) {
    if (!val) return NULL;  //NULL values not allowed in DEQUE
    DEQUE * d = malloc(sizeof(DEQUE));
    if (!d) return NULL;  //if malloc fails, return NULL
    d->dealloc = deleter;
    d->tail = d->head = malloc(sizeof(struct node));
    if (!d->head) {
        free(d);
        return NULL;  
    }//if malloc fails, return NULL
    d->head->val  = val;
    d->head->prev = d->head->next = NULL;
    return d;
}

int delete_list(DEQUE * d) {
    if (!d) return -1;

    int success        = 0;
    struct node * curr = d->head;
    while (curr) {
        if (d->dealloc) success += d->dealloc(curr->val);
        struct node * tmp = curr->next;
        free(curr);
        curr = tmp;
    }
    return success;  //return the total of vals returned by the deallocator, should be 0
}

void * list_front(DEQUE * d) {
    return d && d->head ? d->head->val : NULL;
}

void * list_back(DEQUE * d) {
    return d && d->tail ? d->tail->val : NULL;
}

DEQUE * push_back(DEQUE * d, void * val) {
    if (!val || !d) return d;  //null values not allowed in DEQUE

    struct node * tmp = malloc(sizeof(struct node));
    if (!tmp) return NULL;
    tmp->val          = val;
    tmp->prev         = d->tail;
    tmp->next = NULL;
    d->tail = d->tail->next = tmp;
    return d;
}

DEQUE * push_front(DEQUE * d, void * val) {
    if (!val || !d) return d;  //null values not allowed in DEQUE

    struct node * tmp = malloc(sizeof(struct node));
    if (!tmp) return NULL;
    tmp->val          = val;
    tmp->next         = d->head;
    tmp->prev = NULL;
    d->head = d->head->prev = tmp;
    return d;
}

void * pop_back(DEQUE * d) {
    if (!d) return NULL;       //invalid input

    struct node * tmp = d->tail;

    d->tail    = d->tail->prev;
    void * val = tmp->val;
    free(tmp);

    if (!d->tail) free(d);  //if no elements left in DEQUE, destroy DEQUE
    else d->tail->next = NULL;
    return val;
}

void * pop_front(DEQUE * d) {
    if (!d) return NULL;       //invalid input

    struct node * tmp = d->head;
    d->head           = d->head->next;
    void * val        = tmp->val;
    free(tmp);

    if (!d->head) free(d);  //if no elements left in DEQUE, destroy DEQUE
    else d->head->prev = NULL;
    return val;
}

struct node * traverse(struct node * n, size_t i) {
    if (i <= 0 || !n) return n;
    return traverse(n->next, i - 1);
}

void * get(DEQUE * d, size_t i) {
    if (!d || i < 0) return NULL;

    struct node * node_i = traverse(d->head, i);
    return node_i ? node_i->val : NULL;
}

void * extract_node(DEQUE * d, struct node * n) {
    if (!d || !n || !d->head || !d->tail) return NULL;
    if (d->head == n) return pop_front(d);
    if (d->tail == n) return pop_back(d);

    void * val = n->val;
    struct node * prev = n->prev;
    struct node * next = n->next;
    free(n);
    prev->next = next;
    next->prev = prev;
    return val;
}
