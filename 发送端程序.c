#include <reg52.h>		   		// 头文件包含
#include <intrins.h>

#define uchar unsigned char	// 以后unsigned char就可以用uchar代替
#define uint  unsigned int	// 以后unsigned int 就可以用uint 代替

sfr ISP_DATA  = 0xe2;			// 数据寄存器
sfr ISP_ADDRH = 0xe3;			// 地址寄存器高八位
sfr ISP_ADDRL = 0xe4;			// 地址寄存器低八位
sfr ISP_CMD   = 0xe5;			// 命令寄存器
sfr ISP_TRIG  = 0xe6;			// 命令触发寄存器
sfr ISP_CONTR = 0xe7;			// 命令寄存器

sbit LcdRs_P   = P2^4;      // 1602液晶的RS管脚       
sbit LcdRw_P   = P2^5;      // 1602液晶的RW管脚 
sbit LcdEn_P   = P2^6;      // 1602液晶的EN管脚
sbit KeySet_P  = P1^2;		// “设置”按键的管脚
sbit KeyDown_P = P1^6;		// “减”按键的管脚
sbit KeyUp_P   = P1^4;		// “加”按键的管脚 
sbit DQ_0=P3^7;             //DS18B20 端口

uchar AlarmTH;			    // 温度上限报警值

unsigned char  TempBuffer1[]={0x00,0x00};


/*********************************************************/
// 串口配置
/*********************************************************/
void UsartConfiguration()
{
	SCON=0X50;			//设置为工作方式1
	TMOD=0X20;			//设置计数器工作方式2
	PCON=0X80;			//波特率加倍
	TH1=0XF3;			//计数器初始值设置，注意波特率是4800的
	TL1=0XF3;
	TR1=1;				//打开计数器
}


/*********************************************************/
// 单片机内部EEPROM不使能
/*********************************************************/
void ISP_Disable()
{
	ISP_CONTR = 0;
	ISP_ADDRH = 0;
	ISP_ADDRL = 0;
}


/*********************************************************/
// 从单片机内部EEPROM读一个字节，从0x2000地址开始
/*********************************************************/
unsigned char EEPROM_Read(unsigned int add)
{
	ISP_DATA  = 0x00;
	ISP_CONTR = 0x83;
	ISP_CMD   = 0x01;
	ISP_ADDRH = (unsigned char)(add>>8);
	ISP_ADDRL = (unsigned char)(add&0xff);
	ISP_TRIG  = 0x46;	   
	ISP_TRIG  = 0xB9;
	_nop_();
	ISP_Disable();
	return (ISP_DATA);
}


/*********************************************************/
// 往单片机内部EEPROM写一个字节，从0x2000地址开始
/*********************************************************/
void EEPROM_Write(unsigned int add,unsigned char ch)
{
	ISP_CONTR = 0x83;
	ISP_CMD   = 0x02;
	ISP_ADDRH = (unsigned char)(add>>8);
	ISP_ADDRL = (unsigned char)(add&0xff);
	ISP_DATA  = ch;
	ISP_TRIG  = 0x46;
	ISP_TRIG  = 0xB9;
	_nop_();
	ISP_Disable();
}


/*********************************************************/
// 擦除单片机内部EEPROM的一个扇区
// 写8个扇区中随便一个的地址，便擦除该扇区，写入前要先擦除
/*********************************************************/
void Sector_Erase(unsigned int add)	  
{
	ISP_CONTR = 0x83;
	ISP_CMD   = 0x03;
	ISP_ADDRH = (unsigned char)(add>>8);
	ISP_ADDRL = (unsigned char)(add&0xff);
	ISP_TRIG  = 0x46;
	ISP_TRIG  = 0xB9;
	_nop_();
	ISP_Disable();
}


/*********************************************************/
// 微秒级的延时函数，time是要延时的毫秒数
/*********************************************************/
void DelayUs2x(unsigned char t)
{   
 while(--t);
}


/*********************************************************/
// 毫秒级的延时函数，time是要延时的毫秒数
/*********************************************************/
void DelayMs(uint time)
{
	uint i,j;
	for(i=0;i<time;i++)
		for(j=0;j<112;j++);
}


/*********************************************************/
// 1602液晶写命令函数，cmd就是要写入的命令
/*********************************************************/
void LcdWriteCmd(uchar cmd)
{ 
	LcdRs_P = 0;
	LcdRw_P = 0;
	LcdEn_P = 0;
	P0=cmd;
	DelayMs(2);
	LcdEn_P = 1;    
	DelayMs(2);
	LcdEn_P = 0;	
}


/*********************************************************/
// 1602液晶写数据函数，dat就是要写入的数据
/*********************************************************/
void LcdWriteData(uchar dat)
{
	LcdRs_P = 1; 
	LcdRw_P = 0;
	LcdEn_P = 0;
	P0=dat;
	DelayMs(2);
	LcdEn_P = 1;    
	DelayMs(2);
	LcdEn_P = 0;
}


/*********************************************************/
// 1602液晶初始化函数
/*********************************************************/
void LcdInit()
{
	LcdWriteCmd(0x38);        // 16*2显示，5*7点阵，8位数据口
	LcdWriteCmd(0x0C);        // 开显示，不显示光标
	LcdWriteCmd(0x06);        // 地址加1，当写入数据后光标右移
	LcdWriteCmd(0x01);        // 清屏
}


/*********************************************************/
// 液晶光标定位函数
/*********************************************************/
void LcdGotoXY(uchar line,uchar column)
{
	// 第一行
	if(line==0)        
		LcdWriteCmd(0x80+column); 
	// 第二行
	if(line==1)        
		LcdWriteCmd(0x80+0x40+column); 
}


/*********************************************************/
// 液晶输出字符串函数
/*********************************************************/
void LcdPrintStr(uchar *str)
{
	while(*str!='\0') 			// 判断是否到字符串的尽头了
		LcdWriteData(*str++);
}


/*********************************************************/
// 液晶输出数字
/*********************************************************/
void LcdPrintNum(uchar num)
{
	LcdWriteData(num/10+48);	// 十位
	LcdWriteData(num%10+48); 	// 个位
}


/*********************************************************/
// 液晶显示内容的初始化
/*********************************************************/
void LcdShowInit()
{
	LcdGotoXY(0,0);						    // 第0行的显示内容
	LcdPrintStr("  Temp System  ");
	LcdGotoXY(1,0);						   	// 第1行的显示内容
	LcdPrintStr("T:   C   H:   C");
	LcdGotoXY(1,4);						    // 温度单位摄氏度上面的圆圈符号
	LcdWriteData(0xdf);	
	LcdGotoXY(1,13);						// 温度单位摄氏度上面的圆圈符号
	LcdWriteData(0xdf);	
}



/*********************************************************/
// DS18B20初始化
/*********************************************************/
bit Init_DS18B20_0(void)
{
	 bit dat=0;
	 DQ_0 = 1;    //DQ复位
	 DelayUs2x(5);   //稍做延时
	 DQ_0 = 0;         //单片机将DQ拉低
	 DelayUs2x(200); //精确延时 大于 480us 小于960us
	 DelayUs2x(200);
	 DQ_0 = 1;        //拉高总线
	 DelayUs2x(50); //15~60us 后 接收60-240us的存在脉冲
	 dat=DQ_0;        //如果x=0则初始化成功, x=1则初始化失败
	 DelayUs2x(25); //稍作延时返回
	 return dat;
}


/*********************************************************/
// 串口读取一个字节
/*********************************************************/
unsigned char ReadOneChar_0(void)
{
unsigned char i=0;
unsigned char dat = 0;
for (i=8;i>0;i--)
 {
  DQ_0 = 0; // 给脉冲信号
  dat>>=1;
  DQ_0 = 1; // 给脉冲信号
  if(DQ_0)
   dat|=0x80;
  DelayUs2x(25);
 }
 return(dat);
}

/*********************************************************/
// 串口写入一个字节
/*********************************************************/
void WriteOneChar_0(unsigned char dat)
{
 unsigned char i=0;
 for (i=8; i>0; i--)
 {
  DQ_0 = 0;
  DQ_0 = dat&0x01;
  DelayUs2x(25);
  DQ_0 = 1;
  dat>>=1;
 }
DelayUs2x(25);
}

/*********************************************************/
//读取温度数据
/*********************************************************/
unsigned int ReadTemperature_0(void)
{
	unsigned char a=0;
	unsigned int b=0;
	unsigned int t=0;
	Init_DS18B20_0();
	WriteOneChar_0(0xCC); // 跳过读序号列号的操作
	WriteOneChar_0(0x44); // 启动温度转换
	DelayMs(10);
	Init_DS18B20_0();
	WriteOneChar_0(0xCC); //跳过读序号列号的操作 
	WriteOneChar_0(0xBE); //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
	a=ReadOneChar_0();   //低位
	b=ReadOneChar_0();   //高位
	b<<=8;
	t=a+b;
	return(t);
}

/*********************************************************/
// 按键扫描，用于设置温湿度报警范围
/*********************************************************/
void KeyScanf()
{
	if(KeySet_P==0)		// 判断设置按键是否被按下
	{

		/*将液晶显示改为设置页面的*******************************************************/

		LcdWriteCmd(0x01);				     // 设置界面的显示框架
		LcdGotoXY(0,0);
		LcdPrintStr("TempUp:          ");

		LcdGotoXY(0,9);	 					// 在液晶上填充温度的上限值
		LcdPrintNum(AlarmTH);

		LcdGotoXY(0,10);	 				// 光标定位到第0行第7列
		LcdWriteCmd(0x0F);				    // 光标闪烁
		
		DelayMs(10);	  					// 去除按键按下的抖动
		while(!KeySet_P);	 				// 等待按键释放
		DelayMs(10);					  	// 去除按键松开的抖动


		/*设置温度的上限值****************************************************************/
				
		while(KeySet_P)	  				// “设置键”没有被按下，则一直处于温度上限的设置
		{
			if(KeyDown_P==0)				// 判断 “减按键“ 是否被按下
			{
				if(AlarmTH>0)  				// 只有当温度上限值大于0时，才能减1			
					AlarmTH--;
				LcdGotoXY(0,9);	 	  	// 重新刷新显示更改后的温度上限值
				LcdPrintNum(AlarmTH);
				DelayMs(350);					// 延时
			}
			if(KeyUp_P==0)			   	// 判断 “加按键“ 是否被按下
			{
				if(AlarmTH<99)	 			// 只有当温度上限值小于99时，才能加1
					AlarmTH++;
				LcdGotoXY(0,9);				// 重新刷新显示更改后的温度上限值 	
				LcdPrintNum(AlarmTH);
				DelayMs(350);					// 延时
			}								 
		}

		DelayMs(10);	  					// 去除按键按下的抖动
		while(!KeySet_P);	 				// 等待按键释放
		DelayMs(10);					  	// 去除按键松开的抖动

		LcdWriteCmd(0x0C);	  		        // 取消光标闪烁
		LcdShowInit();						// 液晶显示为检测界面的

		Sector_Erase(0x2000);			 			// 存储之前必须先擦除
		EEPROM_Write(0x2001,AlarmTH);		// 把温度上限存入到EEPROM的0x2001这个地址
	}	
}


/*********************************************************/
// 主函数
/*********************************************************/
void main()
{
	uchar i;
	unsigned int temp_0; 

	LcdInit();							// 液晶功能的初始化			
	LcdShowInit(); 					// 液晶显示的初始化
	UsartConfiguration();          //串口初始化

	AlarmTH=EEPROM_Read(0x2001);	// 从EEPROM的0x2001这个地址读取温度的报警上限

	if((AlarmTH==0)||(AlarmTH>100))	// 如果温度上限报警值读出来异常（等于0或大于100），则重新赋值
		AlarmTH=35;
	
	while(1)
	{
		temp_0=ReadTemperature_0();	//读取温度
        temp_0=(unsigned int)temp_0*0.0625;//温度转化
		
		LcdGotoXY(1,2);	 			// 显示定位
		LcdPrintNum(temp_0);		// 显示温度值
		LcdGotoXY(1,11);			// 显示定位
		LcdPrintNum(AlarmTH);		// 显示温度阈值
		
		if(temp_0<AlarmTH)          //温度小于设定温度上限，继电器持续开启
		{
			TempBuffer1[1]='1';
			SBUF=TempBuffer1[1];    //发送报警标志
			while(!TI);		        //等待发送数据完成
			TI=0;
		}
		else 
		{
			TempBuffer1[1]='2';
			SBUF=TempBuffer1[1];     //发送报警标志
			while(!TI);		         //等待发送数据完成
			TI=0;
		}

		for(i=0;i<110;i++)
		{
			KeyScanf();					// 按键扫描
		}
	}
}
