#include "lightDips.h"
#include "sampler.h"
#include "i2c.h"
#include "udp.h"
#include "periodTimer.h"
#include "threadManager.h"
#include <pthread.h>
#include <stdio.h>

int threadArr[4];

void runAllThreads()
{
    pthread_t samplerThread, dipCounterThread, displayThread, udpThread;

    threadArr[0] = pthread_create(&samplerThread, NULL, sample, NULL);
    threadArr[1] = pthread_create(&dipCounterThread, NULL, dipCounter, NULL);
    threadArr[2] = pthread_create(&displayThread, NULL, displayDigits, NULL);
    threadArr[3] = pthread_create(&udpThread, NULL, udpServerThread, NULL);

    // for (int i = 0; i < 4; i++)
    // {
    //     if (threadArr[i] != 0)
    //     {
    //         printf("Thread %d failed to start\n", i);
    //     }
    // }

    pthread_join(samplerThread, NULL);
    pthread_join(dipCounterThread, NULL);
    pthread_join(displayThread, NULL);
    pthread_join(udpThread, NULL);

    return;
}
