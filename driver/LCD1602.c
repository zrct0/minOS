#include "LCD1602.h"


sbit LCD02_RS = P2^0; //RS是高电平是是数据选项，是低电平是命令选项
sbit LCD02_RW = P2^1; //R/W 是高电平是读操作，是低电平是写操作
sbit LCD02_E =  P2^2; //E端为使能端，当E端由高电平跳变为低电平时，液晶模块执行命令
#define LCD02_DB P0		//数据端口

void LCD02_write_command(uchar dat);
void LCD02_write_data(uchar dat);
void LCD02_set_xy(uchar x,uchar y);

void LCD02_init(){
    sleep(1);
		//指令6：功能设置命令。
		//DL表示在高电平时为4位总线，低电平时为8位总线:1
	  //N表示在低电平时为单行显示，高电平时双行显示:1
	  //F表示在低电平时显示5×7的点阵字符，高电平时显示5×10的点阵字符:0
    LCD02_write_command(0x38); 
    sleep(1);
		//指令4：显示开关控制。
		//D用于控制整体显示的开与关，高电平表示开显示，低电平表示关显示:1
		//C用于控制光标的开与关，高电平表示有光标，低电平表示无光标:0
		//B用于控制光标是否闪烁，高电平闪烁，低电平不闪烁:0
		LCD02_write_command(0x0C);
		sleep(1);
		//指令3：输入方式设置。   
		//I/D表示光标的移动方向，高电平右移，低电平左移:1
		//S表示显示屏上所有文字是否左移或右移，高电平表示有效，低电平表示无效:0
    LCD02_write_command(0x06); 
    sleep(1);
		//指令1：清屏。 
		//指令码01H，光标复位到地址00H
    LCD02_write_command(0x01); 
    sleep(1);
}


void LCD02_disp_string(uchar x,uchar y,uchar *s){
	uchar i = 0x00;
  LCD02_set_xy(x++, y); 	
  while(*(s + i) !='\0')
  {
    LCD02_write_data(*(s + i++));   
		LCD02_set_xy(x++,y);
  }
}



//RS＝0、RW＝0——表示向LCD写入指令
void LCD02_write_command(uchar dat){
	lock();
	sleep(1);
	LCD02_RS=0; 
	LCD02_RW=0; 
	LCD02_DB=dat;
	sleep(1);
	LCD02_E=1; 
	sleep(1);
	LCD02_E=0;
	unlock();
}

//RS＝1、RW＝0——表示向LCD写入数据
void LCD02_write_data(uchar dat){
	lock();
	sleep(1);
	LCD02_RS=1;
	LCD02_RW=0;
	LCD02_DB=dat;
	sleep(1);
	LCD02_E=1;
	sleep(1);
	LCD02_E=0;
	unlock();
}

/*
1602字符液晶显示可分为上下两部分各16位进行显示，处于不同行时的字符显示地址如下
第一行：00H-0FH
第二行：40H-4FH
数据存储器地址设置指令：1xxx xxxx
所以第一位为1，则第一行起始地址为80H,第二行起始地址为C0H
*/
void LCD02_set_xy(uchar x,uchar y){		
  uchar address;	
	lock();
  address = (x + y * 0x40) | 0x80; 
  sleep(1);
  LCD02_write_command(address);
	unlock();
}