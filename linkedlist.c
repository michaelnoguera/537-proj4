/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file linkedlist.c
 */

#include "linkedlist.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/**
 * Initializes a new empty LinkedList.
 * Remember to call ll_free when done!
 *
 * @return pointer to new heap-allocated LinkedList
 */
LinkedList* ll_initialize() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) {
        perror("Error allocating memory for new list.\n");
        exit(EXIT_FAILURE);
    }

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

/**
 * Adds a new node to the end of a LinkedList.
 *
 * @param list The list to append to.
 * @param value Pointer to thing contained within the new node.
 */
void ll_push(LinkedList* list, void* value) {
    if (list == NULL) {
        perror("Can't add an element to NULL.");
        exit(EXIT_FAILURE);
    }

    // construct node to add
    struct ll_node_t* new = (struct ll_node_t*)malloc(sizeof(struct ll_node_t));
    if (new == NULL) {
        perror("Error allocating memory for new node.\n");
        exit(EXIT_FAILURE);
    }
    new->value = value;

    // attach node to tail of list
    if (list->size == 0) {
        list->head = new;
        list->tail = new;
    } else {
        list->tail->next = new;
        list->tail = new;
    }
    list->size++;

    new->next = list->head; // complete circular linking
}

/**
 * Gets the value of the node at an index.
 *
 * @param list The list to search.
 * @param index The index of the node to be retrieved. Lists are zero-indexed.
 * @return The value of the node, or NULL if no such node exists.
 */
void* ll_get(const LinkedList* list, int index) {
    if (list == NULL) {
        perror("Can't get an element from NULL.");
        exit(EXIT_FAILURE);
    }

    if (list->size <= index) return NULL;

    struct ll_node_t* curr = list->head;
    for (int i = 0; i < index; i++) { curr = curr->next; }

    return curr->value;
}

/**
 * Converts a LinkedList into a fixed-length array of void pointers
 *
 * @param list LinkedList to convert
 * @return pointer to heap-allocated array of void pointers, or NULL when
 * invalid input (list can't be NULL, of course!)
 */
void** ll_to_array(LinkedList* list) {
    if (list == NULL) return NULL;

    void** arr = malloc(sizeof(void*) * (list->size + 1));

    struct ll_node_t* n = list->head;
    for (int i = 0; i < list->size; i++) {
        arr[i] = n->value;
        n = n->next;
    }

    arr[list->size] = NULL;

    return arr;
}

/**
 * Frees all memory from the specified `LinkedList` and pointees of the pointers
 * it stores.
 * If `ptr` is NULL, no operation is performed.
 *
 * @param ptr Pointer to LinkedList to be freed.
 */
void ll_free(LinkedList* ptr) {
    if (ptr == NULL) return;

    // free all nodes
    struct ll_node_t* tmp = NULL;
    while (ptr->head != ptr->tail) {
        tmp = ptr->head;
        ptr->head = ptr->head->next;
        free(tmp->value);
        free(tmp);
    }
    free(ptr->tail->value);
    free(ptr->tail);

    // free list wrapper
    free(ptr);
}

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
void ll_destruct(LinkedList* ptr) {
    if (ptr == NULL) return;

    // free all nodes
    struct ll_node_t* tmp = NULL;
    while (ptr->head != ptr->tail) {
        tmp = ptr->head;
        ptr->head = ptr->head->next;
        free(tmp);
    }
    free(ptr->tail);

    // free list wrapper
    free(ptr);
}

// Debugging helper function, prints node values as mem addrs.
inline void ll_print(LinkedList* list) {
    if (list == NULL) return;
    printf("\x1B[4mLinkedList\x1B[0m\n");
    printf("\t\x1B[31maddr\x1B[0m = %p\n", (void*)list);
    printf("\t\x1B[31msize\x1B[0m = %i\n", list->size);

    if (list->size > 0) {
        // print head ptr
        printf("\t\x1B[95mhead\x1B[0m");

        // print all nodes
        struct ll_node_t* n = list->head;
        const int NODES_ON_LINE = 5;
        for (int i = 0; i < list->size; i++) {
            if (i % NODES_ON_LINE == NODES_ON_LINE - 1) printf("\n\t");
            printf("\x1B[2m->\x1B[0m\x1B[33m%i.\x1B[0m\x1B[1m%p\x1B[0m", i,
                   n->value);

            n = n->next;
        }

        // print tail ptr
        printf("\x1B[2m<-\x1B[0m\x1B[95mtail\x1B[0m\n");
    }
}

// Debugging helper function, prints node values as strings
inline void ll_print_as_strings(LinkedList* list) {
    if (list == NULL) return;
    printf("\x1B[4mLinkedList\x1B[0m\n");
    printf("\t\x1B[31maddr\x1B[0m = %p\n", (void*)list);
    printf("\t\x1B[31msize\x1B[0m = %i\n", list->size);

    if (list->size > 0) {
        // print head ptr
        printf("\t\x1B[95mhead\x1B[0m");

        // print all nodes
        struct ll_node_t* n = list->head;
        const int NODES_ON_LINE = 5;
        for (int i = 0; i < list->size; i++) {
            if (i % NODES_ON_LINE == NODES_ON_LINE - 1) printf("\n\t");
            printf("\x1B[2m->\x1B[0m\x1B[33m%i.\x1B[0m\"\x1B[1m%s\x1B[0m\"", i,
                   (char*)n->value);

            n = n->next;
        }

        // print tail ptr
        printf("\x1B[2m<-\x1B[0m\x1B[95mtail\x1B[0m\n");
    }
}

// TODO: Remove. This is just for testing

#include "trace_parser.h"

inline void ll_print_as_custom(LinkedList* list) {
    if (list == NULL) return;
    printf("\x1B[4mLinkedList\x1B[0m\n");
    printf("\t\x1B[31maddr\x1B[0m = %p\n", (void*)list);
    printf("\t\x1B[31msize\x1B[0m = %i\n", list->size);

    if (list->size > 0) {
        // print head ptr
        printf("\t\x1B[95mhead\x1B[0m");

        // print all nodes
        struct ll_node_t* n = list->head;
        const int NODES_ON_LINE = 5;
        for (int i = 0; i < list->size; i++) {
            if (i % NODES_ON_LINE == NODES_ON_LINE - 1) printf("\n\t");
            printf("\x1B[2m->\x1B[0m\x1B[33m%i.\x1B[0m\"\x1B[1m pid: %d start: %d end: %d \x1B[0m\"", i, ((PData*)n->value)->pid, ((PData*)n->value)->startline, ((PData*)n->value)->endline);

            n = n->next;
        }

        // print tail ptr
        printf("\x1B[2m<-\x1B[0m\x1B[95mtail\x1B[0m\n");
    }
}