/* linkedlist.h  --- Linked list abstraction ---  Kaan B Erdogmus, Belinda Liu CIS 380, kaanberk*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

/* a deque is a struct that has pointers to the head, tail of the linked list and a pointer
 * to a deallocator function that is called on all void * vals when the list is deleted
 * the deallocator can be NULL in which case no automatic deletion happens
 * As a form of abstraction, all basic operations can be performed by passing a pointer
 * a deque into relevant functions in the header
 * O(1) for all operations except get(i) which is O(n)
 */

/* a node struct contains a pointer to a value (does not contain the value) 
 * and pointers to the previous and next nodes in the linked list
 * a pointer value of NULL indicates no prev/next node
 */
struct node {
    void * val;
    struct node * prev;
    struct node * next;
};

typedef struct {
    struct node * head;
    struct node * tail;
    int (*dealloc)(void *);
} deque;

/* 
 * any operations that would make a deque empty deallocate the deque and set the pointer to deque to NULL
 * this abstraction is memory-safe as long as only the classes in the header interact with deque and the deque
 * is deleted through delete_list (or by emptying it out)
 * all deque instances returned or passed are declared const to prevent modification outside of the class
 */

/* make a new deque struct with one node containing the value NULL and an optional deleter function for deallocation */
const deque * make_list(void * val, int (*deleter)(void *));

/* delete the deque by first calling the deallocate function in the deque on the values, then free the nodes and the deque */
int delete_list(const deque * d);

/* return the value stored at the front of the deque */
void * list_front(const deque * d);

/* return the value stored at the front of the deque */
void * list_back(const deque * d);

/* add a new value to the end of the deque */
const deque * push_back(const deque * d, void * val);

/* add a new value to the front of the deque */
const deque * push_front(const deque * d, void * val);

/* return and pop the value at the end of the deque */
void * pop_back(const deque * d);

/* return and pop the value at the front of the deque */
void * pop_front(const deque * d);

/* gets the ith value in the deque or NULL if out of bound */
void * get(const deque * d, size_t i);

#endif
