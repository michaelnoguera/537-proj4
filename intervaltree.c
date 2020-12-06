/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Implementation of a simple balancing interval tree with search, insert, and print operations supported.
 * @file intervaltree.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "intervaltree.h"

// Node constructor
IntervalNode* it_initnode(size_t low, size_t high) {
    IntervalNode* in;
    if ((in = (IntervalNode*)malloc(sizeof(IntervalNode))) == NULL) {
        perror("Error allocating memory for new node.");
        exit(EXIT_FAILURE);
    }
    in->low = low;
    in->high = high;
    in->max = high;
    in->left = NULL;
    in->right = NULL;

    return in;
}

// Simple check to see if an integer is contained within an interval
bool it_contains(size_t low, size_t high, size_t x) {
    return low <= x && x <= high;
}

// Insert node at the given subtree
// Helper for it_insert, so that the root can be set properly by return value.
IntervalNode* it_insert_recursive(IntervalNode* root, IntervalNode* new_node) {

    // BASE CASE: correct empty location found --> add here
    if (root == NULL) {
        return new_node;
    }

    // ELSE: try to insert in left/right tree depending on where the interval lies w.r.t to the current node
    if (new_node->low < root->low) {
        root->left = it_insert_recursive(root->left, new_node);
    } else {
        root->right = it_insert_recursive(root->right, new_node);
    }

    // Update the maximum value of the subtree at root (used to make find() faster)
    if (new_node->high > root->max) {
        root->max = new_node->high;
    }

    return root;
}

// Insert interval into a given tree
void it_insert(IntervalNode* root, IntervalNode* new_node) {
    assert(root != NULL && new_node != NULL);

    root = it_insert_recursive(root, new_node);
}

// Finds integer in whole subtree
bool it_find_bool(IntervalNode* root, size_t x) {

    // Make sure the root is not null, and that x isn't greater than the maximum.
    if ((root == NULL) || (x > root->max)) {
        return false;
    }

    // Check current root
    if (it_contains(root->low, root->high, x)) return true; 

    // If the left node exists and X is within the
    // max range set by the left node, we check it.
    // Otherwise it has to be in the right node.
    if (root->left != NULL && root->left->max >= x) {
        return it_find_bool(root->left, x);
    } else {
        return it_find_bool(root->right, x);
    }
}

// Same as it_find_bool, but actually returns the pointer to the the found node.
IntervalNode* it_find(IntervalNode* root, size_t x) {

    // Make sure the root is not null, and that x isn't greater than the maximum.
    if ((root == NULL) || (x > root->max)) {
        return NULL;
    }

    // Check current root
    if (it_contains(root->low, root->high, x)) return root; 

    // If the left node exists and X is within the
    // max range set by the left node, we check it.
    // Otherwise it has to be in the right node.
    if (root->left != NULL && root->left->max >= x) {
        return it_find(root->left, x);
    } else {
        return it_find(root->right, x);
    }
}

// Prints the tree
void it_print(IntervalNode* root) {

    if (root != NULL) {
        printf("low: %lu (%lu), high: %lu; ", root->low, root->fpos_start, root->high);

        if (root->left != NULL) it_print(root->left);
        if (root->right != NULL) it_print(root->right);
    }
}

// WIP: potentially useful for extra credit
// (still need to be able to tell it to look for the next line of a specific VPN)
// not particularly useful at the moment since it only tells you when a process has its next line,
// not when a page does. checking for the VPN would require being passed a FILE* though, should probably wrap around this
size_t it_giveNext(IntervalNode* root, size_t current) {
    // Make sure the root is not null, and that x isn't greater than the maximum.
    // If it is, the next value in the tree is 0, i.e., current is greater than or equal to anything in the tree.
    if ((root == NULL) || (current >= root->max)) {
        return 0;
    }

    // Check current root
    if (root->low <= current && current < root->high) {
        return current+1;
    }

    if (root->left != NULL && root->left->max > current) {
        return it_giveNext(root->left, current);
    } else {
        return it_giveNext(root->right, current);
    }
}

// Getters and setters were used here instead of putting in init because often times
// the Fpos needs to be set after init.

// Setter function for file position decoration.
void it_setFpos(IntervalNode* n, long p) {
    n->fpos_start = p;
}

// Setter function for file position decoration.
long it_getFpos(IntervalNode* n) {
    return n->fpos_start;
}
