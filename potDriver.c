// Demo application to read analog input voltage 0 on the BeagleBone
// Assumes ADC cape already loaded by uBoot:

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include "sampler.h"
#include "i2c.h"
#include "udp.h"

// main driver
int main()
{
	Sampler_startSampling();
	startUdpThread();
	startDisplayThread();
	// sleep(3);
	// Sampler_stopSampling();
	// while (true)
	// {
	// 	double reading = Sampler_getAverageReading();
	// 	printf("Reading: %f\n", reading);
	// 	sleep(1);

	// 	// if (rand() % 10 == 0)
	// 	// {
	// 	// 	Sampler_setHistorySize(rand() % 1000);
	// 	// }

	// 	if (Sampler_getNumSamplesTaken() > 1000)
	// 	{
	// 		Sampler_stopSampling();
	// 		break;
	// 	}
	// }

	// // Once per second, print the POT reading
	// while (true)
	// {
	// 	int reading = getPOTReading();
	// 	printf("Reading: %d\n", reading);
	// 	sleep(1);
	// }

		// Print the history of readings and the size of the history
// 	int length;
// 	double *history = Sampler_getHistory(&length);
// 	printf("Size of history: %d\n", length);
// 	for (int i = 0; i < length; i++)
// 	{
// 		printf("History[%d] = %f\n", i, history[i]);
// 	}
// 	free(history);

while(true) {
// stay alive
}

// sleep(10);
// stopDisplayThread();
return 0;
}
