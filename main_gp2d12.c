#include <reg51.h>
#include "types.h"
#include "mcp3204.h"
#include "lcd.h"
#include "delay.h"
#include "i2c.h"
#include "keypad.h"
#include "lcd.h"
#include <string.h>


void modify_password(void);
void password(void);
char KeyValue(void);
void editfunction(void);
void modify_setpoint(void);

sbit led = P3^7;
bit flag;

extern unsigned char pwd[10];
void EINT0(void) interrupt 0
{
	flag =1;
}

main()
{

	u8 RangeCM,setpoint,i;
	u16 Volts;
	f32 AnalogReading; 
	
	IE=0x81;
	IT0=1;
	flag=0;
	LCD_Init();
	
	//i2cDevByteWrite(0xA0,0x00,20)
	//password lenghth must be 5 bytes
	
	Write_CMD_LCD(0x80);
	for(i=0;i<5;i++)                            //set default password 12345
			i2cDevByteWrite(0xA0,i+1,'1'+i);
	

while(1)
 {	
	Write_CMD_LCD(0x80);
	Write_str_LCD("S.P: ");
	setpoint=i2cDevRandomRead(0xA0,0x00);
	Write_int_LCD(setpoint);
	Write_str_LCD(" CM  ");
	delay_ms(40);
	AnalogReading=Read1_ADC_MCP3204(0);
	Volts=AnalogReading * (1024/5);
	RangeCM=(6787/(Volts-3))-4;  
	
		Write_CMD_LCD(0xC0);
	  Write_str_LCD("C.D: ");
		Write_int_LCD(RangeCM);
		Write_str_LCD(" CM  ");
  	
		if(RangeCM > setpoint)       //if Car distance is greater than fixed distance (setpoint)
				led=0;								   //	then  led is OFF
		else
			led=1;											//otherwise led is O
		
		if(flag)                      // EINT ISR   ( ext.interrupt 0 push button ISR)
		{
			flag=0;
			editfunction();             
		}
		delay_ms(1000);
  }
}

void editfunction()
{
	char i,str[7],set=0,key,cnt=0;
	while(1)
	{
		Write_CMD_LCD(0x01);   //LCD screen clearing
	  Write_CMD_LCD(0x80);
		Write_str_LCD("Enter Password:");
		password();            //asking defaultpassword for Owner(root) permission
		for(i=1;i<6;i++)
			str[i-1] = i2cDevRandomRead(0xA0,i);  //reading default password from EEPROM{location 0x01,0x02,0x03,0x04,0x05}
																				    // and coping that byte data into str    {str[0],str[1],str[2],str[3],str[4]}  
		str[i-1] = '\0';
	  if(!strcmp(str,pwd))            //comaparing typed password with defautl password[1st time default, 2nd time current]    
	  {																//if both matched then control come inside loop
			cnt=0;
		  Write_CMD_LCD(0x01);          //LCD screen clearing
			Write_str_LCD("correct password");
			delay_ms(500);
			Write_CMD_LCD(0x01);
	    Write_CMD_LCD(0x80);          		//LCD screen's first line, first element position select
		  Write_str_LCD("1. PWD change");
			Write_CMD_LCD(0xC0);							//LCD screen's second line, first element position select
		  Write_str_LCD("2. S.P change");
			key=KeyValue();
			if(key=='1')
				 modify_password();             //modifing current password 
			else if(key=='2')
				 modify_setpoint();							//modifing set point 
			else															//invalid choice 
			{
				Write_CMD_LCD(0x01);
				Write_CMD_LCD(0x80);
				Write_str_LCD("Wrong Selection:");
				delay_ms(1000);
			}
			break;
		}
		else
		{
			Write_CMD_LCD(0x01);
			Write_str_LCD("Wrong password");          //current password not matched
			delay_ms(500);
			cnt++;																		//unsuccessfull attempts count increment
			if(cnt==3)                                
			{
				Write_CMD_LCD(0x01);
			  Write_str_LCD("Too many attempt");
				Write_CMD_LCD(0xC0);
			  Write_str_LCD("System Blocked");
				delay_ms(500);
				while(1);
			}
		}	
	 }
	Write_CMD_LCD(0x01);
}

void modify_setpoint()              // modifing set point 
{
	char set=0,key;
	Write_CMD_LCD(0x01);							//clearing LCD screen
	Write_str_LCD("Enter S.P(10-80)");
	while(1)
	{
		key = KeyValue();
		if(key == '#')
			break;
		else
		{
			set = (set*10) + (key-48);
			Write_CMD_LCD(0xC0);
			Write_int_LCD(set);
		}
	}
	if(set>10 && set<80)
	{
		Write_CMD_LCD(0x01);
		Write_str_LCD("Valid S.P");
	  delay_ms(1500);
		i2cDevByteWrite(0xA0,0x00,set);
		Write_CMD_LCD(0x01);
		Write_str_LCD("New S.P Updated");
		delay_ms(1500);
	}
	else
	{
		Write_CMD_LCD(0x01);
		Write_str_LCD("Invalid S.P");
		Write_CMD_LCD(0xC0);
		Write_str_LCD("Try Again");
		delay_ms(1500);
	}
}
