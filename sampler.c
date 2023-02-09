#define _DEFAULT_SOURCE
#include "sampler.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include "periodTimer.h"

// turn on DEV to use the simulator
// set to 0 to use the real hardware
#define DEV 0
// reads the voltage from the potentiometer(1.4)
int getPOTReading()
{
#if !DEV
    // Open file
    FILE *f = fopen(A2D_FILE_VOLTAGE0, "r");
    if (!f)
    {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf(" Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0)
    {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }
    // Close file
    fclose(f);
    return a2dReading;
#else
    return rand() % 4096;
#endif
}

double getReading()
{
#if !DEV
    FILE *f = fopen(A2D_FILE_VOLTAGE1, "r");
    if (!f)
    {
        printf("ERROR: Unable to open voltage input file. Cape loaded?\n");
        printf("       Check /boot/uEnv.txt for correct options.\n");
        exit(-1);
    }
    // Get reading
    int a2dReading = 0;
    int itemsRead = fscanf(f, "%d", &a2dReading);
    if (itemsRead <= 0)
    {
        printf("ERROR: Unable to read values from voltage input file.\n");
        exit(-1);
    }

    // Close file
    fclose(f);

    double voltage = ((double)a2dReading / A2D_MAX_READING) * A2D_VOLTAGE_REF_V;

    return voltage;
#else
    return rand() % 1000;
#endif
}

// thread variables
pthread_mutex_t mutex;
pthread_mutex_t getter_mutex;

// global variables
double average = 0.0;

// buffer variables
int buffer_capacity = INITIAL_BUFFER_SIZE;
int buffer_size = 0;

int buffer_head = 0;
int buffer_tail = 0;
long long samples_taken = 0;
double *buffer = NULL;

bool samplerThreadRunning = false;

void *sample(void *args)
{
    while (samplerThreadRunning)
    {
        double reading = getReading();
        Period_markEvent(PERIOD_EVENT_SAMPLE_LIGHT);
        // POT reading should be called in every 1 sec from the POTdriver
        //  double potReading = getPOTReading();

        // Use the value read from the POT as the size of the history, except use size 1 if reading 0.
        // if (potReading == 0)
        // {
        //     Sampler_setHistorySize(1);
        // }
        // else
        // {
        //     Sampler_setHistorySize(potReading);
        // }

        // Initially set the average as the first read value
        if (samples_taken == 0)
        {
            average = reading;
        }

        pthread_mutex_lock(&mutex);
        if (buffer_size == buffer_capacity)
        {
            buffer_tail = (buffer_tail + 1) % buffer_capacity;
            buffer_size--;
            buffer_head = (buffer_head + 1) % buffer_capacity;
        }
        // add the new reading to the buffer
        buffer[buffer_head] = reading;
        buffer_head = (buffer_head + 1) % buffer_capacity;
        buffer_size++;
        pthread_mutex_unlock(&mutex);

        // update the average
        pthread_mutex_lock(&getter_mutex);
        average = average * (1 - SMOOTHING_FACTOR) + reading * SMOOTHING_FACTOR;

        samples_taken++;
        pthread_mutex_unlock(&getter_mutex);
        // sleep for 1ms
        usleep(1000); // 1ms
    }

    free(buffer);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&getter_mutex);
    return NULL;
}

// Begin/end the background thread which samples light levels.
void Sampler_startSampling(pthread_t *samplerThread)
{

    if (samplerThreadRunning)
    {
        return;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&getter_mutex, NULL);

    samplerThreadRunning = true; // this must be set before the thread is created
                                 // otherwise the thread will exit immediately
                                 // because it will see that the variable is false inside the while loop of sample() function

    buffer = (double *)malloc(sizeof(double) * buffer_capacity);
    pthread_create(samplerThread, NULL, sample, NULL); // this will call the sample() function in a new thread
                                                      // you don't need to call sample() yourself
}

void Sampler_stopSampling(void)
{
    samplerThreadRunning = false;
}

// Set/get the maximum number of samples to store in the history.
void Sampler_setHistorySize(int newSize)
{
    pthread_mutex_lock(&mutex);
    if (newSize == buffer_capacity)
    {
        pthread_mutex_unlock(&mutex);
        return;
    }

    double *new_buffer = (double *)malloc(sizeof(double) * newSize);
    int new_buffer_size = 0;
    int new_buffer_head = 0;

    for (int i = 0; i < buffer_size && i < newSize; i++)
    {
        new_buffer[new_buffer_head] = buffer[(buffer_tail + i) % buffer_capacity];
        new_buffer_head = (new_buffer_head + 1) % newSize;
        new_buffer_size++;
    }

    free(buffer);
    buffer = new_buffer;
    buffer_capacity = newSize;

    buffer_size = new_buffer_size;
    buffer_tail = 0;
    buffer_head = new_buffer_size;
    pthread_mutex_unlock(&mutex);
}

int Sampler_getHistorySize(void)
{
    pthread_mutex_lock(&mutex);
    int buffer_capacity_cpy = buffer_capacity;
    pthread_mutex_unlock(&mutex);
    return buffer_capacity_cpy;
}

// Get a copy of the samples in the sample history.
// Returns a newly allocated array and sets `length` to be the
// number of elements in the returned array (output-only parameter).
// The calling code must call free() on the returned pointer.
// Note: provides both data and size to ensure consistency.
double *Sampler_getHistory(int *length)
{
    pthread_mutex_lock(&mutex);
    double *history = (double *)malloc(sizeof(double) * buffer_size);
    for (int i = 0; i < buffer_size; i++)
    {
        history[i] = buffer[(buffer_tail + i) % buffer_capacity];
    }
    *length = buffer_size;
    pthread_mutex_unlock(&mutex);
    return history;
}

// Returns how many valid samples are currently in the history.
// May be less than the history size if the history is not yet full.
int Sampler_getNumSamplesInHistory()
{
    pthread_mutex_lock(&mutex);
    int buffer_size_cpy = buffer_size;
    pthread_mutex_unlock(&mutex);
    return buffer_size_cpy;
}

// Get the average light level (not tied to the history).
double Sampler_getAverageReading(void)
{
    pthread_mutex_lock(&getter_mutex);
    double avg = average;
    pthread_mutex_unlock(&getter_mutex);
    return avg;
}

// Get the total number of light level samples taken so far.
long long Sampler_getNumSamplesTaken(void)
{
    pthread_mutex_lock(&getter_mutex);
    long long cpy = samples_taken;
    pthread_mutex_unlock(&getter_mutex);

    return cpy;
}
