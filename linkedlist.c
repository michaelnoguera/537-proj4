/**
 * CS 537 Programming Assignment 4 (Fall 2020)
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

/** Node constructor. **/
static inline LinkedListNode* llnode_init(void* value) {
    LinkedListNode* new = (LinkedListNode*)malloc(sizeof(LinkedListNode));
    if (new == NULL) {
        perror("Error allocating memory for new node.\n");
        exit(EXIT_FAILURE);
    }
    new->value = value;
    return new;
}

/**
 * Inserts node at location following prev.
 * @details list must have size>0 
 */
static inline void ll_insertAfter(LinkedList* list, LinkedListNode* prev, LinkedListNode* new) {
    new->next = prev->next;
    prev->next = new;
    if (new->next == list->head) list->tail = new;

    list->size++;
}

/**
 * Inserts node at head of list
 */
static inline void ll_insertAtHead(LinkedList* list, LinkedListNode* new) {
    if (list->size == 0) {
        list->head = new;
        list->tail = new;
        new->next = list->head;
    } else {
        new->next = list->head;
        list->head = new;
    }
    
    list->size++;
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
    LinkedListNode* new = llnode_init(value);

    // attach node to tail of list
    if (list->size == 0) {
        ll_insertAtHead(list, new);
    } else {
        ll_insertAfter(list, list->tail, new);
    }
}


/**
 * Adds a new node to a LinkedList, maintaining sort.
 * @details O(N), average N/2
 *
 * @param list The list to append to.
 * @param compare Comparator used to determine sorted order.
 * @param value Pointer to thing contained within the new node.
 *
void llsorted_push(LinkedList* list, Comparator compare, void* value) {
    if (list == NULL) {
        perror("Can't add an element to NULL.");
        exit(EXIT_FAILURE);
    }

    // construct node to add
    LinkedListNode* new = llnode_init(value);

    LinkedListNode* n = list->head;

    // EDGE CASE: empty list
    if (n == NULL) return NULL;

    // EDGE CASE: only one node
    if ((*compare)(n->value, value) == EQUALTO)
        
    ll_insertAfter(list, n, new);
    // Check all values until end of list or greater value encountered
    while ((*compare)(n->next->value, value) < GREATERTHAN) {
        if (n->next == list->head) { // hit end of list without finding spot
            ll_insertAfter(list, list->tail, new);
            return;
        }
    }
    ll_insertAfter(list, list->tail, new);


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
}*/

/**
 * Gets the value of the node at an index.
 * @details O(1N), average N/2, traverses from low to high if sorted
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

    LinkedListNode* curr = list->head;
    for (int i = 0; i < index; i++) { curr = curr->next; }

    return curr->value;
}

/**
 * Gets the first node with a given value.
 * @details O(1N), use llsorted_getNodeByValue if sorted
 *
 * @param list The list to search.
 * @param value The value to search for.
 * @param compare Pointer to comparision function, as defined in this module.
 * @return The node, or NULL if no such node exists.
 */
static LinkedListNode* ll_getNodeByValue(const LinkedList* list,
                                         Comparator compare, void* value) {
    if (list == NULL) {
        perror("Can't get an element from NULL.");
        exit(EXIT_FAILURE);
    }

    // EDGE CASE: empty list
    if (list->head == NULL) return NULL;

    // EDGE CASE: first node is target
    if ((*compare)(list->head->value, value) == EQUALTO) return list->head;

    // Traverse entire list looking for a match
    for (LinkedListNode* n = list->head; n->next != list->head; n = n->next) {
        if ((*compare)(n->value, value) == EQUALTO) return n;
        if ((*compare)(n->value, value) == ERROR) {
            perror("WARNING: comparison failed");
        }
    }

    return NULL; // no match found
}

/**
 * Gets the node with a given value. List must be sorted.
 * @details O(1N), but use tail pointer if you need the last element. Average
 * case N/2, better for smaller items in sort.
 *
 * @param list The list to search.
 * @param value The value to search for.
 * @param compare Pointer to comparision function, as defined in this module.
 * @return The node, or NULL if no such node exists.
 */
static LinkedListNode* llsorted_getNodeByValue(const LinkedList* list,
                                               Comparator compare,
                                               void* value) {
    if (list == NULL) {
        perror("Can't get an element from NULL.");
        exit(EXIT_FAILURE);
    }
    LinkedListNode* n = list->head;

    // EDGE CASE: empty list
    if (n == NULL) return NULL;

    // EDGE CASE: only one node
    if ((*compare)(n->value, value) == EQUALTO) return n;

    // Check all values until end of list or greater value encountered
    while (n->next != list->head) {
        switch ((*compare)(n->value, value)) {
            case LESSTHAN:
                continue;
            case EQUALTO:
                return n;
            case GREATERTHAN:
                return NULL;
            case ERROR:
                perror("WARNING: comparison failed");
                break;
        }
    }

    return NULL; // target not found
}

/**
 * Calls a Comparator on two nodes and returns the result.
 * @param compare pointer to Comparator function, as defined in this module
 * @param a the first value to compare
 * @param b the value to compare a to
 * @return -1 if a<b, 0 if a==b, 1 if a>b
 */
static Comparison ll_compareNodes(Comparator compare, LinkedListNode* a,
                                  LinkedListNode* b) {
    if (a == NULL || b == NULL) {
        perror("null node not valid for comparison");
        exit(EXIT_FAILURE);
    } else if ((*compare)(a->value, b->value) == ERROR) {
        perror("comparison failed");
        exit(EXIT_FAILURE);
    } else
        return (*compare)(a->value, b->value);
}

/**
 * Converts a LinkedList into a fixed-length array of void pointers
 * @details O(N)
 *
 * @param list LinkedList to convert
 * @return pointer to heap-allocated array of void pointers, or NULL when
 * invalid input (list can't be NULL, of course!)
 */
void** ll_to_array(LinkedList* list) {
    if (list == NULL) return NULL;

    void** arr = malloc(sizeof(void*) * (list->size + 1));

    LinkedListNode* n = list->head;
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
    LinkedListNode* tmp = NULL;
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
    LinkedListNode* tmp = NULL;
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
        LinkedListNode* n = list->head;
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
        LinkedListNode* n = list->head;
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

#include "memory.h"

inline void ll_print_as_custom(LinkedList* list) {
    if (list == NULL) return;
    printf("\x1B[4mLinkedList\x1B[0m\n");
    printf("\t\x1B[31maddr\x1B[0m = %p\n", (void*)list);
    printf("\t\x1B[31msize\x1B[0m = %i\n", list->size);

    if (list->size > 0) {
        // print head ptr
        printf("\t\x1B[95mhead\x1B[0m");

        // print all nodes
        LinkedListNode* n = list->head;
        for (int i = 0; i < list->size; i++) {

            printf(
              "\t\x1B[2m->\x1B[0m\x1B[33m%i.\x1B[0m\"\x1B[1m pid: %ld start: "
              "%ld end: %ld \x1B[0m, INTERVALS: ",
              i, ((Process*)n->value)->pid, ((Process*)n->value)->firstline,
              ((Process*)n->value)->lastline);
            it_print(((Process*)n->value)->lineIntervals);
            printf("\n");
            n = n->next;
        }

        // print tail ptr
        printf("\x1B[2m<-\x1B[0m\x1B[95mtail\x1B[0m\n");
    }
}