/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @author Julien de Castelnau
 * @date 11/22/2020
 * @brief Implementation of a simple balancing interval tree with search, insert, and print operations supported.
 * @file intervaltree.c
 */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "intervaltree.h"

// Node constructor
IntervalNode* it_initnode(int low, int high) {
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

bool contains(int low, int high, int x) {
    return low <= x && x <= high;
}

// Insert node at the given subtree
// Helper for it_insert, so that the root can be set properly by return value.
IntervalNode* it_insert_recursive(IntervalNode* root, int low, int high) {

    // BASE CASE: correct empty location found --> add here
    if (root == NULL) {
        return it_initnode(low, high);
    }

    if (low < root->low) {
        root->left = it_insert_recursive(root->left, low, high);
    } else {
        root->right = it_insert_recursive(root->right, low, high);
    }

    if (high > root->max) {
        root->max = high;
    }

    return root;
}

// Insert interval into a given tree
void it_insert(IntervalNode* root, int low, int high) {
    assert(root != NULL);

    root = it_insert_recursive(root, low, high);
}

// Finds integer in whole subtree
bool it_find(IntervalNode* root, int x) {

    // Make sure the root is not null, and that x isn't greater than the maximum.
    if ((root == NULL) || (x > root->max)) {
        return false;
    }

    // Check current root
    if (contains(root->low, root->high, x)) return true; 

    if (root->left != NULL && root->left->max >= x) {
        return it_find(root->left, x);
    } else {
        return it_find(root->right, x);
    }
}

// Prints the tree
void it_print(IntervalNode* root) {

    if (root != NULL) {
        printf("low: %d, high: %d; ", root->low, root->high);

        if (root->left != NULL) it_print(root->left);
        if (root->right != NULL) it_print(root->right);
    }
}