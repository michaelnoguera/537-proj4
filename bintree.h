/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera
 * @date 11/4/2020
 * @brief Implementation of AVL balanced binary tree with string keys and void* values.
 * @file bintree.h
 */

#ifndef _BINTREE_
#define _BINTREE_

#include <stdlib.h>

struct bt_node_t;

typedef struct bt_node_t {
    struct bt_node_t* left;
    struct bt_node_t* right;
    int bf;
    const void* value;  /// pointer to whatever value is to be stored
    char key[];    /// pointer to string key
} BTNode;

/// Implementation of AVL Balanced Binary Tree
typedef struct bt_tree_t {
    struct bt_node_t* root;
} BTree;

/**
 * Constructs a new binary tree 
 * @return a pointer to the BTree
 */
BTree* bt_initializeTree();

/**
 * Inserts a value identified by a string key to the tree.
 * 
 * @param tree BTree to insert to
 * @param key string key for accessing, duplicate keys not allowed
 * @param value void* value associated with the key 
 * 
 * @return 0 on success, 1 if there is already a node with the specified key.
 * Exits with EXIT_FAILURE upon spontaneous failure.
 */
int bt_insert(BTree* tree, const char* key, const void* value);

/**
 * Retrieves a value identified by a string key from the tree.
 * 
 * @param tree BTree to search
 * @param key string key to search for
 * 
 * @return The value associated wih the node, or NULL if none found.
 * Note that null does not neccesarily mean failure because it is a valid value
 * as well.
 */
const void* bt_get(const BTree* tree, const char* key);
#endif