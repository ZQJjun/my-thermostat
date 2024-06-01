#include <reg52.h>		   		// ͷ�ļ�����
#include <intrins.h>

#define uchar unsigned char	// �Ժ�unsigned char�Ϳ�����uchar����
#define uint  unsigned int	// �Ժ�unsigned int �Ϳ�����uint ����

sfr ISP_DATA  = 0xe2;			// ���ݼĴ���
sfr ISP_ADDRH = 0xe3;			// ��ַ�Ĵ����߰�λ
sfr ISP_ADDRL = 0xe4;			// ��ַ�Ĵ����Ͱ�λ
sfr ISP_CMD   = 0xe5;			// ����Ĵ���
sfr ISP_TRIG  = 0xe6;			// ������Ĵ���
sfr ISP_CONTR = 0xe7;			// ����Ĵ���

sbit LcdRs_P   = P2^4;      // 1602Һ����RS�ܽ�       
sbit LcdRw_P   = P2^5;      // 1602Һ����RW�ܽ� 
sbit LcdEn_P   = P2^6;      // 1602Һ����EN�ܽ�
sbit KeySet_P  = P1^2;		// �����á������Ĺܽ�
sbit KeyDown_P = P1^6;		// �����������Ĺܽ�
sbit KeyUp_P   = P1^4;		// ���ӡ������Ĺܽ� 
sbit DQ_0=P3^7;             //DS18B20 �˿�

uchar AlarmTH;			    // �¶����ޱ���ֵ

unsigned char  TempBuffer1[]={0x00,0x00};


/*********************************************************/
// ��������
/*********************************************************/
void UsartConfiguration()
{
	SCON=0X50;			//����Ϊ������ʽ1
	TMOD=0X20;			//���ü�����������ʽ2
	PCON=0X80;			//�����ʼӱ�
	TH1=0XF3;			//��������ʼֵ���ã�ע�Ⲩ������4800��
	TL1=0XF3;
	TR1=1;				//�򿪼�����
}


/*********************************************************/
// ��Ƭ���ڲ�EEPROM��ʹ��
/*********************************************************/
void ISP_Disable()
{
	ISP_CONTR = 0;
	ISP_ADDRH = 0;
	ISP_ADDRL = 0;
}


/*********************************************************/
// �ӵ�Ƭ���ڲ�EEPROM��һ���ֽڣ���0x2000��ַ��ʼ
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
// ����Ƭ���ڲ�EEPROMдһ���ֽڣ���0x2000��ַ��ʼ
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
// ������Ƭ���ڲ�EEPROM��һ������
// д8�����������һ���ĵ�ַ���������������д��ǰҪ�Ȳ���
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
// ΢�뼶����ʱ������time��Ҫ��ʱ�ĺ�����
/*********************************************************/
void DelayUs2x(unsigned char t)
{   
 while(--t);
}


/*********************************************************/
// ���뼶����ʱ������time��Ҫ��ʱ�ĺ�����
/*********************************************************/
void DelayMs(uint time)
{
	uint i,j;
	for(i=0;i<time;i++)
		for(j=0;j<112;j++);
}


/*********************************************************/
// 1602Һ��д�������cmd����Ҫд�������
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
// 1602Һ��д���ݺ�����dat����Ҫд�������
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
// 1602Һ����ʼ������
/*********************************************************/
void LcdInit()
{
	LcdWriteCmd(0x38);        // 16*2��ʾ��5*7����8λ���ݿ�
	LcdWriteCmd(0x0C);        // ����ʾ������ʾ���
	LcdWriteCmd(0x06);        // ��ַ��1����д�����ݺ�������
	LcdWriteCmd(0x01);        // ����
}


/*********************************************************/
// Һ����궨λ����
/*********************************************************/
void LcdGotoXY(uchar line,uchar column)
{
	// ��һ��
	if(line==0)        
		LcdWriteCmd(0x80+column); 
	// �ڶ���
	if(line==1)        
		LcdWriteCmd(0x80+0x40+column); 
}


/*********************************************************/
// Һ������ַ�������
/*********************************************************/
void LcdPrintStr(uchar *str)
{
	while(*str!='\0') 			// �ж��Ƿ��ַ����ľ�ͷ��
		LcdWriteData(*str++);
}


/*********************************************************/
// Һ���������
/*********************************************************/
void LcdPrintNum(uchar num)
{
	LcdWriteData(num/10+48);	// ʮλ
	LcdWriteData(num%10+48); 	// ��λ
}


/*********************************************************/
// Һ����ʾ���ݵĳ�ʼ��
/*********************************************************/
void LcdShowInit()
{
	LcdGotoXY(0,0);						    // ��0�е���ʾ����
	LcdPrintStr("  Temp System  ");
	LcdGotoXY(1,0);						   	// ��1�е���ʾ����
	LcdPrintStr("T:   C   H:   C");
	LcdGotoXY(1,4);						    // �¶ȵ�λ���϶������ԲȦ����
	LcdWriteData(0xdf);	
	LcdGotoXY(1,13);						// �¶ȵ�λ���϶������ԲȦ����
	LcdWriteData(0xdf);	
}



/*********************************************************/
// DS18B20��ʼ��
/*********************************************************/
bit Init_DS18B20_0(void)
{
	 bit dat=0;
	 DQ_0 = 1;    //DQ��λ
	 DelayUs2x(5);   //������ʱ
	 DQ_0 = 0;         //��Ƭ����DQ����
	 DelayUs2x(200); //��ȷ��ʱ ���� 480us С��960us
	 DelayUs2x(200);
	 DQ_0 = 1;        //��������
	 DelayUs2x(50); //15~60us �� ����60-240us�Ĵ�������
	 dat=DQ_0;        //���x=0���ʼ���ɹ�, x=1���ʼ��ʧ��
	 DelayUs2x(25); //������ʱ����
	 return dat;
}


/*********************************************************/
// ���ڶ�ȡһ���ֽ�
/*********************************************************/
unsigned char ReadOneChar_0(void)
{
unsigned char i=0;
unsigned char dat = 0;
for (i=8;i>0;i--)
 {
  DQ_0 = 0; // �������ź�
  dat>>=1;
  DQ_0 = 1; // �������ź�
  if(DQ_0)
   dat|=0x80;
  DelayUs2x(25);
 }
 return(dat);
}

/*********************************************************/
// ����д��һ���ֽ�
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
//��ȡ�¶�����
/*********************************************************/
unsigned int ReadTemperature_0(void)
{
	unsigned char a=0;
	unsigned int b=0;
	unsigned int t=0;
	Init_DS18B20_0();
	WriteOneChar_0(0xCC); // ����������кŵĲ���
	WriteOneChar_0(0x44); // �����¶�ת��
	DelayMs(10);
	Init_DS18B20_0();
	WriteOneChar_0(0xCC); //����������кŵĲ��� 
	WriteOneChar_0(0xBE); //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
	a=ReadOneChar_0();   //��λ
	b=ReadOneChar_0();   //��λ
	b<<=8;
	t=a+b;
	return(t);
}

/*********************************************************/
// ����ɨ�裬����������ʪ�ȱ�����Χ
/*********************************************************/
void KeyScanf()
{
	if(KeySet_P==0)		// �ж����ð����Ƿ񱻰���
	{

		/*��Һ����ʾ��Ϊ����ҳ���*******************************************************/

		LcdWriteCmd(0x01);				     // ���ý������ʾ���
		LcdGotoXY(0,0);
		LcdPrintStr("TempUp:          ");

		LcdGotoXY(0,9);	 					// ��Һ��������¶ȵ�����ֵ
		LcdPrintNum(AlarmTH);

		LcdGotoXY(0,10);	 				// ��궨λ����0�е�7��
		LcdWriteCmd(0x0F);				    // �����˸
		
		DelayMs(10);	  					// ȥ���������µĶ���
		while(!KeySet_P);	 				// �ȴ������ͷ�
		DelayMs(10);					  	// ȥ�������ɿ��Ķ���


		/*�����¶ȵ�����ֵ****************************************************************/
				
		while(KeySet_P)	  				// �����ü���û�б����£���һֱ�����¶����޵�����
		{
			if(KeyDown_P==0)				// �ж� ���������� �Ƿ񱻰���
			{
				if(AlarmTH>0)  				// ֻ�е��¶�����ֵ����0ʱ�����ܼ�1			
					AlarmTH--;
				LcdGotoXY(0,9);	 	  	// ����ˢ����ʾ���ĺ���¶�����ֵ
				LcdPrintNum(AlarmTH);
				DelayMs(350);					// ��ʱ
			}
			if(KeyUp_P==0)			   	// �ж� ���Ӱ����� �Ƿ񱻰���
			{
				if(AlarmTH<99)	 			// ֻ�е��¶�����ֵС��99ʱ�����ܼ�1
					AlarmTH++;
				LcdGotoXY(0,9);				// ����ˢ����ʾ���ĺ���¶�����ֵ 	
				LcdPrintNum(AlarmTH);
				DelayMs(350);					// ��ʱ
			}								 
		}

		DelayMs(10);	  					// ȥ���������µĶ���
		while(!KeySet_P);	 				// �ȴ������ͷ�
		DelayMs(10);					  	// ȥ�������ɿ��Ķ���

		LcdWriteCmd(0x0C);	  		        // ȡ�������˸
		LcdShowInit();						// Һ����ʾΪ�������

		Sector_Erase(0x2000);			 			// �洢֮ǰ�����Ȳ���
		EEPROM_Write(0x2001,AlarmTH);		// ���¶����޴��뵽EEPROM��0x2001�����ַ
	}	
}


/*********************************************************/
// ������
/*********************************************************/
void main()
{
	uchar i;
	unsigned int temp_0; 

	LcdInit();							// Һ�����ܵĳ�ʼ��			
	LcdShowInit(); 					// Һ����ʾ�ĳ�ʼ��
	UsartConfiguration();          //���ڳ�ʼ��

	AlarmTH=EEPROM_Read(0x2001);	// ��EEPROM��0x2001�����ַ��ȡ�¶ȵı�������

	if((AlarmTH==0)||(AlarmTH>100))	// ����¶����ޱ���ֵ�������쳣������0�����100���������¸�ֵ
		AlarmTH=35;
	
	while(1)
	{
		temp_0=ReadTemperature_0();	//��ȡ�¶�
        temp_0=(unsigned int)temp_0*0.0625;//�¶�ת��
		
		LcdGotoXY(1,2);	 			// ��ʾ��λ
		LcdPrintNum(temp_0);		// ��ʾ�¶�ֵ
		LcdGotoXY(1,11);			// ��ʾ��λ
		LcdPrintNum(AlarmTH);		// ��ʾ�¶���ֵ
		
		if(temp_0<AlarmTH)          //�¶�С���趨�¶����ޣ��̵�����������
		{
			TempBuffer1[1]='1';
			SBUF=TempBuffer1[1];    //���ͱ�����־
			while(!TI);		        //�ȴ������������
			TI=0;
		}
		else 
		{
			TempBuffer1[1]='2';
			SBUF=TempBuffer1[1];     //���ͱ�����־
			while(!TI);		         //�ȴ������������
			TI=0;
		}

		for(i=0;i<110;i++)
		{
			KeyScanf();					// ����ɨ��
		}
	}
}
