#include <reg52.h>
#include <math.h>
#include <stdio.h>

#include "chuankou.h"	    ////����ͷ�ļ�

sbit JDQ=P1^7;

///*Temp_Buffer[7]�����洢���ڷ��͵����ݣ�com_dat������¼���ڷ��͵ĸ���*/
unsigned char Temp_Buffer[7], *q, com_dat;      //�Ӵ��ڽ��յ�����

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
	if(RI == 1)   //��Ӳ�����յ�һ������ʱ��RI����λ
	{
		Temp_Buffer[1] = SBUF; 		//�ѴӴ��ڶ��������浽����
		RI = 0; 		
	} 
	EA = 1;
}


