// Manages all of the threads that the program uses.
#ifndef _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_
#include <stdbool.h>

extern bool allThreadsRunning;

// Runs all of the threads that the program requires.
void runProgram(void);

#endif
