#include <stdlib.h>
#include <assert.h>
#include "intervaltree.h"

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
void it_insert(IntervalNode* root, int low, int high) {

    // BASE CASE: correct empty location found --> add here
    if (root == NULL) return;

    if (low < root->low) {
        it_insert(root->left, low, high);
    } else {
        it_insert(root->right, low, high);
    }

    if (high > root->max) {
        root->max = high;
    }
}

bool it_find(IntervalNode* root, int x) {
    // Make sure the root is not null, and that x isn't greater than the maximum.
    if (root == NULL || x >= root->max) {
        return false;
    }

    // Check current root
    if (contains(root->low, root->high, x)) return true;

    if (it_find(root->left, x)) {
        return true;
    } else {
        return x < root->low && it_find(root->right, x);
    }
}