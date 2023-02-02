// Demo application to read analog input voltage 0 on the BeagleBone
// Assumes ADC cape already loaded by uBoot:

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include "sampler.h"

// tester code for 1.1
int main()
{
	Sampler_startSampling();
	while (true)
	{
		double reading = Sampler_getAverageReading();
		printf("Reading: %f\n", reading);
		sleep(1);

		if (rand() % 10 == 0)
		{
			Sampler_setHistorySize(rand() % 1000);
		}

		if (Sampler_getNumSamplesTaken() > 1000)
		{
			Sampler_stopSampling();
			break;
		}
	}
}
