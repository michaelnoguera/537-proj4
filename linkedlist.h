/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file linkedlist.h
 * @brief Implementation of heap-allocated circular singly-linked list with head 
 * and tail pointers.
 * 
 * Nodes store `int` values. Order is not guaranteed when multiple nodes have the
 * same value, especially after remove operations.
 */

#include <stdlib.h>

#ifndef _LINKEDLIST_
#define _LINKEDLIST_

typedef struct ll_node_t {
    void* value; /**< the value stored in the node */ 
    struct ll_node_t* next; /**< pointer to the next node in the list */
} LinkedListNode;

typedef struct linkedlist_t {
    int size; /**< number of items currently in the list*/
    struct ll_node_t* head; /**< pointer to the head of the list*/
    struct ll_node_t* tail; /**< pointer to the tail of the list*/
} LinkedList;

LinkedList* ll_initialize();

/**
 * Adds a new node to the end of a LinkedList.
 * 
 * @param list The list to append to.
 * @param value Pointer to thing contained within the new node.
 */
void ll_push(LinkedList* list, void* value);

/**
 * Gets the value of the node at an index.
 * 
 * @param list The list to search.
 * @param index The index of the node to be retrieved. Lists are zero-indexed.
 * @return The value of the node, or NULL if no such node exists.
 */
void* ll_get(const LinkedList* list, int index);

/**
 * Frees all memory from the specified `LinkedList` and pointees of the pointers
 * it stores.
 * If `ptr` is NULL, no operation is performed.
 * 
 * @param ptr Pointer to LinkedList to be freed.
 */
void ll_free(LinkedList* ptr);

/**
 * Converts a LinkedList into a fixed-length array of void pointers
 * 
 * @param list LinkedList to convert
 * @return pointer to heap-allocated array of void pointers, or NULL when
 * invalid input (list can't be NULL, of course!)
 */
void** ll_to_array(LinkedList* list);

/**
 * Frees overhead of the specified `LinkedList` while leaving contents intact. 
 * Pointers provided to `ll_push` are not freed, but the `LinkedList` and all 
 * `LinkedListNodes` are.
 * 
 * Make sure you have pointers to all values before calling this function.
 * 
 * If `ptr` is NULL, no operation is performed.
 * 
 * @param ptr Pointer to LinkedList to be freed.
 */
void ll_destruct(LinkedList* ptr);
void ll_print(LinkedList* list);
void ll_print_as_strings(LinkedList* list);
void ll_print_as_custom(LinkedList* list);
#endif