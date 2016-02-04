/*
 * query.cpp - a class for a single query record
 *
 * Peter Jansen
 * Australian Antarctic Division
 */

#ifndef QUERY_HPP
#define QUERY_HPP
#include "port.hpp"
#include "timeStamp.h"

class query
{
    private:
        timeT first;
        timeT last;
        timeT nextRuntime;

    public:
        port *device;
        int receiveWait;
        int send;
        int sendClear;
        int nmea;
        unsigned long int timeout;
        timeT period;
        int number;
        char message[1000];
        int messageLength;
        char header[100];
        int sent;
        int received;
        int sumErrors;
        int checked;
        int timeouts;
        char args[16][256];
        int srq;
        int srqReq;

        query(void);
        void setPeriod(char *p);
        void setFirstTime(char *p, timeT time);
        void setLastTime(char *p, timeT time);
        int finished(timeT time);
        void timeStamp(int to, timeT time);
        void print(int to);
        int thisRecord(timeT timeNow);
};
#endif
