#ifndef LIUNIX_STDIO_H
#define LIUNIX_STDIO_H

#include<liunix/stdarg.h>
int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char*fmt, ...);

#endif