#include <stdarg.h>
#include <stdio.h>

int int2str(char* str, int n)
{
    // calculate how many numbers n has
    int a = n;
    int k = 1;
    while (a >= 10) {
        a /= 10;
        k++;
    }

    // insert characters backward
    int ret = k;
    while (k > 0) {
        str[k-1] = '0' + (n % 10);
        n /= 10;
        k--;
    }
    return ret;
}

int sprintf(char * restrict s, char* restrict format, ...)
{
    va_list args;
    va_start(args, format);
    int orig_s = s;
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == '%') {
                *(s++) = '%';
                format++;
            } else if(*format == 'd') {
                int* n = (int*) args;
                s += int2str(s, *n);
                format++;
            }
        } else {
            *s = *format;
            s++;
            format++;
        }
    }
    *s = '\0';
    int ret = s - orig_s;
    va_end(args);
    return ret;
}
