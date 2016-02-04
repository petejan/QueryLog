/* stats - read from standard input, and produce stats of values 
 * in the line
 */

#define _XOPEN_SOURCE /* glibc2 needs this */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>

int number;
double *b;
double *x;
char key[128];

double calc(double t[], double a[])
{
    int i;
    double avr1;
    double sd;
    double avr;
    double av;
    double slope;
    double rsq;
    double sx, sxy, sy, sx2, sy2;

    // calculate the rough average
    avr1 = 0.0;
    for(i=0;i<number;i++)
    {
        avr1 += a[i];
    }
    avr1 = avr1 / number;

    // remove bias
    for (i = 0;i<number;i++)
    {
        b[i] = a[i] - avr1;
        x[i] = t[i] - t[0];
    }

    avr = 0.0;
    for(i=0;i<number;i++)
    {
        avr += b[i];
    }
    avr = (avr / number) + avr1;

    double err;
    double inter;

    sd = 0.0;
    sx = 0.0;
    sy = 0.0;
    sxy = 0.0;
    sx2 = 0.0;
    sy2 = 0.0;

    for(i=0;i<number;i++)
    {
        err = a[i] - avr;
        sd = sd + (err * err);

        sx += x[i];
        sy += err;
        sxy += x[i] * err;
        sx2 += x[i] * x[i];
        sy2 += err * err;
    }

    sd = sqrt(sd / number);

    slope = (number * (sxy) - (sx * sy)) / ((number * sx2) - (sx * sx));
    inter = (sy - slope * sx)/number;

    rsq = ((number * sxy) - (sx * sy))/(sqrt((number * sx2) - (sx * sx)) * sqrt((number * sy2) - (sy * sy)));

    av = 0.0;
    for(i=0;i<number;i++)
    {
        err = ((x[i] * slope) + inter) - (a[i] - avr);

        av += err * err;
    }

    av = sqrt(av / number);

    time_t ts;
    struct tm *tm;
    char tbuf[128];

    ts = t[number-1];
    tm = localtime(&ts);
    strftime(tbuf, 128, "%Y-%m-%d %H:%M:%S", tm);

    printf("%s ,SD:%s , %lf, AVR, %lf, SLOPE, %lf, RSQ, %lf, ALV, %lf\n", tbuf, key, sd, avr, slope, rsq, av);

    return sd;
}

int main (int argc, char **argv)
{
    char ok;
    int optc;
    char buffer[80];
    int i;
    int j;
    int lineNo;
    char line[2048];
    int lineI;
    int bytes_read;
    int col;
    char *p;
    char *pm;
    double value;
    double *values;
    double *times;
    int valueI;
    struct tm tm;
    int quite;

    ok = 0;
    lineNo = 0;
    lineI = 0;
    number = 0;
    col = 0;
    valueI = 0;

    if (argc < 3)
    {
        printf("usage : [-q] %s <number> <col> [<key>]\n", argv[0]);

        exit(EXIT_FAILURE);
    }
    
    i = 1;
    quite = 0;

    if (argv[i][0] == '-')
    {
        if (argv[i][1] == 'q')
        {
            quite = 1;
            i++;
        }
    }

    number = strtoul(argv[i++], NULL, 10);
    col = strtoul(argv[i++], NULL, 10);

    if (argc > i)
    {
        strcpy(key, argv[i++]);
    }
    else
    {
        strcpy(key, "");
    }

    times = (double *)malloc(sizeof(double) * number);
    values = (double *)malloc(sizeof(double) * number);
    b = (double *)malloc(sizeof(double) * number);
    x = (double *)malloc(sizeof(double) * number);

    while (1)
    {
        bytes_read = read (0, buffer, sizeof buffer);
        if (bytes_read <= 0)
            break;
        
        for(i=0;i<bytes_read;i++)
        {
            if (buffer[i] == '\n')
            {
                line[lineI] = 0;

                if (quite == 0)
                {
                    printf("%s\n", line);
                }

                if (strstr(line, key) != NULL)
                {
                    /* try and convert the line if we can */
                    p = strptime(line, "%Y-%m-%d %H:%M:%S", &tm);
                    if (p != NULL)
                    {
                        times[valueI] = mktime(&tm);
                    }
                    else
                    {
                        times[valueI] = valueI;
                    }

                    /* find the value of the column in the line */
                    p = line;
                    p = strtok(line, ", ");
                    for(j=0;j<col;j++)
                    {
                        if (p != NULL)
                            p = strtok(NULL, ", ");
                        else
                            continue;
                        // printf("COL %2d: %s\n", j, p);
                    }

                    if (p != NULL)
                    {
                        value = atof(p);
                        values[valueI] = value;
                        valueI++;
                    }
                    if (valueI >= number)
                    {
                        valueI = 0;
                        calc(times, values);
                    }
                }

                lineNo++;
                lineI = 0;
            }
            else
            {
                if (lineI < 2048) 
                    line[lineI++] = buffer[i];
            }
        }
    }

    if (bytes_read == -1)
    {
        perror ("read error");
        ok = 1;
    }

    if (close (STDIN_FILENO) != 0)
        perror ("standard input");

    free(times);
    free(values);
    free(b);
    free(x);

    exit (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
