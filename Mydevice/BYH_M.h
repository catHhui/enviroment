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

#ifndef __BYH_M_H
#define	__BYH_M_H

#include <rtthread.h>


/*BYH_M传感器数据块*/
typedef struct 
{
	rt_uint8_t BYH_M_CMD[6];	//存放 BYH_M 基本命令
	
	rt_int16_t Temperature_x10;	//存放温度值 x10
	rt_int16_t Humidity_x10;	//存放湿度值 x10
	rt_int16_t Noise_x10;		//存放噪音值 x10
	
	rt_int16_t PM2_5;			//存放PM2.5值
	rt_int16_t PM10;			//存放PM10值
	
}BYH_M_t;

int BYH_M_init(const char *uartx,rt_uint32_t baud_rate);//BYH_M设备初始化函数
	
rt_uint16_t CRC16_ModbusRTU(rt_uint8_t *puchMsg, rt_uint32_t usDataLen);//计算数据的校验值

rt_err_t BYH_M_get_humiture(BYH_M_t *BYH_M); //获取温湿度数据
rt_err_t BYH_M_get_noise(BYH_M_t *BYH_M);	//获取噪音值
rt_err_t BYH_M_get_PM_Data(BYH_M_t *BYH_M);	//获取PM2.5、PM10的值
	
#endif

