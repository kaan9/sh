/* 
 * linkedlist.c  --- linkedlist.h implementation ---  
 * Kaan B Erdogmus, Belinda Liu CIS 380, kaanberk
 */
#include "linkedlist.h"

DEQUE * make_empty_list(int (*deleter)(void *)) {
    DEQUE * d = malloc(sizeof(DEQUE));
    if (!d) return NULL;
    d->dealloc = deleter;
    d->tail = d->head = NULL;
    d->size = 0;
    return d;
}

DEQUE * make_list(void * val, int (*deleter)(void *)) {
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
    d->size = 1;
    return d;
}

int delete_list(DEQUE * d) {
    if (!d) return -1;
    if (d->size == 0) {
        free(d);
        return 0;
    }
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
    if (!d) return NULL;  //check that deque is not null
    struct node * tmp = malloc(sizeof(struct node));
    if (!tmp) return NULL;
    tmp->val          = val;
    tmp->prev         = d->tail;
    tmp->next = NULL;
    if (d->size == 0) {
        d->head = tmp;
        d->tail = tmp;
    } else {
        d->tail = d->tail->next = tmp;
    }
    d->size++;
    return d;
}

DEQUE * push_front(DEQUE * d, void * val) {
    if (!d) return NULL;  //check that deque is not null
    struct node * tmp = malloc(sizeof(struct node));
    if (!tmp) return NULL;
    tmp->val          = val;
    tmp->next         = d->head;
    tmp->prev = NULL;
    if (d->size == 0) {
        d->head = tmp;
        d->tail = tmp;
    } else {
        d->head = d->head->prev = tmp;
    }
    d->size++;
    return d;
}

void * pop_back(DEQUE * d) {
    if (!d) return NULL;       //invalid input
    if (d->size == 0) {
        return NULL; //nothing to pop
    }
    struct node * tmp = d->tail;
    d->tail    = d->tail->prev;
    void * ret = tmp->val;
    free(tmp);
    if (d->size == 1) {
        d->head = NULL;
    } else {
        d->tail->next = NULL;
    }
    d->size--;
    return ret;
}

void * pop_front(DEQUE * d) {
    if (!d) return NULL;       //invalid input
    struct node * tmp = d->head;
    d->head           = d->head->next;
    void * ret        = tmp->val;
    free(tmp);
    if (d->size == 1) {
        d->tail = NULL;
    } else {
        d->head->prev = NULL;
    }
    d->size--;
    return ret;
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
    if (d->size == 1) {
        void * ret = n->val;
        d->head = d->tail = NULL;
        d->size--;
        return ret;
    } else {
        void * ret = n->val;
        struct node * prev = n->prev;
        struct node * next = n->next;
        free(n);
        prev->next = next;
        next->prev = prev;
        d->size--;
        return ret;
    }
}
