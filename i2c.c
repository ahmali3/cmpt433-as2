#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdbool.h>
#include "i2c.h"
#include "utility.h"
#include "sampler.h"
#include "lightDips.h"
#include "threadManager.h"

// Arrays of the hex values to write to the top and bottom segments of the 14-segment display
int topDigits[10] = {0x86, 0x02, 0x0E, 0x06, 0x8A, 0x8C, 0x8C, 0x14, 0x8E, 0x8E};
int bottomDigits[10] = {0xA1, 0x80, 0x31, 0xB0, 0x90, 0xB0, 0xB1, 0x02, 0xB1, 0x90};

void modifyFile(FILE *file, char *value)
{
	if (!file)
	{
		printf("Error modifying file. File is NULL.\n");
		exit(1);
	}
	rewind(file);
	fputs(value, file);
	fflush(file);
}

// Background thread that displays the digits on the 14-segment display
void *displayDigits(void *arg)
{
	int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
	initDisplay();
	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	FILE *leftDigitFile = fopen("/sys/class/gpio/gpio61/value", "w");
	FILE *rightDigitFile = fopen("/sys/class/gpio/gpio44/value", "w");

	int numDips = 0;
	int leftDigit = 0;
	int rightDigit = 0;
	long long timeElapsedInMs = 0;
	long long start = getTimeInMs();
	long long end = getTimeInMs();

	while (allThreadsRunning)
	{
		end = getTimeInMs();
		timeElapsedInMs = end - start;
		if (timeElapsedInMs >= 100)
		{
			start = getTimeInMs();
			timeElapsedInMs = 0;
			numDips = getDipCount();
			if (numDips > 99)
				numDips = 99;

			leftDigit = numDips / 10;
			rightDigit = numDips % 10;
		}

		// Set both pins off
		modifyFile(leftDigitFile, TURN_OFF);
		modifyFile(rightDigitFile, TURN_OFF);

		// Display left digit
		writeI2cReg(i2cFileDesc, REG_OUTA, bottomDigits[leftDigit]);
		writeI2cReg(i2cFileDesc, REG_OUTB, topDigits[leftDigit]);

		// Turn on left digit
		modifyFile(leftDigitFile, TURN_ON);

		usleep(5000);

		// Set both pins off
		modifyFile(leftDigitFile, TURN_OFF);
		modifyFile(rightDigitFile, TURN_OFF);

		// Display right digit
		writeI2cReg(i2cFileDesc, REG_OUTA, bottomDigits[rightDigit]);
		writeI2cReg(i2cFileDesc, REG_OUTB, topDigits[rightDigit]);

		// Turn on right digit
		modifyFile(rightDigitFile, TURN_ON);

		usleep(5000);
	}
	// Set both pins off before exiting
	modifyFile(leftDigitFile, TURN_OFF);
	modifyFile(rightDigitFile, TURN_OFF);
	fclose(leftDigitFile);
	fclose(rightDigitFile);
	return NULL;
}

// Begins the background thread which displays the digits
void startDisplayThread(pthread_t *thread)
{
	pthread_create(thread, NULL, displayDigits, NULL); // this will call displayDigits in a background thread
}

// Stops the background thread which displays the digits
void stopDisplayThread()
{
    allThreadsRunning = false;
}

int initI2cBus(char *bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0)
	{
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0)
	{
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2)
	{
		perror("Unable to write i2c register");
		exit(-1);
	}
}

// Exports the left-digit pin on the 14-segment display
void exportDisplayPin61(void)
{
	int export61 = -1;
	while (export61 != 0)
	{
		export61 = runCommand("echo 61 > /sys/class/gpio/export");
	}
}

// Exports the right-digit pin on the 14-segment display
void exportDisplayPin44(void)
{
	int export44 = -1;
	while (export44 != 0)
	{
		export44 = runCommand("echo 44 > /sys/class/gpio/export");
	}
}

// Initializes the 14-segment i2c display
void initDisplay(void)
{
	int output61 = -1;
	int output44 = -1;
	while (output61 != 0 && output44 != 0)
	{
		runCommand("config-pin P9_18 i2c");
		runCommand("config-pin P9_17 i2c");

		output61 = runCommand("echo out > /sys/class/gpio/gpio61/direction");
		if (output61 != 0)
			exportDisplayPin61();
		output44 = runCommand("echo out > /sys/class/gpio/gpio44/direction");
		if (output44 != 0)
			exportDisplayPin44();
	}
}
