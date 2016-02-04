#ifdef __BORLANDC__
  #pragma argsused
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#define SAMPLES 200000
#define PARAMS 10
#define KEYS 100

struct sample
{
  time_t t;
  int key;
  int nParams;
  double params[PARAMS];
};



struct keys
{
  int count;
  char key[128];
  double average[PARAMS];
  double stdev[PARAMS];
  double max[PARAMS];
  double min[PARAMS];
  int sCount[PARAMS];
  int nParams;
};



char * nextParam;
int isstring;

double max( double x, double y )
{
  if ( x >= y )
  {
    return x;
  }
  return y;
}

double min( double x, double y )
{
  if ( x <= y )
  {
    return x;
  }
  return y;
}


char * getFirstParam( char * p )
{
  char * shuffle;

  isstring = 0;

  /* Skip over whitespace and commas */

  while ( ( * p == ' ' ) || ( * p == '\t' ) || ( * p == ',' ) )
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
  while ( ( ( ( isstring == 0 ) && ( * nextParam != ',' ) ) || ( ( isstring == 1 ) && ( * nextParam != '\"' ) ) )
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

    while ( ( * nextParam == ' ' ) || ( * nextParam == '\t' ) || ( * nextParam == ',' ) )
    {
      nextParam++;
    }
  }

  return p;
}

int readLine( char * line )
{
  int i;
  char ch;

  i = 0;
  do
  {
    ch = getchar();

    line[i++] = ch;
  }
  while ( ch != '\n' && ch != '\r' && ( ch != EOF ) );

  if ( i > 0 )
  {
    line[i - 1] = 0;
    i--;
  }
  else
    line[0] = 0;

  if ( ch == EOF )
  {
    return -1;
  }

  return i;
}

int main( int argc, char * argv[] )
{
  struct keys list[KEYS];
  struct sample * samples;
  int i, j, k, n;
  char key[128];
  char lineparams[20] [30];
  char linekey[128];
  char origline[1024];
  char line[1024];
  int nsamples, readSamples;
  char * p;
  int firstRxParam;
  time_t startTime;
  int startSample;
  struct tm t;
  time_t sampleTime = 0;
  char sTime[128];

  samples = ( sample * ) malloc( sizeof( sample[SAMPLES] ) );

  for ( i = 0; i < KEYS; i++ )
  {
    list[i].key[0] = '\0';
    list[i].count = 0;
    list[i].nParams = 0;
  }

  for ( j = 0; j < SAMPLES; j++ )
  {
    samples[j].t = 0;
    samples[j].key = -1;
    for ( k = 0; k < PARAMS; k++ )
    {
      samples[j].params[0] = 0.0;
    }
  }

  if ( argc < 3 )
  {
    printf( "Usage: %s <key> <samples>\n", argv[0] );

    exit( -1 );
  }

  strcpy( key, argv[1] );

  nsamples = strtoul( argv[2], NULL, 10 );

  //  printf( "key : '%s' samples %d\n", key, samples );

  do
  {
    memset( origline, '\0', sizeof( origline ) );
    memset( line, '\0', sizeof( line ) );
    memset( linekey, '\0', sizeof( linekey ) );
    i = readLine( origline );
    strcpy( line, origline);

    //    printf( "line length %d : %s\n", i, line );

    if ( i > 1 )
    {
      p = getFirstParam( line );

      // Read the timestamp
      int year;
      n = sscanf( p, "%d-%d-%d %d:%d:%d.%*d", & year, & t.tm_mon, & t.tm_mday, & t.tm_hour, & t.tm_min, & t.tm_sec );

      t.tm_year = year - 1900;

      if ( n == 6 )
      {
        sampleTime = mktime( & t );
      }
      else
      {
        printf( "Badly formatted date : %s\n", p );
      }

      strftime( sTime, sizeof( sTime ), "%Y-%m-%d %H:%M:%S", & t );
      //      printf("Time : %s\n", sTime);

      // separate the line params into parts and put into the line params array

      // clear the parameters
      for ( j = 0; j < 20; j++ )
      {
        lineparams[j] [0] = '\0';
      }

      j = 0;

      while ( (* p != 0) && (j < 20))
      {
        strcpy( lineparams[j], p );
        //        printf(" param %d : %s\n", j, p);
        j++;
        p = getFirstParam( nextParam );
      }

      // if we get more than two params, check if its a new key
      if ( j > 2 )
      {
        if ( strcmp( lineparams[2], "Rx" ) == 0 )
        {
          strcpy( linekey, lineparams[1] );
          firstRxParam = 3;
        }
        else
        {
          strcpy( linekey, lineparams[1] );
          strcat( linekey, lineparams[3] );
          firstRxParam = 5;
        }

        //        printf("line key : %s\n", linekey);

        int next = -1;
        for ( n = 0; n < KEYS; n++ )
        {
          if ( ( list[n].key[0] == '\0' ) && ( next == -1 ) )
          {
            next = n;
          }
          else if ( strcmp( linekey, list[n].key ) == 0 )
          {
            break;
          }
        }

        // if key not found in existing list add a new one
        if ( ( n == KEYS ) && ( next != -1 ) )
        {
          strcpy( list[next].key, linekey );
          n = next;
        }
        //        printf("Key %d %d\n", n, readSamples);
        if ( n != KEYS )
        {
          samples[readSamples].key = n;
          samples[readSamples].t = sampleTime;
          samples[readSamples].nParams = 0;

          int lp;
          double f;
          for ( lp = firstRxParam; lineparams[lp] [0] != '\0'; lp++ )
          {
            int pams = 0;
            f = 0.0;
            pams = sscanf( lineparams[lp], "%lf", & f );

            // special for DigiQuartz values that start with *0001
            if ( strncmp( lineparams[lp], "*0001", 4 ) == 0 )
            {
              pams = sscanf( lineparams[lp], "*0001%lf", & f );
            }

            if ( pams == 1 )
            {
              samples[readSamples].params[lp - firstRxParam] = f;
              samples[readSamples].nParams++;
            }
          }
          // printf("Key %d %d %d\n", n, readSamples, samples[readSamples].nParams);

          list[n].count++;
          readSamples++;
        }
      } // j > 2
    } // i > 1

    // check if this line contains the key we're separating with
    if ( (strstr( origline, key ) != NULL ) || (i == -1) )
    {
      startTime = sampleTime - nsamples;

      struct tm * tp = localtime( & startTime );
      strftime( sTime, sizeof( sTime ), "%Y-%m-%d %H:%M:%S", tp );
      printf( "Key Found %4d Time %s, %s\n", readSamples, sTime, linekey );

      /* Key Found, calculate the stats */

      /* clear the stats first */
      for ( int x = 0; x < KEYS; x++ )
      {
        for ( k = 0; k < PARAMS; k++ )
        {
          list[x].sCount[k] = 0;
          list[x].average[k] = 0.0;
          list[x].stdev[k] = 0.0;
          list[x].max[k] = DBL_MIN;
          list[x].min[k] = DBL_MAX;
        }
      }

      for ( startSample = readSamples - 1; ( startSample >= 0 ) && ( samples[startSample].t > startTime ); startSample-- )
      {
        int thisKey;
        double param;

        thisKey = samples[startSample].key;
        list[thisKey].nParams = samples[startSample].nParams;

        /* compute the average, max, min */
        for ( k = 0; k < PARAMS; k++ )
        {
          param = samples[startSample].params[k];
          // printf("sample %d key %d param %d : %f\n", startSample, thisKey, k, param);

          list[thisKey].sCount[k] ++;
          list[thisKey].average[k] += param;

          list[thisKey].max[k] = max( list[thisKey].max[k], param );
          list[thisKey].min[k] = min( list[thisKey].min[k], param );
        }
      }

      int l;

      for ( k = 0; k < KEYS; k++ )
      {
        for ( l = 0; l < PARAMS; l++ )
        {
          if ( list[k].sCount[l] > 0 )
          {
            list[k].average[l] = list[k].average[l] / list[k].sCount[l];
          }
          list[k].stdev[l] = 0.0;
        }
      }

      /* now the standard deviation */
      for ( startSample = readSamples - 1; ( startSample >= 0 ) && ( samples[startSample].t > startTime ); startSample-- )
      {
        int thisKey;
        double param;

        thisKey = samples[startSample].key;
        list[thisKey].nParams = samples[startSample].nParams;

        for ( k = 0; k < PARAMS; k++ )
        {
          param = samples[startSample].params[k];
          if ( list[thisKey].sCount[k] > 0 )
          {
            list[thisKey].stdev[k] += ( param - list[thisKey].average[k] ) * ( param - list[thisKey].average[k] );
          }
        }
      }

      for ( k = 0; k < KEYS; k++ )
      {
        for ( l = 0; l < PARAMS; l++ )
        {
          if ( list[k].sCount[l] > 0 )
          {
            list[k].stdev[l] = sqrt( list[k].stdev[l] / list[k].sCount[l] );
          }
        }
      }

      /* print out the results */
      for ( n = 0; n < KEYS; n++ )
      {
        if ( list[n].key[0] != '\0' )
        {
          printf( "%s, %-40s, %4d, ", sTime, list[n].key, list[n].sCount[0] );
          for ( k = 0; k < list[n].nParams; k++ )
          {
            printf( "%12lf, ", list[n].average[k] );
          }
          printf( "Stdev, ");
          for ( k = 0; k < list[n].nParams; k++ )
          {
            printf( "%12lf, ", list[n].stdev[k] );
          }

          printf( "\n" );
        }
      }
    } // key found
  }
  while ( i > 0 );

  return 0;
}
