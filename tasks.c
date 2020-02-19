#include "os.h"


void task1(){	
	uchar i, j;
	i = 1;
	for(j=0x00;j<0x08;j++){
		i = i << 1;	
		P1 = i;
		sleep(1000);		
	}
	P1 = 0xFF;
}

void task2(){	
	uchar i = 1;
	while(1){	
		i = i << 1;
		if(i==0){
			i=1;
		}
		P2 = i;
		sleep(100);		
	}
}

void addTasks(){
	addTask(task1);
	addTask(task2);
}