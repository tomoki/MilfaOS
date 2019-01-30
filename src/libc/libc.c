int sprintf(char * restrict s, char* restrict format, ...)
{
    int orig_s = s;
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            if (*format == '%') {
                *(s++) = '%';
                format++;
            }
        } else {
            *s = *format;
            s++;
            format++;
        }
    }
    *s = '\0';
    return s - orig_s;
}