/** 
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @authors Michael Noguera and Julien de Castelnau
 * @date 11/22/2020
 * @file main.c
 * @brief parses command line arguments & handles main simulation loop
 */

#include "trace_parser.h"
#include "intervaltree.h"

#include <getopt.h>

int main (int argc, char** argv) {
    int opt = 0;
    int memsize = 0;
    int pagesize = 0;
    char* filename = NULL;
    FILE* tracefile = NULL;

    while ((opt = getopt(argc, argv, "-p:m:")) != -1) {
        switch ((char)opt) {
            case 'm':
                assert(optarg != NULL);
                memsize = (int)strtol(optarg, NULL, 10);
                break;
            case 'p':
                assert(optarg != NULL);
                pagesize = (int)strtol(optarg, NULL, 10);
                break;
            case '?':
                fprintf(stderr, "ERROR: error parsing command line args\n");
                exit(EXIT_FAILURE);
                break;
            default:
                filename = optarg;
                break;
        }
    }

    if (memsize == 0) {
        fprintf(stderr, "ERROR: must specify valid memory size (in MB) on command line\n");
        exit(EXIT_FAILURE);
    }
    if (pagesize == 0) {
        fprintf(stderr, "ERROR: must specify valid page size (in bytes) on command line\n");
        exit(EXIT_FAILURE);
    }
    if (filename == NULL) {
        fprintf(stderr, "ERROR: must specify valid file name on command line\n");
        exit(EXIT_FAILURE);
    } else {
        tracefile = fopen(filename, "r");
        if (tracefile == NULL) {
            fprintf(stderr, "ERROR: error opening specified trace file\n");
            exit(EXIT_FAILURE);
        }
    }

    PQueue* trace_data = first_pass(tracefile);

    pqueue_print(trace_data);

    // Interval Tree test cases

    IntervalNode* root = it_initnode(1, 5);
    it_insert(root, 7, 8);
    it_insert(root, 8, 10);
    it_insert(root, 2, 4);

    it_print(root);
    printf("\n");

    for (int i = 0; i < 15; i++) {
        printf("interval search for %d; result: %d \n", i, it_find(root, i));
    }
    

    return 0;
}