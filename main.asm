    ORG 0000H
    LJMP START
    ORG 000BH
    LJMP Timer0Interrupt

START:
    MOV SP,#60H
	MOV P1,#00H
	MOV 30H,#00H;中断次数
	MOV 31H,#40H;任务数组栈指针
	MOV 32H,#00H;任务数
	MOV 33H,#00H;当前任务下标
	LCALL AddTasks
	LCALL InitTimer0

LOOP:
         ;add your code here!
    LJMP LOOP

InitTimer0:
    MOV TMOD,#01H
    MOV TH0,#0FCH
    MOV TL0,#18H
    SETB EA
    SETB ET0
    SETB TR0
    RET
	
Sleep:   
    MOV R7,#0A7H
DL1:
    MOV R6,#0ABH
DL0:
    MOV R5,#10H
    DJNZ R5,$
    DJNZ R6,DL0
    DJNZ R7,DL1
    NOP
	RET

AddTasks:
	MOV R6,#LOW(Task1)
	MOV R7,#HIGH(Task1)
	LCALL AddTask
	MOV R6,#LOW(Task2)
	MOV R7,#HIGH(Task2)
	LCALL AddTask
	RET

;接收1个uint参数,参数地址：R6,R7
AddTask:	
	;获取任务数组指针
	MOV R0,31H
	;参数的低字节存入指针位置
	MOV A, R6	
	MOV @R0, A
	;指针位置++
	INC R0	
	;参数的高字节存入指针位置
	MOV A, R7
	MOV @R0, A
	;指针位置++
	INC R0
	;将指针位置赋值回31H
	MOV 31H,R0
	;任务数++
	INC 32H
	RET

Task1:  
	CPL P1.0
	LCALL Sleep
	LJMP Task1	
	
Task2:	
	CPL P1.1
	LCALL Sleep
	LJMP Task2	

TaskDispatch:	
	MOV A,33H
	CJNE A,32H,TaskDispatchNEXT;IF(当前任务下标==任务总数){
	MOV 33H,#00H
TaskDispatchNEXT:	;}else{
	;当前任务下标x2(左移实现)
	MOV A,33H;
	RL A
	;40H+当前任务数
	ADD A,#40H
	;准备要执行的任务地址（低8位）赋值给DPL
	MOV R0,A
	MOV A,@R0
	MOV DPL,A
	;准备要执行的任务地址（低8位）赋值给DPL
	;R0++
	INC R0
	MOV A,@R0
	MOV DPH,A
	;根据DPTR的地址进行跳转
	MOV A,#00H
	INC 33H
	JMP @A+DPTR		
	RET
	
Timer0Interrupt:    
    MOV TH0,#00H
    MOV TL0,#00H
    MOV P2,30H;显示当前任务数于P2口
	INC 30H;中断次数++
	DEC SP
	DEC SP
	;将调度函数入口地址入栈
	MOV A,#LOW(TaskDispatch)
	PUSH ACC
	MOV A,#HIGH(TaskDispatch)
	PUSH ACC
    RETI

END
