#ifndef LIUNIX_STDLIU_H
#define LIUNIX_STDLIB_H

#include<liunix/types.h>

void delay(u32 count);
void hang();

u8 bcd_to_bin(u8 value);
u8 bin_to_bcd(u8 value);

#endif