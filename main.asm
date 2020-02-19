    EXTRN CODE(OS_Main)	
	EXTRN CODE(OS_taskDispatch)
	EXTRN CODE(OS_TaskRecover)
	D_RN_TASK_IDX DATA 30H;当前任务下标	
	D_ITRPT_COUNT DATA 31H;中断次数	
	D_pTAK_RM_STK DATA 33H;任务RAM栈指针			
		
	C_TASKs_RAM_S EQU 0000H;任务RAM起始地址		
	C_MAIN_SP_SRT EQU 70H;默认栈数据起始地址	
	C_sgTAK_RM_CT EQU 20H;单个任务最大数据量	
	C_sgTAK_SP_CT EQU 20H;单个任务栈最大数据量
	
	
	ORG 0000H
    LJMP START
    ORG 000BH
    LJMP Timer0Interrupt

START:
	;SP不为07H,进行任务回收
	MOV A,SP
	CJNE A,#07H,STARTRECOVER
	AJMP STARTNEXT
STARTRECOVER:
	LJMP OS_TaskRecover
STARTNEXT:
	;正式开始初始化
    MOV SP,#C_MAIN_SP_SRT	
	MOV D_ITRPT_COUNT,#00H;中断次数		
	MOV D_RN_TASK_IDX,#00H;当前任务下标	
	LCALL OS_Main
	LCALL InitTimer0
	LJMP TaskDispatch	

InitTimer0:
    MOV TMOD,#01H
    MOV TH0,#0FCH
    MOV TL0,#18H
    SETB EA
    SETB ET0
    SETB TR0
    RET

TaskDispatch:	
	LJMP OS_taskDispatch

;将当前任务单个数据入栈保护
;参数存放：A
PushTaskData:	
	PUSH ACC
	;MOV (任务数据起始地址 + 当前任务下标*单个任务所占用的最大数据量) + 当前任务数据栈指针
	;MOV (C_TASKs_RAM_S + D_RN_TASK_IDX*C_sgTAK_RM_CT) + D_pTAK_RM_STK,A
	MOV A,D_RN_TASK_IDX
	MOV B,#C_sgTAK_RM_CT
	MUL AB					;D_RN_TASK_IDX*C_sgTAK_RM_CT
	ADD A,#C_TASKs_RAM_S	;(C_TASKs_RAM_S + AND)
	ADD A,D_pTAK_RM_STK		;AND + D_pTAK_RM_STK
	MOV R1,A
	POP ACC
	MOV P2,#00H
	MOVX @R1,A
	INC D_pTAK_RM_STK	
	RET
	
Timer0Interrupt: 
    MOV TH0,#00H
    MOV TL0,#00H
	;切换寄存器组到第0区
	CLR RS0	    
	INC D_ITRPT_COUNT;中断次数++	
	MOV D_pTAK_RM_STK, #00H;对任务数据栈指针清0
	;当前ACC入任务栈保护
	LCALL PushTaskData
	;当前PSW入任务栈保护
	MOV ACC,PSW
	LCALL PushTaskData	
	;当前CP指针位置入任务栈保护
	POP ACC
	MOV R0,ACC
	POP ACC
	LCALL PushTaskData
	MOV A,R0
	LCALL PushTaskData
	;当前SP指针位置入任务栈保护
	MOV A,SP
	LCALL PushTaskData	
	;切换堆栈指针指向主栈
	MOV SP,#C_MAIN_SP_SRT
	;将调度函数入口地址入栈	
	MOV A,#LOW(TaskDispatch)
	PUSH ACC
	MOV A,#HIGH(TaskDispatch)
	PUSH ACC
    RETI

END
