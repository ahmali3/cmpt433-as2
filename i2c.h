// Module that uses the Zen cape's two digit 14-segment display to display the number of dips that the program
// has detected within the currently stored history.
#ifndef _I2C_H_
#define _I2C_H_
#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"

#define I2C_DEVICE_ADDRESS 0x20

#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15

#define TURN_ON "1"
#define TURN_OFF "0"

#define EXPORT_PATH "/sys/class/gpio/export"
#define PIN_61_DIRECTION_PATH "/sys/class/gpio/gpio61/direction"
#define PIN_44_DIRECTION_PATH "/sys/class/gpio/gpio44/direction"
#define GPIO_OUTPUT "out"
#define LEFT_DIGIT_PIN 61
#define RIGHT_DIGIT_PIN 44

int initI2cBus(char *bus, int address);

void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

// Begins the background thread which displays the digits
void startDisplayThread(void);

// Stops the background thread which displays the digits
void stopDisplayThread(void);

// Initializes the 14-segment i2c display
void initDisplay(void);

// Background thread that displays the digits on the 14-segment display
void *displayDigits(void *arg);

#endif
