#ifndef _UDP_H_
#define _UDP_H_

#define PORT 12345
#define MAX_BUFFER_SIZE 1500

// Sends a message through the UDP socket.
void sendMessage(char *message);

// Sets up the UDP socket by creating it and binding it to an address.
void setupUdpSocket(void);

// Receives a UDP packet.
int receivePacket(char *buffer);

// Closes the UDP socket.
void closeSocket(void);

// Prints the help message.
void printHelp(void);

// Prints the number of samples taken by the sampler.
void printCount(void);

// Prints the length of the sampler history.
void printLength(void);

// Prints the entire history of samples taken by the sampler.
void printHistory(void);

// Prints the last n samples taken by the sampler.
void printLastN(int n);

// Prints the number of dips taken by the sampler.
void printDips(void);

// Closes all threads and sockets and exits.
void stop(void);

// Handles which print function to call based on the received command.
void handleCommand(char *command);

// Creates a thread that listens for UDP packets.
void startUdpThread();

// Stops the UDP thread currently running.
void stopUdpThread();

void *udpServerThread(void *arg);

#endif
