/**************************************************
*
*作者    ：黄晖
*
*程序描述：气象多要素百叶箱BYH-M（多传感器）驱动
*
*version ：V1.0
*编写日期：2019.10.14
*
*version ：V1.1
*编写日期：2019.10.28
*版本说明：	1、更改BYH_M传感器的修改控制结构体，使
		其使用更加方便；
			2、同时修改相关函数的输入参数。
***************************************************/

#include "BYH_M.h"
#include <rtdevice.h>


/* 串口设备句柄 */
static rt_device_t serial;

/*
**************************************************
* 函数名称：BYH_M_init
* 函数功能：BYH_M设备初始化函数
* 入口参数：baud_rate；波特率
* 返回参数：无
* 说明：    
***************************************************
*/
int BYH_M_init(const char *uartx,rt_uint32_t baud_rate)
{
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT; /* 配置参数 */
	
	/* 查找串口设备 */
	serial = rt_device_find(uartx);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uartx);
        return RT_ERROR;
    }
	
	/* 以中断接收及轮询发送模式打开串口设备 */
	rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
	
	/* 打开设备后才可修改串口配置参数 */
	config.baud_rate = baud_rate;
	rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
	
	return 0;
}

/*
**************************************************
* 函数名称：CRC16_ModbusRTU
* 函数功能：计算数据的校验值
* 入口参数：puchMsg:校验数据的地址
			usDataLen:校验数据的长度
* 返回参数：wCRCin:校验值
* 说明：    
***************************************************
*/
rt_uint16_t CRC16_ModbusRTU(rt_uint8_t *puchMsg, rt_uint32_t usDataLen)
{
	rt_uint16_t wCRCin = 0xFFFF;
	rt_uint16_t wCPoly = 0xa001;
	rt_uint16_t tmp=0;
	rt_uint8_t wChar = 0;
	rt_uint8_t i;
	
	while(usDataLen--) 	
	{
		wChar = *(puchMsg++);
		wCRCin ^= (wChar);
		for(i=0;i<8;i++)
		{
			if(wCRCin & 0x0001)
			wCRCin = (wCRCin >> 1) ^ wCPoly;
			else
			wCRCin = wCRCin >> 1;
		}
	}
	
	tmp=(wCRCin<<8)&0xff00;
	wCRCin=tmp|((wCRCin>>8)&0x00ff);
	
	return wCRCin;
}

/*
**************************************************
* 函数名称：BYH_M_get_humiture
* 函数功能：
* 入口参数：BYH_M: 传感器的数据结构体的地址			
* 返回参数：
* 说明：    
***************************************************
*/
rt_err_t BYH_M_get_humiture(BYH_M_t *BYH_M)
{
	rt_uint8_t The_CMD[8],temp[10];
	rt_uint16_t crc;
	rt_int16_t tmp;
	rt_int32_t i;
	
	for(i=0;i<6;i++)
	{
		The_CMD[i]=(BYH_M->BYH_M_CMD[i]);
	}
	The_CMD[3]+=0;	//寄存器地址加0
	The_CMD[5]+=2;	//读取两个寄存器的值
					//即温湿度
	
	/*得到发送数据的crc校验值*/
	crc=CRC16_ModbusRTU(The_CMD,6);
	The_CMD[6]=(rt_uint8_t)(crc>>8);
	The_CMD[7]=(rt_uint8_t)crc;
	
	rt_device_write(serial, 0, The_CMD, 8);
	
	/*读取接收到的数据*/
	rt_thread_mdelay(100);
	rt_device_read(serial,0,temp,9);
	
	/*校验接收的数据，如果错误就返回*/
	crc=temp[7];
	crc=(crc<<8)|temp[8];
	if(crc!=CRC16_ModbusRTU(temp,7))
		return RT_ERROR;
	
//	rt_device_write(serial, 0, temp,9);
	
	//保存湿度值
	tmp=temp[3];
	tmp=(tmp<<8)|temp[4];
	BYH_M->Humidity_x10=tmp;
	//保存温度值
	tmp=temp[5];
	tmp=(tmp<<8)|temp[6];
	BYH_M->Temperature_x10=tmp;
	
	return RT_EOK;
}

/*
**************************************************
* 函数名称：BYH_M_get_noise
* 函数功能：
* 入口参数：BYH_M: 传感器的数据结构体的地址	
* 返回参数：
* 说明：    
***************************************************
*/
rt_err_t BYH_M_get_noise(BYH_M_t *BYH_M)
{
	rt_uint8_t The_CMD[8],temp[10];
	rt_uint16_t crc;
	rt_int16_t tmp;
	rt_int32_t i;
	
	for(i=0;i<6;i++)
	{
		The_CMD[i]=(BYH_M->BYH_M_CMD[i]);
	}
	
	The_CMD[3]+=2;	//寄存器地址加2
	The_CMD[5]+=1;	//读取一个个寄存器的值
					//即噪声
	
	/*得到发送数据的crc校验值*/
	crc=CRC16_ModbusRTU(The_CMD,6);
	The_CMD[6]=(rt_uint8_t)(crc>>8);
	The_CMD[7]=(rt_uint8_t)crc;
	
	rt_device_write(serial, 0, The_CMD, 8);
	
	/*读取接收到的数据*/
	rt_thread_mdelay(100);
	rt_device_read(serial,0,temp,7);
	
	/*校验接收的数据，如果错误就返回*/
	crc=temp[5];
	crc=(crc<<8)|temp[6];
	if(crc!=CRC16_ModbusRTU(temp,5))
		return RT_ERROR;
	else
	{
		//保存噪音值
		tmp=temp[3];
		tmp=(tmp<<8)|temp[4];
		BYH_M->Noise_x10=tmp;
	}
	return RT_EOK;
}

/*
**************************************************
* 函数名称：BYH_M_get_PM_Data
* 函数功能：
* 入口参数：BYH_M: 传感器的数据结构体的地址		
* 返回参数：
* 说明：    
***************************************************
*/
rt_err_t BYH_M_get_PM_Data(BYH_M_t *BYH_M)
{
	rt_uint8_t The_CMD[8],temp[10];
	rt_uint16_t crc;
	rt_int16_t tmp;
	rt_int32_t i;
	
	for(i=0;i<6;i++)
	{
		The_CMD[i]=(BYH_M->BYH_M_CMD[i]);
	}
	The_CMD[3]+=3;	//寄存器地址加0
	The_CMD[5]+=2;	//读取两个寄存器的值
					//即PM2.5、PM10度
	
	/*得到发送数据的crc校验值*/
	crc=CRC16_ModbusRTU(The_CMD,6);
	The_CMD[6]=(rt_uint8_t)(crc>>8);
	The_CMD[7]=(rt_uint8_t)crc;
	
	rt_device_write(serial, 0, The_CMD, 8);
	
	/*读取接收到的数据*/
	rt_thread_mdelay(100);
	rt_device_read(serial,0,temp,9);
	
	/*校验接收的数据，如果错误就返回*/
	crc=temp[7];
	crc=(crc<<8)|temp[8];
	if(crc!=CRC16_ModbusRTU(temp,7))
		return RT_ERROR;
	
//	rt_device_write(serial, 0, temp,9);
	
	//保存PM2.5值
	tmp=temp[3];
	tmp=(tmp<<8)|temp[4];
	BYH_M->PM2_5=tmp;
	//保存PM10值
	tmp=temp[5];
	tmp=(tmp<<8)|temp[6];
	BYH_M->PM10=tmp;
	
	return RT_EOK;
}

