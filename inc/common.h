
#include <stdio.h>

extern int printControlChars;
extern FILE *logStream;

void cleanPrint( int to, char * p , int len);
char * getParameter( char * p );

int lprintf(int to, const char *format, ...);

typedef unsigned long long int timeT;

