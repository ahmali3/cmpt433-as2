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
#include "lightDips.h"

bool udpThreadRunning = false;

// Socket variables
int sock;
struct sockaddr_in serveraddr;
struct sockaddr_in clientaddr;
socklen_t clientlen = sizeof(clientaddr);

// Idea from https://stackoverflow.com/questions/68456477/passing-multiple-buffers-with-iovec-in-c-linux-sockets

// Sends a message through the UDP socket.
void sendMessage(char *message)
{
    struct iovec iov[1];
    struct msghdr msg;
    memset(&msg, 0, sizeof(msg));
    iov[0].iov_base = message;
    iov[0].iov_len = strlen(message);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = &clientaddr;
    msg.msg_namelen = sizeof(clientaddr);
    sendmsg(sock, &msg, 0);
}

void setupUdpSocket(void)
{
    // Creates a socket
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Failed to open socket.\n\n");
        exit(1);
    }

    // Binds the socket to an address
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(PORT);
    if (bind(sock, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Binding failed.\n\n");
        exit(1);
    }
}

// Receives a UDP packet
int receivePacket(char *buffer)
{
    int n = recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&clientaddr, &clientlen);
    if (n < 0)
    {
        perror("Failed to receive packet.\n\n");
        exit(1);
    }
    return n;
}

// Closes the UDP socket
void closeSocket(void)
{
    close(sock);
}

// Prints the help message
void printHelp(void)
{
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "Accepted command examples:\n");
    sendMessage(buffer);
    sprintf(buffer, "count           -- display total number of samples taken.\n");
    sendMessage(buffer);
    sprintf(buffer, "length          -- display number of samples in history (both max, and current).\n");
    sendMessage(buffer);
    sprintf(buffer, "history         -- display the full sample history being saved.\n");
    sendMessage(buffer);
    sprintf(buffer, "get 10          -- display the 10 most recent history values.\n");
    sendMessage(buffer);
    sprintf(buffer, "dips            -- display number of dips.\n");
    sendMessage(buffer);
    sprintf(buffer, "stop            -- cause the server program to end.\n");
    sendMessage(buffer);
    sprintf(buffer, "<enter>         -- repeat last command.\n\n");
    sendMessage(buffer);
}

// Prints the number of samples taken by the sampler
void printCount(void)
{
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "Number of samples taken = %d.\n\n", Sampler_getNumSamplesInHistory());
    sendMessage(buffer);
}


// Prints the length of the sampler history
void printLength(void)
{
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "History can hold %d samples.\n", Sampler_getHistorySize());
    sendMessage(buffer);
    sprintf(buffer, "Currently holding %d samples.\n\n", Sampler_getNumSamplesInHistory());
    sendMessage(buffer);
}

// Prints the entire history.
void printHistory(void)
{
    int i;
    int numSamples;
    double *history = Sampler_getHistory(&numSamples);
    char buffer[MAX_BUFFER_SIZE];

    for (i = 0; i < numSamples; i++)
    {
        sprintf(buffer, "%.3f", history[i]);
        sendMessage(buffer);
        if (i < numSamples - 1)
        {
            sprintf(buffer, ", ");
            sendMessage(buffer);
        }
        if ((i + 1) % 20 == 0)
        {
            sprintf(buffer, "\n");
            sendMessage(buffer);
        }
    }
    sprintf(buffer, "\n\n");
    sendMessage(buffer);
}

// Prints the most recent N samples in the history.
void printGetN(int n)
{
    int i;
    int numSamples;
    double *history = Sampler_getHistory(&numSamples);
    char buffer[MAX_BUFFER_SIZE];

    if (n > numSamples)
    {
        sprintf(buffer, "Error: N is greater than the number of samples currently in the history.\n");
        sendMessage(buffer);
        sprintf(buffer, "Valid range of N is 1 to %d.\n\n", numSamples);
        sendMessage(buffer);
    }
    else
    {
        for (i = numSamples - n; i < numSamples; i++)
        {
            sprintf(buffer, "%.3f", history[i]);
            sendMessage(buffer);
            if (i < numSamples - 1)
            {
                sprintf(buffer, ", ");
                sendMessage(buffer);
            }
            if ((i - numSamples + n + 1) % 20 == 0)
            {
                sprintf(buffer, "\n");
                sendMessage(buffer);
            }
        }
    }
    sprintf(buffer, "\n\n");
    sendMessage(buffer);
}

// Prints the number of dips in the history.
void printDips(void)
{
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "# Dips = %d.\n\n", getDipCount());
    sendMessage(buffer);
}

// Closes all threads and sockets and exits.
void stop(void)
{
    char buffer[MAX_BUFFER_SIZE];
    sprintf(buffer, "Stopping program.\n\n");
    sendMessage(buffer);
    printf("Stopping program.\n\n");

    stopUdpThread();
    exit(0);
}

// Handles which print function to call based on the received command.
void handleCommand(char *command)
{
    if (strcmp(command, "help\n") == 0)
    {
        printHelp();
    }
    else if (strcmp(command, "count\n") == 0)
    {
        printCount();
    }
    else if (strcmp(command, "length\n") == 0)
    {
        printLength();
    }
    else if (strcmp(command, "history\n") == 0)
    {
        printHistory();
    }
    else if (strncmp(command, "get \n", 4) == 0)
    {
        int n = atoi(command + 4);
        printGetN(n);
    }
    else if (strcmp(command, "dips\n") == 0)
    {
        printDips();
    }
    else if (strcmp(command, "stop\n") == 0)
    {
        stop();
        exit(0);
    }
    else
    {
        char buffer[MAX_BUFFER_SIZE];
        sprintf(buffer, "The command '%s' is not recognized.\n\n", command);
        sendMessage(buffer);
        printHelp();
    }
}

void *udpServerThread(void *arg)
{
    char buffer[MAX_BUFFER_SIZE];
    char lastCommand[MAX_BUFFER_SIZE];

    while (udpThreadRunning)
    {
        int dataSize = receivePacket(buffer);
        buffer[dataSize] = '\0';

        if (strcmp(buffer, "\n") == 0)
        {
            strcpy(buffer, lastCommand);
        }
        else
        {
            strcpy(lastCommand, buffer);
        }
        handleCommand(buffer);
    }
    return NULL;
}

// Creates a thread that listens for UDP packets.
void startUdpThread(pthread_t *thread)
{
    if(udpThreadRunning)
    {
        return;
    }
    setupUdpSocket();
    udpThreadRunning = true;
    pthread_create(thread, NULL, udpServerThread, NULL);
}

// Stops the UDP thread currently running.
void stopUdpThread()
{
    udpThreadRunning = false;
    closeSocket();
}
