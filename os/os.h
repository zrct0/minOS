#ifndef H_OS
#define H_OS
#pragma code small
#include <reg52.h>

#define uchar unsigned char
#define uint unsigned int

//任务管理
void addTask(uint task);
void lock();
void unlock();

//堆内存管理
void malloc(uchar size_t, uchar xdata ** p);
void free(uchar xdata ** p);
void GC();

//系统变量
void sleep(uchar ms);
uchar getDataMemoryUnit(uchar address);
uchar getXDataMemoryUnit(uint address);

#endif