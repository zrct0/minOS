#include "os.h"

uchar RN_TASK_IDX _at_ 0x30; 		       //当前任务下标
uchar ITRPT_COUNT _at_ 0x31;			     //中断次数
uchar xdata *pTAK_RM_STK _at_ 0x32;    //任务RAM栈指针	
uchar data *p_JCB _at_ 0x34;		       //JCB地址栈指针
uchar xdata *p_HeapManager _at_ 0x35;  //堆管理器地址指针
uint p_Heap _at_ 0x37;		     //堆地址指针
uint FREE_MEMORY _at_ 0x39; //剩余堆内存

#define C_MAX_TASK_COUNT 0x04		  //最多任务数
#define C_JCB_START 0x50		      //JCB起始地址
#define C_TASKs_RAM_START 0x0000	//任务RAM起始地址	
#define C_sgTASK_SP_SRART 0x80		//任务栈数据起始地址
#define C_MAIN_SP_START 0x70			//默认栈数据起始地址
#define C_JCB_CAPACITY 0x08	      //JCB容量
#define C_TASKs_RAM_CAPACITY 0x20 //单个任务最大容量
#define C_sgTASK_SP_CAPACITY 0x20	//单个任务栈最大容量
#define C_HeapManager_START 0x80  //堆管理器起始地址
#define C_sgHeapManager_CAPACITY 0x04//堆管理器单元容量
#define C_Heap_START 0x160           //堆起始地址
#define C_Heap_END 0x7FF        //堆结束地址

#define JCB_DESTORY 0x00
#define JCB_BLOCK 0x01
#define JCB_RUN 0x02

#define MEMORY_LEN(X) *X
#define MEMORY_START(X) *((uint xdata *)(X + 1))
#define MEMORY_P_ADRESS(X) *(X + 3)	

uchar data Cache_ACC _at_ 0x40;        //ACC临时变量
uchar data Cache_TaskEnter_L _at_ 0x41;//任务人口地址L临时变量
uchar data Cache_TaskEnter_H _at_ 0x42;//任务人口地址H临时变量
uchar data Cache_uchar1 _at_ 0x43;      //临时变量1   
uchar data Cache_uchar2 _at_ 0x44;      //临时变量2 
uchar data Cache_uchar3 _at_ 0x45;      //临时变量3
uint data Cache_uint1 _at_ 0x46;         
uint data Cache_uint2 _at_ 0x48;
uchar xdata *Cache_px_uchar1 _at_ 0x4A;
uint xdata *Cache_px_uint1 _at_ 0x4C;

extern void addTasks();
extern void pushTaskData();

void GC_modifyHeapManagerUnits(uint address_threshold, uchar length);
void GC_moveBytes(uint start, uint end, uchar length);


void OS_Main(){
	addTasks();
	p_Heap = C_Heap_START;
}


void malloc(uchar size_t, uchar xdata ** p){
	lock();
	//在堆管理器内存查找可用的空间
	p_HeapManager = C_HeapManager_START;
	while(MEMORY_P_ADRESS(p_HeapManager)){
		p_HeapManager += C_sgHeapManager_CAPACITY;
	}
	//第1个字节存放需开辟内存的字节数
	MEMORY_LEN(p_HeapManager) = size_t;		
	//第2、3字节记录该内存起始地址
	MEMORY_START(p_HeapManager) = p_Heap;
	//第4字节记录指针在内存中的位置
	MEMORY_P_ADRESS(p_HeapManager) = (uchar)p;	
	//当前堆指针起始地址赋值给p
	*p = (uchar*)p_Heap;
		//堆指针后移	
	p_Heap += size_t;
	unlock();
}

void free(uchar xdata ** p){
	lock();
	p_HeapManager = C_HeapManager_START;
	while(MEMORY_LEN(p_HeapManager)){
		//如果和p地址相同
		if(MEMORY_P_ADRESS(p_HeapManager) == p){
			//再校验地址
			//保存该单元长度
			Cache_uchar1 = MEMORY_LEN(p_HeapManager);
			//保存该单元指向的起始地址
			Cache_uint1 = MEMORY_START(p_HeapManager);			
			//获得p所指向的地址
			Cache_uint2 = (uint)*p;
			//校验地址
			if(Cache_uint2 >= Cache_uint1 && Cache_uint1 < Cache_uint2 + Cache_uchar1){
				MEMORY_P_ADRESS(p_HeapManager) = 0x00;//释放该空间
				return;
			}
		}
		p_HeapManager += C_sgHeapManager_CAPACITY;
	}
	unlock();
}

void GC(){
	lock();
	//寻找需回收的内存
	p_HeapManager = C_HeapManager_START;
	while(MEMORY_LEN(p_HeapManager)){
		if(MEMORY_P_ADRESS(p_HeapManager) == 0x00){
			//获取该单元长度
			Cache_uchar1 = MEMORY_LEN(p_HeapManager);
			//获取该单元指向的起始地址
			Cache_uint1 = MEMORY_START(p_HeapManager);			
			//修改堆管理器中单元的值
			GC_modifyHeapManagerUnits(Cache_uint1, Cache_uchar1);
			//整体移动字节
			GC_moveBytes(Cache_uint1 + Cache_uchar1, p_Heap, Cache_uchar1);
			p_Heap -= Cache_uchar1;
		}
		p_HeapManager += C_sgHeapManager_CAPACITY;
	}
	unlock();
}

//修改堆管理器中单元的值
void GC_modifyHeapManagerUnits(uint address_threshold, uchar length){
	Cache_px_uchar1 = C_HeapManager_START;
	while(MEMORY_LEN(Cache_px_uchar1)){
		//如果地址大于阈值
		if(MEMORY_START(Cache_px_uchar1) > address_threshold){
			//地址减去指定长度
			MEMORY_START(Cache_px_uchar1) -= length;
		}
		Cache_px_uchar1 += C_sgHeapManager_CAPACITY;
	}
}

//整体移动字节
void GC_moveBytes(uint start, uint end, uchar length){
	for(Cache_uint1 = start; Cache_uint1 < end; Cache_uint1++){
		*((uchar xdata *)(Cache_uint1 - length)) = *((uchar xdata *)Cache_uint1);
	}
}

uchar getDataMemoryUnit(uchar address){
	return *((uchar data *)address);
}

uchar getXDataMemoryUnit(uint address){
	return *((uchar xdata *)address);
}

void lock(){
	ET0 = 0;
}

void unlock(){
	ET0 = 1;
}

void yield(){
	TH0 = 0xFF;
  TL0 = 0xFF;
}

void OS_taskDispatch(){

	//寄存器入栈保护	
	for(Cache_uchar1=0x00;Cache_uchar1<0x18;Cache_uchar1++){		
		//MOV (C_TASKs_RAM_S + D_RN_TASK_IDX*C_sgTAK_RM_CT) + D_pTAK_RM_STK,A
		*((uchar xdata *)(C_TASKs_RAM_START + RN_TASK_IDX*C_TASKs_RAM_CAPACITY + pTAK_RM_STK++)) = *((uchar data*)Cache_uchar1);		
	}
	
	//寻找下一个任务的下标
	Cache_uchar1 = RN_TASK_IDX + 1;
	Cache_uchar2 = 0;//是否找到标志位		
	p_JCB = C_JCB_START + Cache_uchar1 * C_JCB_CAPACITY;
	while(!Cache_uchar2){
		//从当前任务下标往后找
		for(; p_JCB < C_JCB_START + C_MAX_TASK_COUNT * C_JCB_CAPACITY; p_JCB += C_JCB_CAPACITY){		
			if(*p_JCB  == JCB_BLOCK){	
			  //上一个任务变为阻塞状态
				*((uchar data*)(C_JCB_START + RN_TASK_IDX * C_JCB_CAPACITY)) = JCB_BLOCK;
				//下一任务变成运行态
				RN_TASK_IDX = Cache_uchar1;
				*(p_JCB++) = JCB_RUN;	
				Cache_uchar2 = 1;//找到了
				break;
			}	
			++Cache_uchar1;
		}
		//如果没找到，从头找
		if(!Cache_uchar2){
			p_JCB = C_JCB_START;
			Cache_uchar1 = 0;
		}
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
	for(Cache_uchar1=0x00;Cache_uchar1<0x18;Cache_uchar1++){			
		*((uchar data*)Cache_uchar1) = *(pTAK_RM_STK++);
	}

	//跳转
	#pragma asm		
	MOV A,Cache_TaskEnter_L
	PUSH ACC
	MOV A,Cache_TaskEnter_H
	PUSH ACC
	MOV A,17H	
	;切换寄存器组到第0区
	CLR RS0
	CLR RS1
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