/*
 * Port interface
 *
 * Peter Jansen
 * Australian Antarctic Divison 2006
 */

#ifndef PORT_HPP
#define PORT_HPP

#ifdef __linux__
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

#include <stdio.h>

class port
{
    protected:
        enum eType
        {
            INVALID,
            GPIB,
            SERIAL,
            IP,
            COMMAND,
            NIDAQ
        };
        unsigned long int timeout;
		unsigned int pace;
        int eosMode;
        int eotMode;
        enum eType type;
        void *Dev;

    public:
        enum setType
        {
            EOSMODE,
            EOTMODE,
            TIMEOUT,
            CLEAR,
			BREAK,
            CONFIG,
			PACING
        };

        char deviceString[128];
        port(char *);
        ~port(void);
        virtual void *portOpen (void);
        virtual void *reOpen(void *);
        virtual void *getDevice(void);
        virtual int portRead (char *buffer, int len);
        virtual int dataAvaliable(void);
        virtual int portWrite (char *buffer, int len);
        virtual int portClose (void);
        virtual int portMode (setType setting, char *mode);
        virtual int portService (void);
        virtual void print (int to);

};

#ifdef ENABLE_GPIB
class gpibPort: public port
{
    private:
        void GPIBCleanup(char *);
        int handle;

    public:
        gpibPort(char *);
        void *portOpen (void);
        void *reOpen(void *);
        int portRead (char *buffer, int len);
        int dataAvaliable(void);
        int portWrite (char *buffer, int len);
        int portClose (void);
        int portMode (setType setting, char *mode);
        int portService (void);
        void print (int to);
};
#endif

class serialPort: public port
{
    private:
#ifdef __WIN32__
        COMMTIMEOUTS PortTimeOut, OldTimeOut;
        DCB dcbCommPort;
        HANDLE handle;
#endif
#if __linux__
        int handle;
        struct termios term_save;
        struct termios tios;
#endif
        char config[64];
        char eosString[64];

        char tempBuffer[128];
        int rxdChars;

    public:
        serialPort(char *);
        void *portOpen (void);
        void *reOpen(void *);
        int portRead (char *buffer, int len);
        int dataAvaliable(void);
        int portWrite (char *buffer, int len);
        int portClose (void);
        int portMode (setType setting, char *mode);
        int portService (void);
};

class commandPort: public port
{
    private:
	char command[128];
    public:
        commandPort(char *);
        void *portOpen (void);
        int portRead (char *buffer, int len);
        int portWrite (char *buffer, int len);
        int portClose (void);
        int portMode (setType setting, char *mode);
        int portService (void);
        void print (int to);
};

class IpPort: public port
{
    private:
        int handle;
        struct sockaddr_in serverName;
        char deviceIp[256];
        int devicePort;
        int udp;
        struct sockaddr sendAddr;
        char eosString[64];
    public:
        IpPort(char *);
        void *portOpen (void);
        void *reOpen(void *);
        int portRead (char *buffer, int len);
        int dataAvaliable(void);
        int portWrite (char *buffer, int len);
        int portClose (void);
        int portMode (setType setting, char *mode);
        int portService (void);
        void print (int to);
};

#ifdef ENABLE_NIDAQ
class NIDAQPort: public port
{
    private:
        int handle;
    public:
        NIDAQPort(char *);
        void *portOpen (void);
        void *reOpen(void *);
        int portRead (char *buffer, int len);
        int dataAvaliable(void);
        int portWrite (char *buffer, int len);
        int portClose (void);
        int portMode (setType setting, char *mode);
        int portService (void);
        void print (int to);
};
#endif

#endif
