/* linkedlist.h  --- Linked list abstraction ---  Kaan B Erdogmus, Belinda Liu CIS 380, kaanberk*/

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <stdio.h>
#include <stdlib.h>

/* a DEQUE is a struct that has pointers to the head, tail of the linked list and a pointer
 * to a deallocator function that is called on all void * vals when the list is deleted
 * the deallocator can be NULL in which case no automatic deletion happens
 * As a form of abstraction, all basic operations can be performed by passing a pointer
 * a DEQUE into relevant functions in the header
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
    int size;
} DEQUE;

/* 
 * any operations that would make a DEQUE empty deallocate the DEQUE and set the pointer to DEQUE
 * to NULL
 * this abstraction is memory-safe as long as only the classes in the header interact with DEQUE
 * and the DEQUE is deleted through delete_list (or by emptying it out)
 * all DEQUE instances returned or passed are declared const to prevent modification outside of
 * the class
 */

/* 
 * make a new DEQUE struct with one node containing the value NULL and an optional deleter
 * function for deallocation
 * returns NULL if val is NULL or malloc failed
 */

DEQUE * make_empty_list(int (*deleter)(void *));

DEQUE * make_list(void * val, int (*deleter)(void *));

/* 
 * delete the DEQUE by first calling the deallocate function in the DEQUE on the values,
 * then free the nodes and the DEQUE
 * if deealloc is NULL, does not automatically deallocate the values pointed to
 * returns the sum of the values returned by the deallocator function on the values
 * return value should be 0 if all deallocation succeeded
 * return value -1 if d is NULL
 */
int delete_list(DEQUE * d);

/* 
 * return the value stored at the front of the DEQUE 
 * returns NULL if d is NULL or the head is NULL
 */
void * list_front(DEQUE * d);

/* 
 * return the value stored at the front of the DEQUE
 * returns NULL if d is NULL or the head is NULL
 */
void * list_back(DEQUE * d);

/* 
 * add a new value to the end of the DEQUE
 * returns NULL if d is NULL
 */
DEQUE * push_back(DEQUE * d, void * val);

/* 
 * add a new value to the front of the DEQUE
 * returns NULL if d is NULL
 */
DEQUE * push_front(DEQUE * d, void * val);

/* return and pop the value at the end of the DEQUE */
void * pop_back(DEQUE * d);

/* return and pop the value at the front of the DEQUE */
void * pop_front(DEQUE * d);

/* gets the ith value in the DEQUE or NULL if out of bound */
void * get(DEQUE * d, size_t i);

/* note: indexed at 0, 
 * returns node at position i
 */
struct node * traverse(struct node * n, size_t i);


/* 
 * note: indexed at 1, 
 * replaces node's val with replace and returns old val
 */
void * replace(DEQUE * d, size_t i, void * replace);

/* 
 * removes the node n from d and returns the val within
 * returns NULL if node invalid or d NULL
 */
void * extract_node(DEQUE * d, struct node * n);

/** 
 * looks for a node with value val, extracts and deletes it 
 *  returns 0 on success
 *  returns -1 if d is null or d is corrupted (invalid deque)
 *  returns 1 if val is not there (including if no elts in d)
 */
int remove_val(DEQUE * d, void * val);

/* 
 * ONLY for job linked list! looks for first node with val == NULL
 * and replaces value with the input val, returns the position of
 * the first node with val == NULL
 * if no values are NULL< pushes to the back
 * TO BE USED for assigning job ids 
 * return -1 if d is NULL or invalid
 */
int insert(DEQUE * d, void * val);

/**
 *  calls the function mapper on each value in deque
 *  and sets the vals to the return value of deque
 *  returns NULL if d is null or d is corrupted (invalid deque)
 *  returns d if successful
 */
DEQUE * map(DEQUE * d, void * (*mapper)(void *));

#endif
