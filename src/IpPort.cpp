/*
 * IP Port interface
 *
 * Peter Jansen
 * Australian Antarctic Division 2006
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef __WIN32__
#include <windows.h>
#define socklen_t int
#endif

#ifdef __linux__
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#endif

#include "port.hpp"
#include "common.h"
#include "timeStamp.h"

int bytes;
int lastRx;

IpPort::IpPort(char *device) : port(device)
{
    char tmp[256];

    timeout = 1000000; // 1 sec
    type = IP;
    udp = 0;

    printf("device %s\n", device);
    strcpy(deviceIp, strtok(device, ":"));
    if (strcmp("UDP", deviceIp) == 0)
    {
        strcpy(deviceIp, strtok(NULL, ":"));
        udp = 1;
    }
    strcpy(tmp, strtok(NULL, ":"));

    devicePort = atoi(tmp);
}

void *IpPort::portOpen(void)
{
    struct hostent *hostinfo;

#ifdef __WIN32__
    int iResult;
    WSADATA wsaData;
#endif

#ifdef __WIN32__
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        fprintf(stderr, "WSAStartup failed: %d\n", iResult);
        return NULL;
    }
#endif

    // open the socket
    if (udp == 1)
    {
        handle = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        //handle = socket(AF_INET, SOCK_DGRAM, 0);
    }
    else
    {
        handle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    }
    printf("Socket %d\n", handle);

    if (handle == INVALID_SOCKET)
    {

#ifdef __WIN32__
		wprintf(L"socket function failed with error = %d\n", WSAGetLastError() );

        WSACleanup();
#endif
        fprintf(stderr, "Cound not open Socket\n");
        return NULL;
    }

    serverName.sin_family = AF_INET;
    serverName.sin_addr.S_un.S_addr = inet_addr(deviceIp);


//    hostinfo = gethostbyname (deviceIp);
//    if (hostinfo == NULL)
//    {
//#ifdef __WIN32__
//        WSACleanup();
//#endif
//        fprintf (stderr, "Unknown host %s\n", deviceIp);
//        return NULL;
//    }
//    memmove((char *)&serverName.sin_addr, (char *)&hostinfo->h_addr, hostinfo->h_length);

    serverName.sin_port = htons (devicePort);

    //serverName.sin_addr = *(struct in_addr *) hostinfo->h_addr;

	printf("h_addr %d\n", serverName.sin_addr);
	printf("h_port %d %d\n", serverName.sin_port, devicePort);

    if (udp == 0)
    {
        if (connect(handle, (const struct sockaddr *) &serverName, sizeof(serverName)) < 0)
        {
#ifdef __WIN32__
            WSACleanup();
#endif
            fprintf(stderr, "Could not connect to socket\n");
            return NULL;
        }
        if (bind(handle, (const struct sockaddr *) &serverName, sizeof(serverName)) == SOCKET_ERROR)
        {
#ifdef __WIN32__
            WSACleanup();
#endif
            fprintf(stderr, "Could not bind to socket\n");
            return NULL;
        }
		printf("Bind %d\n", handle);
    }
    else
    {	
		int opt = 1;
		int r ;
		if ((r = setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt))) != SOCKET_ERROR)
		{
			printf("Set SO_REUSEADDR: ON\n");
		}
		else
		{
#ifdef __WIN32__
			int e = WSAGetLastError();

			printf("setsocketopt error %d errno %d\n", r, e);
#endif
#ifdef __linux__
			printf("setsocketopt error %d \n", r);
#endif

		}
		serverName.sin_family = AF_INET;
		serverName.sin_port = htons(devicePort);
		serverName.sin_addr.s_addr = htonl(INADDR_ANY);
//	    serverName.sin_addr.S_un.S_addr = inet_addr(deviceIp);

	    iResult = bind(handle, (SOCKADDR *) & serverName, sizeof (serverName));
	    if (iResult != 0)
	    {
	        printf("bind failed with error %d\n", WSAGetLastError());
	        return NULL;
	    }
		printf("Bind %d\n", iResult);
    }

#ifdef __linux__
    fcntl(handle, F_SETFL, O_NONBLOCK);
#endif
#ifdef __WIN32__
    u_long iMode = 1;
    //ioctlsocket(handle, FIONBIO, &iMode);
#endif

    Dev = &handle;

    bytes = 0;
    lastRx = 0;

    return Dev;
}

void *IpPort::reOpen(void *newDev)
{
    Dev = newDev;

    return Dev;
}

int IpPort::portClose(void)
{
#ifdef __linux__
    close(handle);
#endif
#ifdef __WIN32__
    closesocket(handle);
    WSACleanup();
#endif

    return 0;
}
char rxBuf[1024];

int IpPort::portRead(char *buffer, int len)
{
    timeT now;
    timeT timeoutTime;
    int ret = -1;
    struct timeval tv;
    int fd;
    int i = 0;
    int j = 0;
    int sad = sizeof(sendAddr);
    char rxChar;
    int rxNew;

    //len = len; // compiler warning

    buffer[0] = '\0';

    now = getTime();
    timeoutTime = now + timeout;

    printf("\nIpPort::portRead bytes %d lastRx %d\n", bytes, lastRx);

    while ((now < timeoutTime) && (ret == -1))
    {
        rxNew = recvfrom(handle, &rxChar, 1, MSG_PEEK, &sendAddr, (socklen_t *)&sad);
        printf("IpPort::portRead recvfrom from new %d\n", rxNew);

        //if (rxNew > 0)
        {
            lastRx = bytes;
            i = recvfrom(handle, &rxBuf[bytes], sizeof(rxBuf) - bytes, 0, &sendAddr, (socklen_t *)&sad);
            printf("IpPort::portRead recvfrom chars %d\n", i);

            if (i > 0)
            {
                bytes += i;
            }
            //printf("\nIpPort::portRead %d bytes %d\n", i , bytes);
            rxNew = 0;
        }
        // printf("\nIpPort::portRead %d bytes %d\n", i , bytes);
        while (bytes > 0)
        {
            buffer[j++] = rxBuf[lastRx];
            bytes --;
            if (rxBuf[lastRx] == eosMode)
            {
                ret = bytes;
            }
            lastRx++;
        }

        now = getTime();
    }
    printf("IpPort::portRead finished %d bytes %d\n", j , bytes);

    buffer[j] = '\0';

    return ret;

}

int IpPort::portWrite(char *buffer, int len)
{
    int nbytes;

	if (udp == 0)
	{
		nbytes = send( handle, buffer, len, 0 );
		if (nbytes < 0)
		{
			fprintf(stderr, " Ip Connection Write error\n");
#ifdef __WIN32__
			printf("socket function failed with error = %d\n", WSAGetLastError() );
#endif
			return -1;
		}
	}
	else
	{
		//printf("h_addr %d\n", serverName.sin_addr);
		//printf("h_port %d %d\n", serverName.sin_port, devicePort);
		nbytes = sendto( handle, buffer, len, 0 , (const sockaddr *) &serverName, sizeof(serverName));

        //sockaddr_in Recv;
        //Recv.sin_family = AF_INET;
        //Recv.sin_port = htons(3000);
        //Recv.sin_addr.s_addr = inet_addr("127.0.0.1");

		//nbytes = sendto( handle, buffer, len, 0 , (const sockaddr *) &Recv, sizeof(Recv));
		if (nbytes < 0)
		{
			fprintf(stderr, " Ip Connection Write error\n");
#ifdef __WIN32__
			printf("socket function failed with error = %d\n", WSAGetLastError() );
#endif
			return -1;
		}
	}
    return 0;
}

int IpPort::dataAvaliable(void)
{
    int i;
    char rxChar;
    int sad = sizeof(sendAddr);

    if (bytes > 0)
    {
        printf("IpPort:avaliable (prev) %d\n", bytes);
        return bytes;
    }

    i = recvfrom(handle, &rxChar, 1, MSG_PEEK, &sendAddr, (socklen_t *)&sad);
    //i = recvfrom(handle, &rxChar, 1, 0, &sendAddr, (socklen_t *)&sad);
    //printf("IpPort::dataAvaliable recvfrom from %d\n", i);

    if (i == SOCKET_ERROR)
    {
    	int e = WSAGetLastError();
    	if (e == WSAEMSGSIZE)
    	{
    		return 1;
    	}
    	printf("Socket error = %d\n", WSAGetLastError());

    	return 0;
    }
    if (i > 0)
    {
		printf("IpPort:avaliable (new) %d\n", i);

        return i;
    }
    return 0;
}

int IpPort::portService(void)
{
    return 0;
}

int IpPort::portMode(setType setting, char *set)
{
  switch (setting)
  {
      case TIMEOUT:
          timeout = strtoul(set, NULL, 10) * 1000L;
          break;
      case EOSMODE:
          eosMode = *set;
          break;
  }
}

void IpPort::print(int to)
{
    if (udp == 0)
    {
        lprintf(to, "%s:%d", deviceIp, devicePort);
    }
    else
    {
        struct in_addr *addr;

        addr = (in_addr *)&sendAddr.sa_data[2];

        lprintf(to, "%s:%d from %7s", deviceIp, devicePort, inet_ntoa(*addr));
    }
}
