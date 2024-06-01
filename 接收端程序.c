#include <reg52.h>
#include <math.h>
#include <stdio.h>

#include "chuankou.h"	    ////串口头文件

sbit JDQ=P1^7;

///*Temp_Buffer[7]用来存储串口发送的数据，com_dat用来记录串口发送的个数*/
unsigned char Temp_Buffer[7], *q, com_dat;      //从串口接收的数据

void delay(unsigned char i)
{
	while(i--);
}

main()
{
  UsartConfiguration();

	 while(1)
	 {	
    if(Temp_Buffer[1]=='1') 
	   JDQ=0;
	  if(Temp_Buffer[1]=='2') 
	   JDQ=1;
	 }
}
  
void Com_Int(void) interrupt 4
{
	EA = 0;
	if(RI == 1)   //当硬件接收到一个数据时，RI会置位
	{
		Temp_Buffer[1] = SBUF; 		//把从串口读出的数存到数组
		RI = 0; 		
	} 
	EA = 1;
}


