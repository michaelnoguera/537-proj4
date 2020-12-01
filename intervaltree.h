/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Implementation of a simple balancing interval tree with search, insert, and print operations supported.
 * @file intervaltree.h
 */

#include <stdbool.h>
#include <stdio.h>

#ifndef _INTTREE_
#define _INTTREE_

typedef struct interval_node_t {
    int low; // low of current interval
    int high; // high of current interval
        int max; // max value present throughout entire subtree given by node
    long fpos_start;

    struct interval_node_t* left; // pointer to left interval node
    struct interval_node_t* right; // pointer to right interval node
} IntervalNode;

bool it_contains(int low, int high, int x);

/**
 * Constructs a new interval node
 * @return a pointer to the IntervalNode
 */
IntervalNode* it_initnode(int low, int high);

/**
 * Inserts an interval node into the tree.
 * 
 * @param root Root of tree to insert into
 * @param new New node to insert
 * 
 * @return none
 */
void it_insert(IntervalNode* root, IntervalNode* new_node);

/**
 * Finds an integer X within the entire tree.
 * 
 * @param root Root of tree to search in
 * @param x value to search for
 * 
 * @return true if found, false if not. 
 */
bool it_find_bool(IntervalNode* root, int x);

/**
 * Finds an integer X within the entire tree.
 * 
 * @param root Root of tree to search in
 * @param x value to search for
 * 
 * @return pointer to found node, NULL if not found. 
 */
IntervalNode* it_find(IntervalNode* root, int x);

/**
 * Walks a specified tree and prints all of its intervals.
 * 
 * @param root Root of tree to print
 * 
 * @return none
 */
void it_print(IntervalNode* root);

/**
 * A "smart increment" function. Returns the value that proceeds the integer passed as an argument,
 * assuming it is not greater than or equal to the whole tree, in which case the result is just 0.
 */
int it_giveNext(IntervalNode* root, int current);

void it_setFpos(IntervalNode* n, long p);
long it_getFpos(IntervalNode* n);
#endif 