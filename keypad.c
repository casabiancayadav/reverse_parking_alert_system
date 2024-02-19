#include<reg51.h>
#include"types.h"
#include<string.h>
#include"keypad.h"
#include"lcd.h"


void delay_ms(unsigned int);
void i2cDevByteWrite(u8,u8,u8);
u8   i2cDevRandomRead(u8,u8);


sbit r0=P2^0;
sbit r1=P2^1;
sbit r2=P2^2;
sbit r3=P2^3;
sbit c0=P2^4;
sbit c1=P2^5;
sbit c2=P2^6;
sbit c3=P2^7;
char row_val,col_val;
char code Key_Val[][4]={'7','8','9','A',
				                '4','5','6','B',
				                '1','2','3','C',
				                '@','0','#','*'};
unsigned char pwd[10];
void Row_Init(void)
{
	r0=r1=r2=r3=0;
}
/*
bit IsKeyPress(void)
{
	return(c0&c1&c2&c3);
}
*/
char KeyValue(void)
{
	Row_Init();
	while((c0&c1&c2&c3));//waiting for key Pressing

	r0=0;r1=r2=r3=1;
	if(!(c0&c1&c2&c3))
	{
		row_val=0;	
		goto col_check;			
	}
	r1=0;r0=r2=r3=1;
	if(!(c0&c1&c2&c3))
	{
		row_val=1;	
		goto col_check;		
	}
	r2=0;r0=r1=r3=1;
	if(!(c0&c1&c2&c3))
	{
		row_val=2;	
		goto col_check;		
	}
	r3=0;r0=r1=r2=1;
	if(!(c0&c1&c2&c3))
		row_val=3;	
		
col_check:
	if(c0==0)
		col_val=0;
	else if(c1==0)
		col_val=1;
	else if(c2==0)
		col_val=2;
	else	
		col_val=3;
	while(!(c0&c1&c2&c3));//waiting for key release
	return(Key_Val[row_val][col_val]);
}

void password()
{
	unsigned char j=0,key;
	Write_CMD_LCD(0xC0);
	memset(pwd,'\0',10);          // pwd[10] array all elements are set 0
	while(1)
	{
		key=KeyValue();
		if(key=='#')       //#==enter
			break;
		else if(key=='*')  //*==delete(backspace)
		{
			if(j!=0)
			{
				j--;
				Write_CMD_LCD(0x10);  //shift curser to left
				Write_DAT_LCD(' ');         //modifing last written character or integer   with space 
				Write_CMD_LCD(0x10);  //  and again shift curser to left
			}
		}
		else
		{
			pwd[j++]= key;
			Write_DAT_LCD(pwd[j-1]);					/* it will show current character which you entered,in password typing time
		 //Write_DAT_LCD('*');                it will not show current character which you entered,it is for security purpose
																						[NOTE]-  USE ANY ONE OUT OF BOTH   */
		}
	}
	pwd[j]='\0';
}

void modify_password()
{
	char i,str[7],str1[7];
	Write_CMD_LCD(0x01);
	Write_CMD_LCD(0x80);
	Write_str_LCD("Current PWD please");          
	password();														//Again asking current password for security purpose
	delay_ms(1000);
	memset(str,'\0',7);
	for(i=1;i<6;i++)
			str[i-1] = i2cDevRandomRead(0xA0,i);
	str[i-1] = '\0';
	if(!strcmp(str,pwd))                     //if password matched ,loop body will excute
	{
		Write_CMD_LCD(0x01);
		Write_CMD_LCD(0x80);
		Write_str_LCD("PWD is Valid..");
		Write_CMD_LCD(0x01);
		Write_CMD_LCD(0x80);	
    Write_str_LCD("Enter New PWD");
		password();                           // asking for new password ,which we want to set
		strcpy(str1,pwd);											// coping latest typed pasa in str1 array
		Write_CMD_LCD(0x01);
		Write_CMD_LCD(0x80);	
    Write_str_LCD("confirm New PWD");     
		password();                           // asking to type latest password which we want to set [ confirmaation ]
		if(!strcmp(str1,pwd))									// if matched, loop body will excute
		{
			Write_CMD_LCD(0x01);
		  Write_CMD_LCD(0x80);
		  Write_str_LCD("PWD is Valid..");
			delay_ms(500);
			for(i=0;pwd[i];i++)                  //  writing latest password inside EEPROM's location
			    i2cDevByteWrite(0xA0,i+1,pwd[i]);
			
			Write_CMD_LCD(0x01);	
      Write_str_LCD("New PWD Updated");
			delay_ms(1500);
		}
		else                                  	//new pass & confirm new pass  will not matched then this else will excute 
		{
			Write_CMD_LCD(0x01);	
      Write_str_LCD("Wrong PWD");
			Write_CMD_LCD(0xC0);	
      Write_str_LCD("Try Again...");
		  delay_ms(500);
		}
	}
	else																// current pass confirmation will not match then this else will excute
	{
		  Write_CMD_LCD(0x01);	
      Write_str_LCD("Wrong PWD");
			Write_CMD_LCD(0xC0);	
      Write_str_LCD("Try Again...");
		  delay_ms(500);
	}
}
	
