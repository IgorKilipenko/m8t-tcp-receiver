#include <stdio.h>
#include <SD.h>
#include "io_helpers.h"

void generateFileName(char *name)
{
    const size_t max_nums = 999;
    bool reset = false;
    const char prefix[] = "raw_";
    const char ext[] = ".ubx";
    size_t number = 1;
    while (number != 0)
    {
        if (number > max_nums)
        {
            number = 1;
            reset = true;
        }
        sprintf(name, "%s%i%s", prefix, number, ext);
        if (SD.exists(name) && !reset)
        {
            // file exist
            number++;
        }
        else
        {
            // next number
            return;
        }
    }
}