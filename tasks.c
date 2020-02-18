#include "os.h"


void task1(){	
	uchar i = 0;
	while(1){		
		P1 = i++;
		sleep(100);		
	}
}

void task2(){	
	uchar i = 0;
	while(1){		
		P3 = i++;
		sleep(100);		
	}
}

void addTasks(){
	addTask(task1);
	addTask(task2);
}