/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @authors Michael Noguera and Julien de Castelnau
 * @date 11/22/2020
 * @file main.c
 * @brief parses command line arguments & coordinates main simulation loop
 */

#define _GNU_SOURCE

#include "intervaltree.h"
#include "memory.h"
#include "process.h"
#include "replace.h"
#include "simulator.h"
#include "stat.h"
#include "trace_parser.h"

#include <errno.h>
#include <getopt.h>
#include <string.h>


/**
 * Parses args
 * @param[in] argc
 * @param[in] argv
 * @param[out] memsize
 * @param[out] pagesize
 * @param[out] filename
 * */
inline static void parseArgs(int argc, char** argv, int* memsize, int* pagesize,
                             char** filename) {
    int opt = 0;
    while ((opt = getopt(argc, argv, "-p:m:h")) != -1) {
        switch ((char)opt) {
            case 'm':
                assert(optarg != NULL);
                errno = 0;
                *memsize = (int)strtol(optarg, NULL, 10);
                if (errno != 0) {
                    fprintf(stderr,
                            "Error parsing -m, must be a valid integer.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'p':
                assert(optarg != NULL);
                errno = 0;
                *pagesize = (int)strtol(optarg, NULL, 10);
                if (errno != 0) {
                    fprintf(stderr,
                            "Error parsing -p, must be a valid integer.\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
                printf("Usage:\n");
                printf(
                  "  ./pfsim-lru [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf(
                  "  ./pfsim-fifo [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf(
                  "  ./pfsim-clock [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf(
                  "  ./pfsim-random [-m real memory size] [-p page size] "
                  "<tracefile>\n");
                printf("\nOptions:\n");
                printf("  -h\t");
                printf("Prints this message.\n");
                printf("  -m\t");
                printf(
                  "Amount of physical memory avaliable, in megabytes. "
                  "Defaults to 1 MB.\n");
                printf("  -p\t");
                printf(
                  "Page size as a number of bytes, must be a power of two. "
                  "Defaults to 4096 bytes.\n");

                exit(EXIT_FAILURE);
                break;
            case '?':
                if (argv != NULL && argv[0] != NULL) {
                    printf("Try '%s -h' for more information.\n",
                           basename(argv[0]));
                } else {
                    fprintf(stderr, "Could not recognize executable name.");
                    printf("Specify '-h' for more information.\n");
                }

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
        fprintf(
          stderr,
          "WARN: page size (-p) not specified, defaulting to 4096 bytes\n");
        *pagesize = 4096;
    }

    if (*memsize < 0) {
        fprintf(stderr, "ERROR: memory size must be positive\n");
        exit(EXIT_FAILURE);
    } else if (*memsize == 0) {
        fprintf(stderr,
                "WARN: memory size (-m) not specified, defaulting to 1 MB\n");
        *memsize = 1;
    }
    *memsize *= 0x100000; // MB -> bytes

    if (*pagesize > *memsize) {
        fprintf(stderr,
                "ERROR: specified page size is larger than memory size\n");
        exit(EXIT_FAILURE);
    }

    if (*filename == NULL) {
        fprintf(stderr,
                "ERROR: must specify valid file name on command line\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char** argv) {
    // 1. Parse command line arguments
    int memsize = 0;
    int pagesize = 0;
    char* filename = NULL;
    parseArgs(argc, argv, &memsize, &pagesize, &filename);
    assert(memsize > 0);
    assert(pagesize > 0);
    assert(filename != NULL);


    FILE* tracefile = NULL;
    tracefile = fopen(filename, "r");
    if (tracefile == NULL) {
        fprintf(stderr, "ERROR: error opening specified trace file\n");
        exit(EXIT_FAILURE);
    }

    // 2. Setup
    int numberOfPhysicalPages = memsize / pagesize;
    assert(numberOfPhysicalPages > 0);
    Memory_init(numberOfPhysicalPages);
    Replace_initReplacementModule(numberOfPhysicalPages);
    Stat_init();
    ProcessQueues_init();

    // 3. Read "first pass", ennumerating pids and building interval tree
    first_pass(tracefile);

    // ProcessQueue_printQueue(RUNNABLE);
    /*for (int i = 0; i < NUM_OF_PROCESS_STATUSES; i++) {
        Process* head = Process_peek(i);
        for (int i = 0; head != NULL; i++, head = STAILQ_NEXT(head, procs)) {
            printf(
              "\t\x1B[2m->\x1B[0m\x1B[33m%3d\x1B[0m\x1B[1m (%p) pid: %ld "
              "start: "
              "%ld current: %ld end: %ld \x1B[0m, INTERVALS: ",
              i, (void*)head, head->pid, head->firstline, head->currentline,
              head->lastline);
            it_print(head->lineIntervals);
            printf("\n");
        }
    }*/

    // 3. RUN SIMULATION
    unsigned long exit_time = Simulator_runSimulation(tracefile);
    Stat_printStats(exit_time);

    return 0;
}
