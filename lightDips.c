#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include "sampler.h"
#include "i2c.h"
#include "periodTimer.h"
#include "lightDips.h"

Period_statistics_t stat;

bool dipThreadRunning = false;
int dipCount = 0;

void printData(double avgLight, double *history, int length, int POTsize, int dips)
{
	// show the statistics
	Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &stat);

	printf("Samples/s = %d Pot Value = %d history size = %d avg = %.3lf dips = %d ", stat.numSamples, POTsize, length, avgLight, dips);
	printf("Sampling[%.3lf %.3lf]avg %.3f/%d\n", stat.minPeriodInMs, stat.maxPeriodInMs, stat.avgPeriodInMs, stat.numSamples);

	// print every 200th sample in the history
	for (int i = 0; i < length; i += 200)
	{
		printf(" %.3lf", history[i]);
	}
	printf("\n");
}

void *dipCounter(void *arg)
{
	startDipCounterThread();
	int length;
	double *history;
	long long sampleCount = Sampler_getNumSamplesTaken();
	while (dipThreadRunning)
	{
		history = Sampler_getHistory(&length);
		double average = Sampler_getAverageReading();
		int POT = getPOTReading(); // 1.4
		sampleCount = Sampler_getNumSamplesTaken() - sampleCount;
		dipCount = 0;
		for (int i = 0; i < length && length >= 100; i++)
		{
			if (history[i] < average - (THRESHOLD + HYSTERESIS))
			{
				dipCount++;
				// skip  the next readings if they are also below the threshold
				while (i < length && history[i] < average - (THRESHOLD + HYSTERESIS))
				{
					i++;
				}
				i--;
			}

			else if (history[i] > average + (THRESHOLD - HYSTERESIS))
			{

				dipCount++;

				// skip  the next readings if they are also above the threshold
				while (i < length && history[i] > average + (THRESHOLD - HYSTERESIS))
				{
					i++;
				}

				i--;
			}
		}

		// print the data
		printData(average, history, length, POT, dipCount);
		free(history);

		
		//stop sampling if 10 dips are detected
		if (dipCount >= MAX_DIPS)
		{

			// sleep until the buffer clears
			int currentBufferSize = Sampler_getHistorySize();
			int currentSampleCount = Sampler_getNumSamplesTaken();

			// wait until the the previous buffer is cleared by new samples
			while (Sampler_getNumSamplesTaken() - currentSampleCount < currentBufferSize)
			{
				sleep(1); // sleep for 500 ms
				Period_getStatisticsAndClear(PERIOD_EVENT_SAMPLE_LIGHT, &stat);
				POT = getPOTReading();
				if (POT == 0)
				{
					Sampler_setHistorySize(1);
				}
				else
				{
					Sampler_setHistorySize(POT);
				}
			}

			// sleep for 1 second
		}

		// resize the buffer (1.4)
		if (POT == 0)
		{
			Sampler_setHistorySize(1);
		}
		else
		{
			Sampler_setHistorySize(POT);
		}
		// sleep for 1 second
		sleep(1);
	}
	return NULL;
}

int getDipCount()
{
	return dipCount;
}

void startDipCounterThread()
{
	Period_init();
	dipThreadRunning = true;
	// start the printer thread
	//pthread_t printer;
	//pthread_create(&printer, NULL, dipCounter, NULL);
	// this thread will run until the user terminates the program

	//pthread_join(printer, NULL);
}

void stopDipCounterThread()
{
	dipThreadRunning = false;
	Period_cleanup();
}