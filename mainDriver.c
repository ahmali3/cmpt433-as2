// Main driver for the program.
// This program is designed to run on the BeagleBone Green.
// It reads the light levels from the light sensor.
// It counts the number of times the light dips below a certain threshold.
// It prints statistics about the light levels.
// It runs a UDP server that can be used to control the program from the host computer.

#define _DEFAULT_SOURCE
#include "threadManager.h"

int main()
{
    runProgram();
    return 0;
}
