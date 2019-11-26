/**************************************************
*
*作者    ：黄晖
*
*程序描述：NB-IOT设备（M5310A）驱动
*
*version ：V1.0
*编写日期：2019.6.4
*版本说明： 初始版本，使用NB_IOT直连华为云
*
*version ：V1.1
*编写日期：2019.10.28
*版本说明：	1、修改为使用结构体存储NB_IOT的标识码等信息
			2、增加一些使用http协议相关程序
***************************************************/

#ifndef __NB_IOT_H
#define	__NB_IOT_H

#include <rtthread.h>
#include <at.h>

typedef	struct
{
	char NB_IMSI[16];//存放NB_IOT设备SIM卡的IMSI号
	char NB_CGSN[16];//存放NB_IOT设备唯一标识码
	char NB_SIGNAL[2];//存放NB_IOT设备信号强度
	rt_int16_t net_sta;//注册网络状况
	
	char  HTTP_EXAMPLE;	//http的实例
	char *HTTP_URL;		//http的URL数据
	char *HTTP_HEADER;	//http的请求头
	char *HTTP_CONTENT;	//http的请求体
	char *HTTP_SEND_URL;//http的请求路径
	
}IOT_DATA_t;

int NB_IOT(void);
	
int NB_IOT_Init(IOT_DATA_t *IOT_DATA,const char *uartx,rt_size_t recv_bufsz);

int NB_IOT_GetCIMI(IOT_DATA_t *IOT_DATA);		//获取设备SIM的IMSI号
int NB_IOT_GetCGSN(IOT_DATA_t *IOT_DATA);		//获取设备唯一标识码
int NB_IOT_GetSignal(IOT_DATA_t *IOT_DATA);		//获取NB_IOT设备的信号强度
int NB_IOT_ConsultNet(void);	//查询设备注网情况

int NB_IOT_HTTPCREATE(IOT_DATA_t *args);
int NB_IOT_HTTPHEADER(IOT_DATA_t *args);
int NB_IOT_HTTPCONTENT(IOT_DATA_t *args);
int NB_IOT_HTTPSEND(IOT_DATA_t *args,int mode);
int NB_IOT_HTTPCLOSE(IOT_DATA_t *args);

int NB_IOT_Connect(char *IP);	//设备连接华为云

int NB_IOT_SendData(char *arr,int length);
int at_client_port_init(void);	// URC数据处理初始化

#endif

