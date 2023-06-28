#ifndef LIUNIX_RTC_H
#define LIUNIX_RTC_H

u8 cmos_read(u8 addr);
void cmos_write(u8 addr, u8 value);

#endif