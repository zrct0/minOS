#include "os.h"

uchar RN_TASK_IDX _at_ 0x30; 		  //当前任务下标
uchar ITRPT_COUNT _at_ 0x31;			//中断次数
uchar xdata *pTAK_RM_STK _at_ 0x32;//任务RAM栈指针	
uchar data *p_JCB _at_ 0x34;		  //JCB地址栈指针

#define C_MAX_TASK_COUNT 0x04		      //最多任务数
#define C_JCB_START 0x50		      //JCB起始地址
#define C_TASKs_RAM_START 0x0000	//任务RAM起始地址	
#define C_sgTASK_SP_SRART 0x80		//任务栈数据起始地址
#define C_MAIN_SP_START 0x70			//默认栈数据起始地址
#define C_JCB_CAPACITY 0x08	      //JCB容量
#define C_TASKs_RAM_CAPACITY 0x20 //单个任务最大容量
#define C_sgTASK_SP_CAPACITY 0x20	//单个任务栈最大容量

#define JCB_DESTORY 0x00
#define JCB_BLOCK 0x01
#define JCB_RUN 0x02

uchar data Cache_ACC _at_ 0x40;        //ACC临时变量
uchar data Cache_TaskEnter_L _at_ 0x41;//任务人口地址L临时变量
uchar data Cache_TaskEnter_H _at_ 0x42;//任务人口地址H临时变量
uchar data i _at_ 0x43;                //临时变量i

extern void addTasks();
extern void pushTaskData();



void OS_Main(){
	addTasks();
}


void OS_taskDispatch(){

	//寄存器入栈保护	
	for(i=0x08;i<0x10;i++){		
		//MOV (C_TASKs_RAM_S + D_RN_TASK_IDX*C_sgTAK_RM_CT) + D_pTAK_RM_STK,A
		*((uchar xdata *)(C_TASKs_RAM_START + RN_TASK_IDX*C_TASKs_RAM_CAPACITY + pTAK_RM_STK++)) = *((uchar data*)i);
		
	}
	
	//寻找下一个任务的下标
	i = 0;
	for(p_JCB = C_JCB_START; p_JCB < C_JCB_START + C_MAX_TASK_COUNT * C_JCB_CAPACITY; p_JCB += C_JCB_CAPACITY){		
		if(*p_JCB  == JCB_BLOCK && i != RN_TASK_IDX){
			//上一个任务变为阻塞状态
			*((uchar data*)(C_JCB_START + RN_TASK_IDX * C_JCB_CAPACITY)) = JCB_BLOCK;
			//下一任务变成运行态
			RN_TASK_IDX = i;
			*(p_JCB++) = JCB_RUN;			
			break;
		}	
		++i;
	}
	pTAK_RM_STK = C_TASKs_RAM_START + RN_TASK_IDX * C_TASKs_RAM_CAPACITY;//任务数据栈指针	
	//恢复ACC	
	Cache_ACC = *(pTAK_RM_STK++);
	//恢复PSW
	*((uchar*)PSW) = *(pTAK_RM_STK++) & 0xE7;
	//获取入口地址
	//先从任务RAM中获取		
	Cache_TaskEnter_L = *(pTAK_RM_STK++);
	Cache_TaskEnter_H = *(pTAK_RM_STK++);
	if(Cache_TaskEnter_L == 0x00 && Cache_TaskEnter_H == 0x00){
		//如果任务数据栈为空，则从JCB获取		
		Cache_TaskEnter_L = *(p_JCB++);
		Cache_TaskEnter_H = *(p_JCB++);		
	}	
	//切换任务栈的起始地址
	//先从任务数据栈中获取
	SP = *(pTAK_RM_STK++);
	if(SP == 0){
		//如果任务数据栈为空，则设置为初始任务栈地址
		SP = C_sgTASK_SP_SRART + RN_TASK_IDX * C_sgTASK_SP_CAPACITY;
	}
	//恢复寄存器数据
	for(i=0x08;i<0x10;i++){			
		*((uchar data*)i) = *(pTAK_RM_STK++);
	}

	//跳转
	#pragma asm		
	MOV A,Cache_TaskEnter_L
	PUSH ACC
	MOV A,Cache_TaskEnter_H
	PUSH ACC
	MOV A,17H	
	SETB RS0;切换寄存器组到第1区	
	RET
	#pragma endasm	
	while(1);
}

void OS_TaskRecover(){		
	//设置JCB	
	p_JCB = C_JCB_START + RN_TASK_IDX * C_JCB_CAPACITY;//任务地址栈指针
	*p_JCB = JCB_DESTORY;	
	while(1);
}

void sleep(uchar ms)   
{
	unsigned char a,b,c;
    for(c=ms;c>0;c--)
      for(b=142;b>0;b--)
        for(a=2;a>0;a--);
}

void addTask(uint task){		
	for(p_JCB = C_JCB_START; p_JCB < C_JCB_START + C_MAX_TASK_COUNT * C_JCB_CAPACITY; p_JCB += C_JCB_CAPACITY){
		if(*p_JCB == JCB_DESTORY){
			*(p_JCB++) = JCB_BLOCK;
			*(p_JCB++) = task;
			*(p_JCB++) = task >> 8;
			break;
		}
	}		
}