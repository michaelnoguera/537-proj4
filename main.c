/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @authors Michael Noguera and Julien de Castelnau
 * @date 11/22/2020
 * @file main.c
 * @brief parses command line arguments & coordinates main simulation loop
 */

#include "intervaltree.h"
#include "memory.h"
#include "process.h"
#include "simulator.h"
#include "trace_parser.h"

#include <getopt.h>


/** Parses args */
inline static void parseArgs(int argc, char** argv, int* memsize, int* pagesize,
                             char** filename, FILE** tracefile) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "-p:m:")) != -1) {
        switch ((char)opt) {
            case 'm':
                assert(optarg != NULL);
                *memsize = (int)strtol(optarg, NULL, 10);
                break;
            case 'p':
                assert(optarg != NULL);
                *pagesize = (int)strtol(optarg, NULL, 10);
                break;
            case '?':
                fprintf(stderr, "ERROR: error parsing command line args\n");
                exit(EXIT_FAILURE);
                break;
            default:
                *filename = optarg;
                break;
        }
    }

    if (*pagesize && *pagesize % 2 != 0) {
        fprintf(stderr, "ERROR: page size must be a power of two\n");
        exit(EXIT_FAILURE);
    } else if (*pagesize < 0) {
        fprintf(stderr, "ERROR: page size must be positive\n");
        exit(EXIT_FAILURE);
    } else if (*pagesize == 0) {
        fprintf(stderr, "WARN: page size not specified, defaulting to 4096B\n");
        *pagesize=4096;
    }

    if (*memsize < 0) {
        fprintf(stderr, "ERROR: memory size must be positive\n");
        exit(EXIT_FAILURE);
    } else if (*memsize == 0) {
        fprintf(stderr,
                "WARN: memory size not specified, defaulting to 1 MB\n");
        *memsize=1;
    }

    if (*filename == NULL) {
        fprintf(stderr,
                "ERROR: must specify valid file name on command line\n");
        exit(EXIT_FAILURE);
    } else {
        *tracefile = fopen(*filename, "r");
        if (*tracefile == NULL) {
            fprintf(stderr, "ERROR: error opening specified trace file\n");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char** argv) {
    // 1. Parse args and open trace file
    int memsize = 0;
    int pagesize = 0;
    char* filename = NULL;
    FILE* tracefile = NULL;
    parseArgs(argc, argv, &memsize, &pagesize, &filename, &tracefile);

    // 2. Setup
    int numberOfPhysicalPages = memsize / pagesize;
    Memory_init(numberOfPhysicalPages);

    ProcessQueues_init();

    // 3. Read "first pass", ennumerating pids and building interval tree
    first_pass(tracefile);

    ProcessQueue_printQueue(NOTSTARTED);

    // 3. RUN SIMULATION
    Simulator_runSimulation(tracefile);

    return 0;
}