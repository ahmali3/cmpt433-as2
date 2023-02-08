// Module to count the number of light dips (thread).
#ifndef _LIGHTDIPS_H_
#define _LIGHTDIPS_H_

#define THRESHOLD 0.1
#define MAX_DIPS 10
#define HYSTERESIS 0.03

// Prints the statistics of the light samples. 
void printData(double avgLight, double *history, int length, int POTsize, int dips);

void startDipCounterThread();

void stopDipCounterThread();

int getDipCount();

// Thread to count dips.
void *dipCounter(void *arg);

#endif
