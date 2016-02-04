/*
 * command Port interface
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
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#endif

#include "common.h"
#include "port.hpp"

commandPort::commandPort(char *device) : port(device)
{
    type = COMMAND;
}

void *commandPort::portOpen(void)
{
    strcpy(command, "command");

    return command;
}

int commandPort::portClose(void)
{
    int ret = 0;

    return ret;
}

int commandPort::portRead(char *buffer, int len)
{
    int ret = 0;

    *buffer = *buffer; // compiler warning
    len = len; // compiler warning

    return ret;
}

int commandPort::portWrite(char *buffer, int len)
{
    int ret = 0;

    *buffer = *buffer; // compiler warning
    len = len; // compiler warning

#ifdef __linux__
    pid_t cpid;

    cpid = fork();
    if (cpid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    ret = 1;

    if (cpid == 0)
    {
        /* Code executed by child */

        char *argsp[16];
        char args[16][64];
        char *p;
        int i;

        memset(args, 0, sizeof(args));

        for(i=0;i<16;i++)
        {
            argsp[i] = args[i];
        }

        p = strtok(buffer, " ");
        i = 0;
        while (p)
        {
            strcpy(args[i], p);
            p = strtok(NULL, " ");
            i++;
        }

	// printf("commandPort::portWrite %s %s\n", deviceString, buffer, NULL);

        execlp(deviceString, deviceString, buffer);

        perror("execvp");

        exit(0); /* exit if failure */
    }
#endif
    return ret;
}

int commandPort::portService(void)
{
    int ret = 0;

    return ret;
}

int commandPort::portMode(setType setting, char *set)
{
    int ret = 0;

    setting = setting; // compiler warning
    *set = *set; // compiler warning

    return ret;
}

void commandPort::print(int to)
{
    lprintf(to, "Command (%s)", deviceString);
}
