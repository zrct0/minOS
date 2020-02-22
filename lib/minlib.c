#include "minlib.h"

//整型转字符串，结果保持在s中
void int2string(uchar *s, uint n){	
	uint i = 0x00;
	uint j;
	uint m = n;
	lock();
	if(!n){
		*s = 0x30;
		*(s + 1) = '\0';
		return;
	}
	while(m){
		m /= 10;	
		++i;
	}
	j = i - 1;
	while(n){
		*(s + j--) = 0x30 + n % 10;
    n /= 10;		
	}
	*(s + i) = '\0';
	unlock();
}

//字符串拼接，结果保持在s1中
void strcat1(uchar *s1, uchar *s2){
	uchar i = 0x00;	
	uchar s1Size = 0x00;
	lock();
	while(*(s1 + s1Size) !='\0'){
		++s1Size;
	}	
	while(*(s2 + i) !='\0'){
		*(s1 + s1Size + i) = *(s2 + i);
		++i;
	}
	*(s1 + s1Size + i) = '\0';
	unlock();
}

//字符串拷贝，结果保持在s1中
void strcopy(uchar *s1, uchar *s2){
	uchar i = 0x00;	
	lock();
	while(*(s2 + i) !='\0'){
		*(s1 + i) = *(s2 + i);
		++i;
	}
	*(s1 + i) = '\0';
	unlock();
}