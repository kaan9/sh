/* linkedlist.c  --- linkedlist.h implementation ---  Kaan B Erdogmus, Belinda Liu CIS 380, kaanberk*/

#include "linkedlist.h"

deque * make_list(void * val, int (*deleter)(void *)) {
    if (!val) return NULL; //NULL values not allowed in deque
    deque * d = malloc(sizeof(deque));
    if (!d) return NULL; //if malloc fails, return NULL
    d->dealloc = deleter;
    d->tail = d->head = malloc(sizeof(struct node));
    if (!d->head) return NULL; //if malloc fails, return NULL
    d->head->val = val;
    d->head->prev = d->head->next = NULL;
    return d;
}

int delete_list(deque * d) {
    if (!d) return -1;
    int success = 0;
    struct node * curr = d->head;
    while (curr) {
        if (d->dealloc) success += d->dealloc(curr->val);
        struct node * tmp = curr->next;
	free(curr);
	curr = tmp;
    }
    return success; //return the total of vals returned by the deallocator, if no errors present should be 0
}

void * list_front(deque * d) {
    return d ? d->head->val : NULL;
}

void * list_back(deque * d) {
    return d ? d->tail->val : NULL;
}

deque * push_back(deque * d, void * val) {
    if (!val || !d) return d; //null values not allowed in deque
    struct node * tmp = malloc(sizeof(struct node));
    tmp->val = val;
    tmp->prev = d->tail;
    d->tail = d->tail->next = tmp;
    return d;
}

deque * push_front(deque * d, void * val) {
    if (!val || !d) return d; //null values not allowed in deque
    struct node * tmp = malloc(sizeof(struct node));
    tmp->val = val;
    tmp->next = d->head;
    d->head = d->head->prev = tmp;
    return d;
}

void * pop_back(deque * d) {
    if (!d) return NULL; //invalid input
    struct node * tmp = d->tail;
    d->tail = d->tail->prev;
    void * val = tmp->val;
    free(tmp);

    if (!d->tail) free(d); //if no elements left in deque, destroy deque
    d->tail->next = NULL;
    return val;
}

void * pop_front(deque * d) {
    if (!d) return NULL; //invalid input
    struct node * tmp = d->head;
    d->head = d->head->next;
    void * val = tmp->val;
    free(tmp);

    if (!d->head) free(d); //if no elements left in deque, destroy deque
    d->head->prev = NULL;
    return val;
}

struct node * traverse(struct node * n, size_t i) {
    if (i <= 0 || !n) return n;
    return traverse(n->next, i - 1);
}

void * get(deque * d, size_t i) {
    if (!d || i < 0) return NULL;
    struct node * node_i = traverse(d->head, i);
    return node_i ? node_i->val : NULL;
}
