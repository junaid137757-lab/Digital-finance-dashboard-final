#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "utility.h"

void getCurrentDate(char *buffer)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    strftime(buffer, 11, "%Y-%m-%d", tm_info);
}

void clearInputBuffer(void)
{
    int c;

    while((c = getchar()) != '\n' && c != EOF)
        ;
}

void readLine(char *buffer, int size)
{
    if(fgets(buffer, size, stdin) != NULL)
    {
        size_t len = strlen(buffer);

        if(len > 0 && buffer[len - 1] == '\n')
        {
            buffer[len - 1] = '\0';
        }
        else
        {
            /* Input was longer than the buffer - discard the rest
               of the line so it doesn't spill into the next prompt. */
            clearInputBuffer();
        }
    }
    else
    {
        buffer[0] = '\0';
    }
}

int readValidInt(int *value)
{
    if(scanf("%d", value) != 1)
    {
        clearInputBuffer();
        printf("Invalid input - please enter a whole number.\n");
        return 0;
    }

    clearInputBuffer();
    return 1;
}

int readValidFloat(float *value)
{
    if(scanf("%f", value) != 1)
    {
        clearInputBuffer();
        printf("Invalid input - please enter a number.\n");
        return 0;
    }

    clearInputBuffer();
    return 1;
}

int caseInsensitiveContains(const char *haystack, const char *needle)
{
    if(*needle == '\0')
        return 1;

    for(; *haystack; haystack++)
    {
        const char *h = haystack;
        const char *n = needle;

        while(*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n))
        {
            h++;
            n++;
        }

        if(*n == '\0')
            return 1;
    }

    return 0;
}
