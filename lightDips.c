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
pthread_mutex_t dip_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

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
	int length;
	double *history;
	long long sampleCount = Sampler_getNumSamplesTaken();
	while (dipThreadRunning)
	{
		history = Sampler_getHistory(&length);
		double average = Sampler_getAverageReading();
		int POT = getPOTReading(); // 1.4
		sampleCount = Sampler_getNumSamplesTaken() - sampleCount;

		pthread_mutex_lock(&dip_counter_mutex);
		dipCount = 0;
		for (int i = 0; i < length && length >= 100; i++)
		{
			if (history[i] < average - THRESHOLD)
			{
				dipCount++;
				// skip  the next readings if they are also below the threshold
				while (i < length && history[i] < average - THRESHOLD + HYSTERESIS)
				{
					i++;
				}
				i--;
			}
			else if (history[i] > average + THRESHOLD)
			{
				dipCount++;
				// skip  the next readings if they are also above the threshold
				while (i < length && history[i] > average + THRESHOLD - HYSTERESIS)
				{
					i++;
				}
				i--;
			}
		}
		pthread_mutex_unlock(&dip_counter_mutex);

		// print the data
		printData(average, history, length, POT, dipCount);
		free(history);

		// stop sampling if 10 dips are detected
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
	int cpy;
	pthread_mutex_lock(&dip_counter_mutex);
	cpy = dipCount;
	pthread_mutex_unlock(&dip_counter_mutex);
	return cpy;
}

void startDipCounterThread(pthread_t *thread)
{
	if(dipThreadRunning)
		return;

	dipThreadRunning = true; //this has to be set before the thread is created
							//because the main loop will check this variable inside the thread
							
	pthread_create(thread, NULL, dipCounter, NULL); //this will call the function dipCounter in a new thread
													//you dont need to call dipCounter() again
}

void stopDipCounterThread()
{
	dipThreadRunning = false;
}
