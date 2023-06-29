#ifndef LIUNIX_STDLIU_H
#define LIUNIX_STDLIB_H

#include<liunix/types.h>

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)

void delay(u32 count);
void hang();

u8 bcd_to_bin(u8 value);
u8 bin_to_bcd(u8 value);

u32 div_round_up(u32 num, u32 size);

#endif