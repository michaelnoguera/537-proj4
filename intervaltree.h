#include <stdbool.h>

typedef struct interval_node_t {
    int low;
    int high;
    int max;

    struct interval_node_t* left;
    struct interval_node_t* right;
} IntervalNode;

IntervalNode* it_initnode(int low, int high);

void it_insert(IntervalNode* root, int low, int high);

bool it_find(IntervalNode* root, int x);