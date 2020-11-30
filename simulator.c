/**
 * CS 537 Programming Assignment 4 (Fall 2020)
 * @file simulator.c
 * @brief Core module that coordinates the simulation
 * @author Michael Noguera
 */

#include "simulator.h"

void Simulator_runSimulation(FILE* tracefile) {
    bool finished = false;
    //unsigned long clock = 0; // time in nanoseconds
    //unsigned long currentline = 1;
    Process* curr = NULL; // currently running process

    while (!finished) {
        // Identify the process to run this "tick"
        if (curr == NULL) {
            curr = Process_peek(RUNNING);
            if (curr == NULL) {
                if (true) finished = true;
                break; // what if there are processes waiting for I/O?
            }
        }
    }
}