#include "threadManager.h"
#include <unistd.h>
#include <stdio.h>
#include "lightDips.h"
#include "sampler.h"
#include "i2c.h"
#include "udp.h"
#include "periodTimer.h"
#include "threadManager.h"

int main()
{
    // Initialize the modules
    Period_init();
    runAllThreads();
    while (1)
    {
    }
    Period_cleanup();
    return 0;
}
