/*
 * Port interface
 *
 * Peter Jansen
 * Australian Antarctic Divison 2006
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __linux__
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "common.h"
#include "port.hpp"

port::port(char *device)
{
  strcpy( deviceString, device);
  type = INVALID;
}

port::~port(void)
{
    type = INVALID;
}

void *port::portOpen(void)
{
    return NULL;
}

void *port::reOpen(void *newDev)
{
    Dev = newDev;

    return Dev;
}

void *port::getDevice(void)
{
    return Dev;
}

int port::portClose(void)
{
    return -1;
}

int port::portRead(char *buffer, int len)
{
    len = len; // compiler warning
    *buffer = *buffer; // compiler waring

    return -1;
}

int port::dataAvaliable(void)
{
    return 0;
}

int port::portWrite(char *buffer, int len)
{
    len = len; // compiler warning
    *buffer = *buffer; // compiler waring

    return -1;
}

int port::portService(void)
{
    return -1;
}

int port::portMode(setType setting, char *set)
{
    setting = setting; // compiler warining
    *set = *set; // compiler warning

    return -1;
}

void port::print(int to)
{
    lprintf(to, "%s", deviceString);
}
