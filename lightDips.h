// lightDips.h
// Module to count the number of light dips (thread).
#ifndef _LIGHTDIPS_H_
#define _LIGHTDIPS_H_

#define THRESHOLD 0.1
#define MAX_DIPS 10
#define HYSTERESIS 0.03


void printData(double avgLight, double *history, int length, int POTsize, int dips);

void startDipCounterThread();

void stopDipCounterThread();

int getDipCount();

// thread to count dips
void *dipCounter(void *arg);

#endif
