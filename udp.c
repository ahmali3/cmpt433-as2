#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <string.h>
#include "sampler.h"
#include "i2c.h"
#include "udp.h"
#include "utility.h"

bool udpThreadRunning = false;

// socket variables
int sockfd;
struct sockaddr_in serveraddr;
struct sockaddr_in clientaddr;
socklen_t clientlen = sizeof(clientaddr);

// function to set up the UDP socket
void setupSocket(void)
{
    // create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    // bind the socket to an address
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("ERROR on binding");
        exit(1);
    }
}

// function to send a UDP packet
void sendPacket(char *buffer)
{
    int n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&clientaddr, clientlen);
    if (n < 0)
    {
        perror("ERROR in sendto");
        exit(1);
    }
}

// function to close the UDP socket
void closeSocket(void)
{
    close(sockfd);
}

// function to print the help message
void printHelp(void)
{
    printf("Accepted command examples:\n");
    printf("count           -- display total number of samples taken.\n");
    printf("length          -- display number of samples in history (both max, and current).\n");
    printf("history         -- display the full sample history being saved.\n");
    printf("get 10          -- display the 10 most recent history values.\n");
    printf("dips            -- display number of dips.\n");
    printf("stop            -- cause the server program to end.\n");
    printf("<enter>         -- repeat last command.\n");
}

// function to print the number of samples taken
void printCount(void)
{
    printf("Number of samples taken = %d.", Sampler_getNumSamplesInHistory());
}

// function to print the length of the history
void printLength(void)
{
    printf("History can hold %d samples.\n", Sampler_getHistorySize());
    printf("Currently holding %d samples.\n", Sampler_getNumSamplesInHistory());
}

// function to print the history
void printHistory(void)
{
    int i;
    int numSamples;
    double *history = Sampler_getHistory(&numSamples);
    for (i = 0; i < numSamples; i++)
    {
        printf("%.3f", history[i]);
        if (i < numSamples - 1)
        {
            printf(", ");
        }
        if ((i + 1) % 20 == 0)
        {
            printf("\n");
        }
    }
}

// function to print the most recent N samples
void printGetN(int n)
{
    int i;
    int numSamples;
    double *history = Sampler_getHistory(&numSamples);
    if (n > numSamples)
    {
        printf("Error: N is greater than the number of samples currently in the history.\n");
        printf("Valid range of N is 1 to %d.\n", numSamples);
    }
    else
    {
        for (i = numSamples - n; i < numSamples; i++)
        {
            printf("%.3f", history[i]);
            if (i < numSamples - 1)
            {
                printf(", ");
            }
            if ((i + 1) % 20 == 0)
            {
                printf("\n");
            }
        }
    }
}

// function to print the number of dips
// void printDips(void)
// {
//     printf("# Dips = %d.\n", Sampler_getNumDips());
// }

// function to stop the program. Closes all threads and sockets and exits.
void stop(void)
{
    printf("Program terminating.\n");
    stopUdpThread();
    exit(0);
}

// function to handle which function to call based on the command
void handleCommand(char *command)
{
    if (strcmp(command, "help"))
    {
        printHelp();
    }
    else if (strcmp(command, "count"))
    {
        printCount();
    }
    else if (strcmp(command, "length"))
    {
        printLength();
    }
    else if (strcmp(command, "history"))
    {
        printHistory();
    }
    else if (strncmp(command, "get ", 4))
    {
        int n = atoi(command + 4);
        printGetN(n);
    }
    // else if (strcmp(command, "dips"))
    // {
    //     printDips();
    // }
    else if (strcmp(command, "stop"))
    {
        printf("Program terminating.\n");
        exit(0);
    }
    else
    {
        printHelp();
    }
}

// Thread function to listen for UDP packets. You can assume that 1,500 bytes of data will fit into a UDP packet.
void *udpThread(void *arg)
{
    char buffer[MAX_BUFFER_SIZE];
    char lastCommand[MAX_BUFFER_SIZE];
    int n;

    while (udpThreadRunning)
    {
        n = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);
        if (n < 0)
        {
            perror("ERROR in recvfrom");
            exit(1);
        }
        buffer[n] = '\0';

        if (strcmp(buffer, ""))
        {
            strcpy(buffer, lastCommand);
        }
        else
        {
            strcpy(lastCommand, buffer);
        }
        handleCommand(buffer);
        sendPacket(buffer);
    }
    return NULL;
}

// Create a thread to listen for UDP packets
void startUdpThread()
{
    setupSocket();
    pthread_t tid;
    pthread_create(&tid, NULL, udpThread, NULL);

    udpThreadRunning = true;
}

// Stop the UDP thread
void stopUdpThread()
{
    udpThreadRunning = false;
    closeSocket();
}
