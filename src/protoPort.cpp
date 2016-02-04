/*
 * proto type Port interface
 *
 * Peter Jansen
 * Australian Antarctic Divison 2006
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "port.hpp"

protoPort::protoPort(char *device) : port(device)
{
    type = INVALID;
}

void *protoPort::portOpen(void)
{
    strcpy(command, "proto");

    return command;
}

int protoPort::portClose(void)
{
    int ret = 0;

    return ret;
}

int protoPort::portRead(char *buffer, int len)
{
    int ret = 0;

    return ret;
}

int protoPort::portWrite(char *buffer, int len)
{
    int ret = 0;

    *buffer = *buffer; // compiler warning
    len = len; // compiler warning

    return ret;
}

int protoPort::portService(void)
{
    int ret = 0;

    return ret;
}

int protoPort::portMode(setType setting, char *set)
{
    int ret = 0;

    setting = setting; // compiler warning
    *set = *set; // compiler warning

    return ret;
}

void protoPort::print(int to)
{
    lprintf(to, "proto type (%s)", deviceString);
}
