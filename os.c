#include "os.h"

uchar INTERRUPT_COUNT _at_ 0x10;				//中断次数
uchar data *p_TASKs_STACK _at_ 0x11;		//任务地址栈指针
uchar TASKs_COUNT _at_ 0x12;						//任务数
uchar RUNNING_TASK_INDEX _at_ 0x13; 		//当前任务下标
uchar data *p_TASK_RAM_STACK _at_ 0x14; //任务数据栈指针	

#define C_MAIN_SP_START 0x70				//默认栈数据起始地址
#define C_TASKs_STACK_START 0x20		//任务数地址栈起始地址
#define C_TASKs_RAM_START 0x30			//任务数据起始地址	
#define C_TASKs_RAM_CAPACITY 0x10 //单个任务最大数据量
#define C_sgTASK_SP_SRART 0x50			//任务栈数据起始地址
#define C_sgTASK_SP_CAPACITY 0x10		//单个任务栈最大数据量

uchar data * p_sg_TASK_RAM _at_ 0x15;//任务数据栈指针
uchar data * p_TASK_STACK _at_ 0x16; //任务地址栈指针
uchar data OS_TEMP_ACC _at_ 0x17; //ACC临时变量

extern void addTasks();
extern void pushTaskData();

void OS_Main(){
	addTasks();
}

void DPTR_JMP(){
	#pragma asm		
	MOV A,DPL
	PUSH ACC
	MOV A,DPH
	PUSH ACC
	MOV A,17H	
	SETB RS0;切换寄存器组到第1区	
	RET
	#pragma endasm
}

void OS_taskDispatch(){	
	//寄存器入栈保护
	uchar i;
	for(i=0x08;i<0x10;i++){		
		//MOV (C_TASKs_RAM_S + D_RN_TASK_IDX*C_sgTAK_RM_CT) + D_pTAK_RM_STK,A
		*(uchar data*)(C_TASKs_RAM_START + RUNNING_TASK_INDEX*C_TASKs_RAM_CAPACITY + p_TASK_RAM_STACK++) = *((uchar data*)i);
	}
	
	//IF(当前任务下标==任务总数)
	++RUNNING_TASK_INDEX;
	if(RUNNING_TASK_INDEX == TASKs_COUNT){
		RUNNING_TASK_INDEX = 0x00;
	}
	p_sg_TASK_RAM = C_TASKs_RAM_START + RUNNING_TASK_INDEX * C_TASKs_RAM_CAPACITY;//任务数据栈指针	
	//恢复ACC	
	OS_TEMP_ACC = *(p_sg_TASK_RAM++);
	//恢复PSW
	*((uchar*)PSW) = *(p_sg_TASK_RAM++) & 0xE7;
	//获取入口地址
	//先从任务数据栈中获取		
	DPL = *(p_sg_TASK_RAM++);
	DPH = *(p_sg_TASK_RAM++);
	if(DPL == 0x00 && DPH == 0x00){
		//如果任务数据栈为空，则从任务地址栈获取
		p_TASK_STACK = C_TASKs_STACK_START + RUNNING_TASK_INDEX * 0x02;//任务地址栈指针
		DPL = *(p_TASK_STACK++);
		DPH = *p_TASK_STACK;
	}	
	//切换任务栈的起始地址
	//先从任务数据栈中获取
	SP = *(p_sg_TASK_RAM++);
	if(SP == 0){
		//如果任务数据栈为空，则设置为初始任务栈地址
		SP = C_sgTASK_SP_SRART + RUNNING_TASK_INDEX * C_sgTASK_SP_CAPACITY;
	}
	//恢复寄存器数据
	for(i=0x08;i<0x10;i++){			
		*((uchar data*)i) = *(p_sg_TASK_RAM++);
	}
	//跳转
	DPTR_JMP();	
}

void sleep(uchar ms)   
{
	unsigned char a,b,c;
    for(c=ms;c>0;c--)
        for(b=142;b>0;b--)
            for(a=2;a>0;a--);
}

void addTask(uint task){
	
	*p_TASKs_STACK = task;
	++p_TASKs_STACK;
	*p_TASKs_STACK = task >> 8;
	++p_TASKs_STACK;
	++TASKs_COUNT;	
}