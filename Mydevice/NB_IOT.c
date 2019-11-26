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

#include "NB_IOT.h"
#include "string.h"

char NB_REC_DATA[20]={"NULL"};//存放华为云服务器下发的命令

int flag=0;

/*
**************************************************
* 函数名称：NB_IOT_Init
* 函数功能：NB_IOT设备初始化
* 入口参数：uartx：串口号
			recv_bufsz：最大接收数据长度
* 返回参数：0-设备初始化成功
			1-设备初始化失败
* 说明：    
***************************************************
*/
int NB_IOT_Init(IOT_DATA_t *IOT_DATA,const char *uartx,rt_size_t recv_bufsz)
{
	int i=0;
	
	rt_thread_mdelay(2000);
	
	/* AT client 初始化,使用uartx设备，最大接收数据长度为recv_bufsz*/
	at_client_init(uartx,recv_bufsz);
	
	at_client_port_init();
	
	rt_kprintf("nb_iot init...\n");
	rt_thread_mdelay(5000);
	
	if(flag==0)
	/* 确认模块连接正常 */
	/*启动时循环发送 AT 命令，直到模块响应数据,超时时间为 0.5 秒*/
    if(at_client_wait_connect(500)!=RT_EOK)
	{
		rt_kprintf("[error]nb_iot don't connect!\n");
		return RT_ERROR;
	}
	
	NB_IOT_GetCGSN(IOT_DATA);
	rt_kprintf("CGSN=%s\n",IOT_DATA->NB_CGSN);
	
	for(i=0;i<10;i++)
	{
		if(NB_IOT_GetCIMI(IOT_DATA)==RT_EOK)
		{
			break;
		}
		rt_thread_mdelay(500);
	}
	rt_kprintf("CIMI=%s\n",IOT_DATA->NB_IMSI);
	
	for(i=0;i<10;i++)
	{
		NB_IOT_GetSignal(IOT_DATA);
		if(IOT_DATA->NB_SIGNAL[0]!='9')
		{
			break;
		}
		rt_thread_mdelay(500);
	}
	rt_kprintf("SIGNAL=%s\n",IOT_DATA->NB_SIGNAL);
	
	i=0;
	while(NB_IOT_ConsultNet()!=RT_EOK)
	{
		rt_thread_mdelay(500);
		i++;
	}
	IOT_DATA->net_sta=1;
	
	rt_kprintf("NB_IOT(M5310-A) device initialize success.\n");
	
	return RT_EOK;
	
}

/*
**************************************************
* 函数名称：NB_IOT_GetCGSN
* 函数功能：获取设备唯一标识码
* 入口参数：无
* 返回参数：RT_EOK-获取成功
			RT_ERROR-获取失败
* 说明：    
***************************************************
*/
int NB_IOT_GetCGSN(IOT_DATA_t *IOT_DATA)
{
	rt_err_t result = RT_EOK;
	const char *CGSN;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，获取设备唯一标识码 */
	rt_thread_mdelay(20);
    if ( at_exec_cmd(resp, "AT+CGSN=1") != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		CGSN=at_resp_get_line_by_kw(resp,"+CGSN:");
		
		for(int i=0;i<15;i++)
		{
			IOT_DATA->NB_CGSN[i]=*(CGSN+i+6);
			if(i==14)
				IOT_DATA->NB_CGSN[i+1]='\0';
		}
	}else
		result=RT_ERROR;
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
}


/*
**************************************************
* 函数名称：NB_IOT_GetCIMI
* 函数功能：获取设备SIM的IMSI号
* 入口参数：无
* 返回参数：RT_EOK-获取成功
			RT_ERROR-获取失败
* 说明：    
***************************************************
*/
int NB_IOT_GetCIMI(IOT_DATA_t *IOT_DATA)
{
	rt_err_t result = RT_EOK;
	const char *CIMI;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，获取SIM的IMSI号 */
	rt_thread_mdelay(20);
    if (at_exec_cmd(resp, "AT+CIMI")!= RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		CIMI=at_resp_get_line(resp,2);
		for(int i=0;i<15;i++)
		{
			IOT_DATA->NB_IMSI[i]=*CIMI++;
			if(i==14)
				IOT_DATA->NB_IMSI[i+1]='\0';
		}
		result = RT_EOK;
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
}

/*
**************************************************
* 函数名称：NB_IOT_GetSignal
* 函数功能：获取NB_IOT设备的信号强度
* 入口参数：无
* 返回参数：RT_EOK-连接成功
			RT_ERROR-连接失败
* 说明：    模块返回 +CSQ:30,99 ,前面31信号满格
			范围：12~31 ，低于12数据可能发送失败
***************************************************
*/
int NB_IOT_GetSignal(IOT_DATA_t *IOT_DATA)
{
	rt_err_t result = RT_EOK;
	const char *signal;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，查询注网情况，返回 +CEREG:0,1 ，注网成功 */
	rt_thread_mdelay(20);
    if (at_exec_cmd(resp, "AT+CSQ") != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		signal=at_resp_get_line_by_kw(resp,"+CSQ:");
		if(*(signal+6)==',' )
		{
			IOT_DATA->NB_SIGNAL[0]='0';
			IOT_DATA->NB_SIGNAL[1]=*(signal+5);
		}else if(*(signal+7)==',' && *(signal+5)=='9' && *(signal+6)=='9')
		{
			IOT_DATA->NB_SIGNAL[0]='9';
			IOT_DATA->NB_SIGNAL[1]='9';
			result=RT_ERROR;
		}
		else if(*(signal+7)==',' && ((*(signal+5)>'0' && *(signal+6)>'2')||*(signal+5)>'2'))
		{
			IOT_DATA->NB_SIGNAL[0]=*(signal+5);
			IOT_DATA->NB_SIGNAL[1]=*(signal+6);
		}
	}
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
}

/*
**************************************************
* 函数名称：NB_IOT_ConsultNet
* 函数功能：查询注网情况
* 入口参数：无
* 返回参数：RT_EOK-注网成功
			RT_ERROR-尚未注网
* 说明：    模块返回 +CEREG:0,1 表示注网成功
***************************************************
*/
int NB_IOT_ConsultNet(void)
{
	rt_err_t result = RT_ERROR;
//	const char *status;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，查询注网情况，返回 +CEREG:0,1 ，注网成功 */
	rt_thread_mdelay(20);
    if (at_exec_cmd(resp, "AT+CEREG?") != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
		
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		if(at_resp_get_line_by_kw(resp,"+CEREG:0,1")!=NULL)
			result=RT_EOK;
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
}

/*
**************************************************
* 函数名称：NB_IOT_HTTPCREATE
* 函数功能：
* 入口参数：	
* 返回参数：
* 说明：    
***************************************************
*/
int NB_IOT_HTTPCREATE(IOT_DATA_t *args)
{
	rt_err_t result = RT_EOK;
	char tmp[50];
	const char *example;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，创建http实例 */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPCREATE=\"%s\"",args->HTTP_URL);
    if (at_exec_cmd(resp, tmp) != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		example=at_resp_get_line_by_kw(resp,"+HTTPCREATE:");
		
		args->HTTP_EXAMPLE=*(example+12);
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* 函数名称：NB_IOT_HTTPHEADER
* 函数功能：
* 入口参数：	
* 返回参数：
* 说明：    
***************************************************
*/
int NB_IOT_HTTPHEADER(IOT_DATA_t *args)
{
	rt_err_t result = RT_EOK;
	char tmp[AT_CMD_MAX_LEN];
	int i=0;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，设置请求头 */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPHEADER=%c,\"%s\"",args->HTTP_EXAMPLE,args->HTTP_HEADER);
	
	i=at_exec_cmd(resp, tmp);
	if(i!=RT_EOK)
    {
		if(i==-RT_ETIMEOUT)
			result=RT_ETIMEOUT;
		else
			result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{	
		result=RT_EOK;
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* 函数名称：NB_IOT_HTTPCONTENT
* 函数功能：
* 入口参数：	
* 返回参数：
* 说明：    
***************************************************
*/
int NB_IOT_HTTPCONTENT(IOT_DATA_t *args)
{
	rt_err_t result = RT_ERROR;
	char tmp[AT_CMD_MAX_LEN];
	int i;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，设置连接信息，即请求体 */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPCONTENT=%c,\"%s\"",args->HTTP_EXAMPLE,args->HTTP_CONTENT);
	
	i=at_exec_cmd(resp, tmp);
	if(i!=RT_EOK)
    {
		if(i==-RT_ETIMEOUT)
			result=RT_ETIMEOUT;
		else
			result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		result=RT_EOK;
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* 函数名称：NB_IOT_HTTPSEND
* 函数功能：
* 入口参数：	
* 返回参数：
* 说明：    
***************************************************
*/
int NB_IOT_HTTPSEND(IOT_DATA_t *args,int mode)
{
	rt_err_t result = RT_ERROR;
	char tmp[AT_CMD_MAX_LEN];
	int i;
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 64 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(64, 2, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，发送http数据 */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPSEND=%c,%d,\"%s\"",args->HTTP_EXAMPLE,mode,args->HTTP_SEND_URL);
	
	i=at_exec_cmd(resp, tmp);
	if(i!=RT_EOK)
    {
		if(i==-RT_ETIMEOUT)
			result=RT_ETIMEOUT;
		else
			result=RT_ERROR;;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		result = RT_EOK;
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* 函数名称：NB_IOT_HTTPCLOSE
* 函数功能：
* 入口参数：	
* 返回参数：
* 说明：    
***************************************************
*/
int NB_IOT_HTTPCLOSE(IOT_DATA_t *args)
{
	rt_err_t result = RT_ERROR;
	char tmp[30];
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 32 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(32, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，关闭HTTP实例 */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPCLOSE=%c",args->HTTP_EXAMPLE);
	
	if(at_exec_cmd(resp, tmp)!=RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		result=RT_EOK;
	}
	
__exit:
	/*删除响应结构体*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* 函数名称：NB_IOT_Connect
* 函数功能：设备连接华为云
* 入口参数：IP：华为云提供的连接IP
* 返回参数：0-连接成功
			1-连接失败
* 说明：    
***************************************************
*/
int NB_IOT_Connect(char *IP)
{
	rt_err_t result = RT_EOK;
	int i=0;
	char cmd[25]={"AT+NCDP="};
	
	for(i=8;i<25;i++)
	{
		if(*(IP)=='\0')
		{
			cmd[i]='\0';
			break;
		}
		cmd[i]=*(IP++);
	}
	rt_kprintf("%s\n",cmd);
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 16 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(16, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* 发送AT指令，确认设备已连接服务器 */
    rt_thread_mdelay(1000);
    if (at_exec_cmd(resp, cmd) != RT_EOK)
    {
        rt_kprintf("指令发送失败!\n");
		result=RT_ERROR;
        goto __exit;		//发送指令失败，跳转到删除响应结构体
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
		rt_kprintf("服务器连接成功!\n");
	else if(at_resp_get_line_by_kw(resp,"ERROR")!=NULL)
	{
		rt_kprintf("服务器连接失败!\n");
		result=RT_ERROR;
	}
	
__exit:
    /*删除响应结构体*/
    at_delete_resp(resp);
    	
	return result;
}

///////////////////////////////////////////
//数据转换函数
char* Char2ASCII(char ch)
{
	switch(ch)
	{
		case '0': return "30";
		case '1': return "31";
		case '2': return "32";
		case '3': return "33";
		case '4': return "34";
		case '5': return "35";
		case '6': return "36";
		case '7': return "37";
		case '8': return "38";
		case '9': return "39";
		case '.': return "2E";
		default : return "00";		
	}
}
//////////////////////////////////////////////
/*
**************************************************
* 函数名称：NB_IOT_SendData
* 函数功能：设备向华为云发送数据
* 入口参数：arr：要发送的数据
			length：发送数据的长度
* 返回参数：0-数据发送成功
			1-数据发送失败
* 说明：    
***************************************************
*/
int NB_IOT_SendData(char *arr,int length)
{	
	int i=0;
	char a,b;
	char *c;
	char cmd[100]={"AT+NMGS="};
	
	a=(int)length/10+48;
	b=length%10+48;

	if(a=='0')
	{
		cmd[8]=b;
		cmd[9]=',';
		cmd[10]='0';
		cmd[11]='0';
		i=12;
	}else
	{
		cmd[8]=a;
		cmd[9]=b;
		cmd[10]=',';
		cmd[11]='0';
		cmd[12]='0';
		i=13;
	}
	for(;i<100;i=i+2)
	{
		if(*arr=='\0')break;
		c=Char2ASCII(*(arr++));
		if(c[0]=='0')
		{
			i=i-2;
		}else
		{
			cmd[i]=c[0];
			cmd[i+1]=c[1];
		}
	}
		
	rt_kprintf("%s\n",cmd);
	
	
	/*定义响应结构体句柄*/
	at_response_t resp=RT_NULL;
	
	/* 创建响应结构体，设置最大支持响应数据长度为 512 字节，响应数据行数无限制，超时时间为 5 秒 */
    resp=at_create_resp(512, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
    /* 发送 AT 命令并接收 AT Server 响应数据，数据及信息存放在 resp 结构体中 */
	rt_thread_mdelay(1000);
    if (at_exec_cmd(resp, cmd) != RT_EOK)
    {
        rt_kprintf("指令发送失败!\n");
		at_delete_resp(resp);
		return RT_ERROR;
    }else if(at_resp_get_line_by_kw(resp,"ERROR")!=NULL)
	{
		rt_kprintf("数据发送失败!\n");
		at_delete_resp(resp);
		return RT_ERROR;
	}else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		rt_kprintf("数据发送成功!\n");		
	}
	
	/* 删除响应结构体 */
    at_delete_resp(resp);
	
	return 0;
}
/////////////////////////////////////////////////////////
// URC 数据执行函数
static void urc_func(const char *data, rt_size_t size)
{
//	rt_kprintf("收到数据为：\n");
//	rt_kprintf("%s\n",data);
	if(strcmp(data,"+NNMI:4,AAAA0000"))
	{
		rt_kprintf("服务器接收数据成功!\n");
	}else
	{
		rt_kprintf("服务器接收数据失败!\n");
	}
	
}

static void urc_Inited_func(const char *data, rt_size_t size)
{
    /* iot 模块上电初始化完成 */
	rt_kprintf("nb_iot connect!\n");
	flag=1;
}

static void urc_connect_func(const char *data, rt_size_t size)
{
    /* 服务器连接成功信息 */
    rt_kprintf("Server connect success!\n");
	rt_kprintf("%s\n",data);
}

static void urc_discon_func(const char *data, rt_size_t size)
{
    /* 服务器断开连接信息 */
    rt_kprintf("Server disconnect!\n");
	rt_kprintf("%s\n",data);
}

static void urc_httpHeader_func(const char *data, rt_size_t size)
{
    /* 接收到 HTTP 头部信息 */
	char tmp[142]={0};
	
//	at_client_recv(tmp,118,5000);
//    rt_kprintf("接收到 HTTP 头部信息:\n");
	rt_kprintf("%s",data);
//	rt_kprintf("%s",tmp);
	rt_kprintf("\n");
	
	
}

static void urc_httpContent_func(const char *data, rt_size_t size)
{
    /* 接收到 HTTP content 数据信息 */
	char tmp[30],tmp1[4]={0};
	
	at_client_recv(tmp1,3,5000);
//    rt_kprintf("接收到 HTTP content 数据信息:\n");
	rt_kprintf("%s",data);
	rt_kprintf("%s\n",tmp1);
	
	if(tmp1[0]=='o'&&tmp1[1]=='k')
		rt_kprintf("send data sucefull!\n");
	if(*(data+12)=='0')
	{
		/*定义响应结构体句柄*/
		at_response_t resp=RT_NULL;
		
		/* 创建响应结构体，设置最大支持响应数据长度为 32 字节，响应数据行数无限制，超时时间为 5 秒 */
		resp=at_create_resp(64, 2, rt_tick_from_millisecond(500));
		if (resp == RT_NULL)
		{
			rt_kprintf("No memory for response structure!\n");		
		}
		
		/* 发送AT指令，关闭HTTP实例 */
		rt_thread_mdelay(20);
		rt_sprintf(tmp,"AT+HTTPCLOSE=%c",*(data+10));
		
		if(at_exec_cmd(resp, tmp)!=RT_ERROR)
		{
		   rt_kprintf("HTTP %c closed!\n",*(data+10));
		}else
			rt_kprintf("HTTP %c\n!",*(data+10));
		if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
		{
			rt_kprintf("HTTP %c closed!\n",*(data+10));
		}
		/*删除响应结构体*/
		at_delete_resp(resp);
		
	}
}

/* URC数据表，前缀 ，后缀 ，执行函数 */
static struct at_urc urc_table[] = {
    {"+NNMI:","00", urc_func},
	{"M5310-A","\r\n", urc_Inited_func},
	{"CONNECT OK","\r\n", urc_connect_func},
	{"+HTTPDISCON:","\r\n", urc_discon_func},
	{"+HTTPNMIH:","\r\n", urc_httpHeader_func},
	{"+HTTPNMIC:","\r\n", urc_httpContent_func},
};

int at_client_port_init(void)
{
    /*  添加多种 URC 数据至 URC 列表中，当接收到同时匹配 URC 前缀和后缀的数据，执行 URC 函数  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
	
    return RT_EOK;
}

