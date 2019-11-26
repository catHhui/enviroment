/**************************************************
*
*作者    ：黄晖
*
*程序描述：LED显示屏（12接口）驱动（四分之一扫描方式）
*
*version ：V1.0
*编写日期：2019.11.9
*
*version ：V1.1
*编写日期：2019.11.19
*版本说明： 1、增加12号字体显示
			2、优化显示方式
***************************************************/

#include "LED_12.h"
#include "LED_font.h"
#include <rtdevice.h>
#include <drv_spi.h>

struct rt_spi_device *spi_dev;
struct rt_spi_configuration cfg;

rt_uint8_t LED_DATA[4*3][16*3]={0};

/*
**************************************************
* 函数名称：
* 函数功能：
* 入口参数：			
* 返回参数：
* 说明：    
***************************************************
*/
int LED_12_Init(void)
{
	/*设置LED显示屏12接口行选择引脚模式*/
	rt_pin_mode(P12_A,PIN_MODE_OUTPUT);
	rt_pin_mode(P12_B,PIN_MODE_OUTPUT);
	rt_pin_write(P12_A,PIN_LOW);
	rt_pin_write(P12_B,PIN_LOW);
	
	/*设置LED显示屏12接口片选引脚模式*/
	rt_pin_mode(P12_OE1,PIN_MODE_OUTPUT);
	rt_pin_write(P12_OE1,PIN_HIGH);
	rt_pin_mode(P12_OE2,PIN_MODE_OUTPUT);
	rt_pin_write(P12_OE2,PIN_HIGH);
	rt_pin_mode(P12_OE3,PIN_MODE_OUTPUT);
	rt_pin_write(P12_OE3,PIN_HIGH);
	
	//PA4为SPI片选引脚
	stm32_spi_bus_attach_device(40,"spi1","spi10");
	
	/* 查找 spi 设备获取设备句柄 */
    spi_dev = (struct rt_spi_device *)rt_device_find("spi10");
	
	//配置spi
    cfg.data_width = 8;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MODE_0 | RT_SPI_MSB;
    cfg.max_hz = 1000*1000;   //1Mhz
	rt_spi_configure(spi_dev,&cfg);

	return 0;
}
/*
**************************************************
* 函数名称：LED_SelectLine
* 函数功能：
* 入口参数：line：行号			
* 返回参数：
* 说明：    
***************************************************
*/
void LED_SelectLine(int line)
{
	rt_pin_write(P12_A,line&0X01);
	rt_pin_write(P12_B,line&0X02);
	
}

/*
**************************************************
* 函数名称：LED_Showchar
* 函数功能：在指定位置显示一个字符
* 入口参数：x :起始 X 坐标（0~11）
			y :起始 Y 坐标（0~11）
			size:要显示字符的字号
			ch:要显示的字符
			
* 返回参数：
* 说明：    
***************************************************
*/
void LED_ShowChar(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,rt_uint8_t ch)
{
	int i,j,t;
	
	ch-=' ';
	
	t=y/4;
	y%=4;
	
	if((size==12)&&(y==0))
		i=1;
	else
		i=0;
	for(;i<(4-y);i++)
	{
		for(j=0;j<4;j++)
		{
			if(size==12)
				LED_DATA[j+4*t][i+4*x]=ASCII_12[ch][12+j-4*i-4*y];
			else
				LED_DATA[j+4*t][i+4*x]=ASCII_16[ch][12+j-4*i-4*y];
		}
	}
	
	if(t>=2)	//超出显示范围，结束
		goto OUT;
	
	if(size==12)
		i=5-y;
	else
		i=4-y;
	for(;i<4;i++)
	{
		for(j=4;j<8;j++)
		{
			if(size==12)
				LED_DATA[j+4*t][i+4*x]=ASCII_12[ch][24+j-4*i-4*y];
			else
				LED_DATA[j+4*t][i+4*x]=ASCII_16[ch][24+j-4*i-4*y];
		}
	}
	
OUT:	
	;
}

/*
**************************************************
* 函数名称：LED_ShowString
* 函数功能：在指定位置显示一个字符
* 入口参数：str:要显示的字符串
			x :起始 X 坐标（0~11）
			y :起始 Y 坐标（0~11）
* 返回参数：
* 说明：    
***************************************************
*/
void LED_ShowString(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,char *str)
{
	
	while((*str<='~')&&(*str>=' '))//判断是不是非法字符!
	{
		LED_ShowChar(x,y,size,*str);
		x++;
		if(x>11)
			break;
		str++;
	}
}
/*
**************************************************
* 函数名称：LED_ShowChese
* 函数功能：在指定位置显示一个汉字
* 入口参数：str:要显示的汉字
			x :起始 X 坐标（0~10）
			y :起始 Y 坐标（0~11）
* 返回参数：
* 说明：    
***************************************************
*/
void LED_ShowChese(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,char *str)
{
	int i,j,t;
	int index=0;
	
	for(i=0;i<10;i++)
	{
		if(*str==*Chese_index[i])
			index=i*2;
	}
	
	t=y/4;
	y%=4;
	
	if((size==12)&&(y==0))
		i=1;
	else
		i=0;
	for(;i<(4-y);i++)
	{
		for(j=0;j<4;j++)
		{
			if(size==12)
			{
				LED_DATA[j+4*t][i+4*x]=Chese_12[index][12+j-4*i-4*y];
				LED_DATA[j+4*t][i+4*x+4]=Chese_12[index+1][12+j-4*i-4*y];
			}
			else
			{
				LED_DATA[j+4*t][i+4*x]=Chese_16[index][12+j-4*i-4*y];
				LED_DATA[j+4*t][i+4*x+4]=Chese_16[index+1][12+j-4*i-4*y];
			}
		}
	}
	
	if(t>=2)	//超出显示范围，结束
		goto OUT;
	
	if(size==12)
		i=5-y;
	else
		i=4-y;
	for(;i<4;i++)
	{
		for(j=4;j<8;j++)
		{
			if(size==12)
			{
				LED_DATA[j+4*t][i+4*x]=Chese_12[index][24+j-4*i-4*y];
				LED_DATA[j+4*t][i+4*x+4]=Chese_12[index+1][24+j-4*i-4*y];
			}
			else
			{
				LED_DATA[j+4*t][i+4*x]=Chese_16[index][24+j-4*i-4*y];
				LED_DATA[j+4*t][i+4*x+4]=Chese_16[index+1][24+j-4*i-4*y];
			}
		}
	}
	
OUT:	
	;
}
/*
**************************************************
* 函数名称：LED_ShowString
* 函数功能：在指定位置显示多个汉字
* 入口参数：str:要显示的汉字
			x :起始 X 坐标（0~10）
			y :起始 Y 坐标（0~11）
* 返回参数：
* 说明：    
***************************************************
*/
void LED_ShowNChese(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,rt_uint16_t n,char *str)
{
	
	while(1)
	{
		LED_ShowChese(x,y,size,str);
		x+=2;
		n-=1;
		if(n<=0)
			break;
		if(x>10)
			break;
		
		str+=2;
	}
	
}
/*
**************************************************
* 函数名称：
* 函数功能：
* 入口参数：			
* 返回参数：
* 说明：    
***************************************************
*/
void LED_Clear(void)
{
	int i,j;
	
	for(i=0;i<12;i++)
	{
		for(j=0;j<48;j++)
		{
			LED_DATA[i][j]=0;
		}
	}
}

/*
**************************************************
* 函数名称：
* 函数功能：
* 入口参数：			
* 返回参数：
* 说明：    
***************************************************
*/
void LED_SendData(void)
{
	int row;
	
	for(row=0;row<4;row++)
	{
		rt_pin_write(P12_OE1,PIN_HIGH);
		
		LED_SelectLine(row);
		rt_spi_send(spi_dev,LED_DATA[row],48);
		
		rt_pin_write(P12_OE1,PIN_LOW);
		rt_hw_us_delay(800);
		rt_pin_write(P12_OE1,PIN_HIGH);
		
		rt_pin_write(P12_OE2,PIN_HIGH);
		
		LED_SelectLine(row);
		rt_spi_send(spi_dev,LED_DATA[row+4],48);
		
		rt_pin_write(P12_OE2,PIN_LOW);
		rt_hw_us_delay(800);
		rt_pin_write(P12_OE2,PIN_HIGH);
		
		rt_pin_write(P12_OE3,PIN_HIGH);
		
		LED_SelectLine(row);
		rt_spi_send(spi_dev,LED_DATA[row+8],48);
		
		rt_pin_write(P12_OE3,PIN_LOW);
		rt_hw_us_delay(800);
		rt_pin_write(P12_OE3,PIN_HIGH);
	}
	
}


/*
**************************************************
* 函数名称：
* 函数功能：
* 入口参数：parameter
* 返回参数：无
* 说明：    
***************************************************
*/

//static void LED_thread_entry(void *parameter)
//{

//	LED_12_Init();
//	
////	LED_Clear();
//	LED_ShowNChese(3,0,12,4,"扬尘监测");
//	LED_ShowString(0,3,12,"PM");
//	LED_ShowChese(2,3,12,"值");
//	LED_ShowString(0,6,12," 2.5:--ug/m3");
//	LED_ShowString(0,9,12," 10 :--ug/m3");
////	LED_ShowNChese(0,3,12,2,"温度");
////	LED_ShowString(4,3,12,": --.-C");
////	LED_ShowNChese(0,6,12,2,"湿度");
////	LED_ShowString(4,6,12,": --.-%");
////	LED_ShowNChese(0,9,12,2,"噪音");
////	LED_ShowString(4,9,12,": --.-db");
//	while(1)
//	{
//		LED_SendData();
////		rt_thread_mdelay(10);
//	}
//	
//}

//int LED(void)
//{
//	rt_err_t ret = RT_EOK;
//	
//	/*创建线程*/
//	rt_thread_t thread=rt_thread_create("BYH_M",LED_thread_entry,RT_NULL,1024,25,10);
//	if(thread !=RT_NULL)
//	{
//		rt_thread_startup(thread);
//	}
//	else
//	{
//		ret = RT_ERROR;
//	}
//	
//	return ret;
//}
///**/
//MSH_CMD_EXPORT(LED, LED thread);
//INIT_COMPONENT_EXPORT(LED);

