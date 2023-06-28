#ifndef LIUNIX_CONSOLE_H
#define LIUNIX_CONSOLE_H

#include<liunix/types.h>

void console_init();
void console_clear();
void console_write(char* buf, u32 count);

#endif