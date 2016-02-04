
#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char *argv[])
{

    int iResult = 0;

    WSADATA wsaData;

    SOCKET RecvSocket;
    sockaddr_in RecvAddr;

    unsigned short Port = 10000;

    if (argc > 1)
    {
    	Port = strtoul(argv[1], NULL, 10);
    }

    wprintf(L"Using Port %d\n", Port);

    char RecvBuf[1024];
    int BufLen = 1024;

    sockaddr_in SenderAddr;
    int SenderAddrSize = sizeof (SenderAddr);

    //-----------------------------------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR)
    {
        wprintf(L"WSAStartup failed with error %d\n", iResult);
        return 1;
    }
    //-----------------------------------------------
    // Create a receiver socket to receive datagrams
    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RecvSocket == INVALID_SOCKET)
    {
        wprintf(L"socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

	int opt = 1;
	int r ;
	if ((r = setsockopt(RecvSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt))) != SOCKET_ERROR)
	{
		wprintf(L"Set SO_REUSEADDR: ON\n");
	}
	else
	{
		int e = WSAGetLastError();

		printf("setsocketopt error %d errno %d\n", r, e);
	}

    //-----------------------------------------------
    // Bind the socket to any address and the specified port.
#ifndef BIND
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    //RecvAddr.sin_addr.s_addr = inet_addr("localhost");

    iResult = bind(RecvSocket, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr));
    if (iResult != 0)
    {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        return 1;
    }
#endif

#ifdef CONNECT
    //----------------------
    // Connect to server.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    iResult = connect(RecvSocket, (SOCKADDR *) & RecvAddr, sizeof (RecvAddr));
    if (iResult == SOCKET_ERROR)
    {
        wprintf(L"connect function failed with error: %ld\n", WSAGetLastError());
        iResult = closesocket(RecvSocket);
        if (iResult == SOCKET_ERROR)
            wprintf(L"closesocket function failed with error: %ld\n", WSAGetLastError());

        WSACleanup();
        return 1;
    }
#endif

    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams
    // on the bound socket.
    wprintf(L"Receiving datagrams...\n");
    iResult = 0;
    SYSTEMTIME st;
    struct in_addr *addr;

    while(iResult != SOCKET_ERROR)
    {
		iResult = recvfrom(RecvSocket, RecvBuf, BufLen, 0, (SOCKADDR *) & SenderAddr, &SenderAddrSize);
		if (iResult == SOCKET_ERROR)
		{
			wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
		}

		GetLocalTime(& st);

        addr = (in_addr *)&SenderAddr.sin_addr;

		if (iResult > 0)
		{
			RecvBuf[iResult] = 0;
			for(int i=0;i<iResult;i++)
			{
				if (RecvBuf[i] == '\n')
				{
					RecvBuf[i] = 0;
				}
			}
			wprintf( L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
					st.wYear, st.wMonth, st.wDay,
					st.wHour, st.wMinute,
					st.wSecond, st.wMilliseconds );

			printf(",From ,%7s ,Received ,%d ,%s\n", inet_ntoa(*addr), iResult, RecvBuf);
		}
    }

    //-----------------------------------------------
    // Close the socket when finished receiving datagrams
    wprintf(L"Finished receiving. Closing socket.\n");
    iResult = closesocket(RecvSocket);
    if (iResult == SOCKET_ERROR)
    {
        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    //-----------------------------------------------
    // Clean up and exit.
    wprintf(L"Exiting.\n");
    WSACleanup();
    return 0;
}

