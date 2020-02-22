#include "os.h"
#include "driver/LCD1602.h"


void display(uchar x, uchar y);

void task1(){	
	uchar xdata * s1;
	uchar xdata * s2;
	uchar xdata * s3;
	malloc(4, &s1);	
	malloc(4, &s2);	
	malloc(4, &s3);	
	int2string(s1, 111);
	int2string(s2, 222);
	int2string(s3, 333);
	free(&s1);
	free(&s2);
	free(&s3);
	GC();
	display(9, 1);
}


void task2(){
	display(0, 0);
}

void task3(){
	display(0, 1);
}

void display(uchar x, uchar y){
	uint data n = 0;
	uchar xdata * s1;
	uchar xdata * s2;
	malloc(12, &s1);
  malloc(8, &s2);	
	LCD02_init();	
	while(1){				
		n += y + 1;		
		strcopy(s1, "ITRP:");
		int2string(s2, getDataMemoryUnit(0x31) + 1);
		strcat1(s2, "  ");
		strcat1(s1, s2);		
		LCD02_disp_string(x, y, s1);			
	}
}

void addTasks(){
	addTask(task1);	
	addTask(task2);
	addTask(task3);
}