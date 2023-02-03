// // Demo application to read analog input voltage 0 on the BeagleBone
// // Assumes ADC cape already loaded by uBoot:

// #include <stdlib.h>
// #include <stdbool.h>
// #include <stdio.h>
// #include <unistd.h>
// #include "sampler.h"

// // tester code for 1.1
// int main()
// {
// 	Sampler_startSampling();
// 	sleep(3);
// 	Sampler_stopSampling();
// 	// while (true)
// 	// {
// 	// 	double reading = Sampler_getAverageReading();
// 	// 	printf("Reading: %f\n", reading);
// 	// 	sleep(1);

// 	// 	// if (rand() % 10 == 0)
// 	// 	// {
// 	// 	// 	Sampler_setHistorySize(rand() % 1000);
// 	// 	// }

// 	// 	if (Sampler_getNumSamplesTaken() > 1000)
// 	// 	{
// 	// 		Sampler_stopSampling();
// 	// 		break;
// 	// 	}
// 	// }

// 	// // Once per second, print the POT reading
// 	// while (true)
// 	// {
// 	// 	int reading = getPOTReading();
// 	// 	printf("Reading: %d\n", reading);
// 	// 	sleep(1);
// 	// }

// 		// Print the history of readings and the size of the history
// 	int length;
// 	double *history = Sampler_getHistory(&length);
// 	printf("Size of history: %d\n", length);
// 	for (int i = 0; i < length; i++)
// 	{
// 		printf("History[%d] = %f\n", i, history[i]);
// 	}
// 	free(history);
// }


// NEW PROGRAM HERE
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>


#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"

#define I2C_DEVICE_ADDRESS 0x20

#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);
// static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr);

// create the array
int topDigits[10] = {0x86, 0x02, 0x0E, 0x06, 0x8A, 0x8C, 0x8C, 0x14, 0x8E, 0x8E};
int bottomDigits[10] = {0xA1, 0x80, 0x31, 0xB0, 0x90, 0xB0, 0xB1, 0x02, 0xB1, 0x90};

// This function is the background thread that does the work of displaying the digits.

void modifyFile(FILE *file, char *value)
{
rewind(file);
fputs(value, file);
fflush(file);
}

void *displayDigits(void *arg)
{
    int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
    writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	 FILE *leftFile = fopen("/sys/class/gpio/gpio61/value", "w");
	 FILE *rightFile = fopen("/sys/class/gpio/gpio44/value", "w");

    while (1)
    {
		modifyFile(leftFile, "0");
		modifyFile(rightFile, "0");
		// Display left digit
		writeI2cReg(i2cFileDesc, REG_OUTA, bottomDigits[0]);
		writeI2cReg(i2cFileDesc, REG_OUTB, topDigits[0]);

		// Turn on left digit
		modifyFile(leftFile, "1");

        // Wait a little bit
        usleep(5000);

		// Set both pins to 0:
		modifyFile(leftFile, "0");
		modifyFile(rightFile, "0");

		// Display right digit
		writeI2cReg(i2cFileDesc, REG_OUTA, bottomDigits[1]);
		writeI2cReg(i2cFileDesc, REG_OUTB, topDigits[1]);

		// Turn on right digit
		modifyFile(rightFile, "1");

		// Wait a little bit
		usleep(5000);
    }
}

int main()
{
    // Create a background thread to display the digits
    pthread_t thread;
    pthread_create(&thread, NULL, displayDigits, NULL);

    // Wait for the background thread to finish
    pthread_join(thread, NULL);

    return 0;

	// printf("Drive display (assumes GPIO #61 and #44 are output and 1\n");
	// int i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);

	// writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	// writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);

	// // Drive an hour-glass looking character (Like an X with a bar on top & bottom)
	// writeI2cReg(i2cFileDesc, REG_OUTA, 0x2A);
	// writeI2cReg(i2cFileDesc, REG_OUTB, 0x54);

	// // Read a register:
	// unsigned char regVal = readI2cReg(i2cFileDesc, REG_OUTA);
	// printf("Reg OUT-A = 0x%02x\n", regVal);

	// // Cleanup I2C access;
	// close(i2cFileDesc);
	// return 0;
}

static int initI2cBus(char* bus, int address)
{
	int i2cFileDesc = open(bus, O_RDWR);
	if (i2cFileDesc < 0) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(-1);
	}

	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("Unable to set I2C device to slave address.");
		exit(-1);
	}
	return i2cFileDesc;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("Unable to write i2c register");
		exit(-1);
	}
}

// static unsigned char readI2cReg(int i2cFileDesc, unsigned char regAddr)
// {
// 	// To read a register, must first write the address
// 	int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
// 	if (res != sizeof(regAddr)) {
// 		perror("Unable to write i2c register.");
// 		exit(-1);
// 	}

// 	// Now read the value and return it
// 	char value = 0;
// 	res = read(i2cFileDesc, &value, sizeof(value));
// 	if (res != sizeof(value)) {
// 		perror("Unable to read i2c register");
// 		exit(-1);
// 	}
// 	return value;
// }