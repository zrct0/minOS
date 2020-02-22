#ifndef H_LCD1602
#define H_LCD1602
#include "os/os.h"
#include "lib/minlib.h"

void LCD02_init();
void LCD02_disp_string(uchar x,uchar y,uchar *s);

#endif