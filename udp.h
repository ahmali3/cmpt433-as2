#ifndef _UDP_H_
#define _UDP_H_

#define PORT 12345
#define MAX_BUFFER_SIZE 1500

// function to set up the UDP socket
void setupSocket(void);

// function to send a UDP packet
void sendPacket(char *buffer);

// function to close the UDP socket
void closeSocket(void);

// Create a thread to listen for UDP packets
void startUdpThread();

// Stop the thread listening for UDP packets
void stopUdpThread();

#endif