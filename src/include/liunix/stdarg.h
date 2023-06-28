#ifndef LIUNIX_STDARG_H
#define LIUNIX_STDARG_H

typedef char* va_list;

#define va_start(ap, v) (ap = (va_list)&v + sizeof(char*))
// 将arg_ptr移到下一个参数的地址（栈中）

#define va_arg(ap, t) (*(t*)((ap += sizeof(char*)) - sizeof(char *)))
// 获取当前位置参数值，同时arg_ptr移动到下一个参数的地址


#define va_end(ap) (ap = (va_list)0)

#endif