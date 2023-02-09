// Demo application to read analog input voltage 0 on the BeagleBone
// Assumes ADC cape already loaded by uBoot:
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
#include "udp.h"


// main driver
int main()
{
	pthread_t sampler_thread, dipcounter_thread, display_thread,udp_thread;

	Period_init();

	//start the threads
	Sampler_startSampling(&sampler_thread);
	startDipCounterThread(&dipcounter_thread);
	startDisplayThread(&display_thread);
	startUdpThread(&udp_thread);


	pthread_join(sampler_thread, NULL);
	pthread_join(dipcounter_thread, NULL);
	pthread_join(display_thread, NULL);
	pthread_join(udp_thread, NULL);


	Sampler_stopSampling();
	stopDipCounterThread();
	stopDisplayThread();
	stopUdpThread();


	Period_cleanup();

	return 0;
}
