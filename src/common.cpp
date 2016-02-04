
#include <stdio.h>
#include <stdarg.h>

#include "common.h"

int printControlChars;
FILE *logStream;

/* Separate a line into parameters */

int isstring;
char * nextParam;

char * getParameter( char * p )
{
    char * shuffle;

    isstring = 0;
    if (p == NULL)
    {
        p = nextParam;
    }

    /* Skip over whitespace and commas */

    while ( ( * p == ' ' ) || ( * p == '\t' ) )
    {
        p++;
    }

    /* If its a " skip it and mark it as a string */

    if ( * p == '\"' )
    {
        isstring = 1;
        p++;
    }

    nextParam = p;
    shuffle = p;
    while ( ( ( ( isstring == 0 ) && ( ( * nextParam != ' ' ) ) ) || ( ( isstring == 1 ) && ( * nextParam != '\"' ) ) )
            && ( * nextParam != '\0' ) )
    {
        if ( * nextParam == '\\' )
        {
            nextParam++;
            switch ( * nextParam )
            {
                case '\0':
                    nextParam--;
                    break;
                case 'r':
                    * shuffle++ = '\r';
                    break;
                case 'n':
                    * shuffle++ = '\n';
                    break;
                case 't':
                    * shuffle++ = '\t';
                    break;
                default:
                    * shuffle++ = * nextParam;
            }
        }
        else
        {
            * shuffle = * nextParam;
            shuffle++;
        }

        nextParam++;
    }

    if ( * nextParam != '\0' ) /* More to come */
    {
        * nextParam++ = '\0';
        /* terminate the string */
        * shuffle++ = '\0';

        /* Skip over whitespace and commas */

        while ( ( * nextParam == ' ' ) || ( * nextParam == '\t' ) )
        {
            nextParam++;
        }
    }

    return p;
}

void cleanPrint( int to, char * p, int len )
{
    int i;

    if ( printControlChars ) lprintf( to, "<");

    i = 0;

    while ( i < len )
    {
        if ( * p == '\r' )
        {
            if ( printControlChars ) lprintf( to, "\\r" );
        }
        else if ( * p == '\n' )
        {
            if ( printControlChars ) lprintf( to, "\\n" );
        }
        else if ( * p == '\t' )
        {
            if ( printControlChars ) lprintf( to, "\\t" );
			else lprintf( to, ",");
        }
        else if ( * p < 0x20 )
        {
            if ( printControlChars ) lprintf( to, "0x%02x " , (*p & 0xff));
        }
        else
        {
            lprintf( to, "%c", * p );
        }
        p++;
	i++;
    }

    if ( printControlChars ) lprintf( to, ">");
}

int lprintf(int to, const char *format, ...)
{
    int i;
    va_list ap;

    va_start(ap, format);

    if (to == 0)
    {
        i = vfprintf(stderr, format, ap);
    }
    else
    {
        i = vfprintf(stdout, format, ap);
        if (logStream != NULL)
        {
            i = vfprintf(logStream, format, ap);
        }
    }

    va_end(ap);

    return i;
}

