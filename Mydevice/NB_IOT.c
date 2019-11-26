/**************************************************
*
*����    ������
*
*����������NB-IOT�豸��M5310A������
*
*version ��V1.0
*��д���ڣ�2019.6.4
*�汾˵���� ��ʼ�汾��ʹ��NB_IOTֱ����Ϊ��
*
*version ��V1.1
*��д���ڣ�2019.10.28
*�汾˵����	1���޸�Ϊʹ�ýṹ��洢NB_IOT�ı�ʶ�����Ϣ
			2������һЩʹ��httpЭ����س���
***************************************************/

#include "NB_IOT.h"
#include "string.h"

char NB_REC_DATA[20]={"NULL"};//��Ż�Ϊ�Ʒ������·�������

int flag=0;

/*
**************************************************
* �������ƣ�NB_IOT_Init
* �������ܣ�NB_IOT�豸��ʼ��
* ��ڲ�����uartx�����ں�
			recv_bufsz�����������ݳ���
* ���ز�����0-�豸��ʼ���ɹ�
			1-�豸��ʼ��ʧ��
* ˵����    
***************************************************
*/
int NB_IOT_Init(IOT_DATA_t *IOT_DATA,const char *uartx,rt_size_t recv_bufsz)
{
	int i=0;
	
	rt_thread_mdelay(2000);
	
	/* AT client ��ʼ��,ʹ��uartx�豸�����������ݳ���Ϊrecv_bufsz*/
	at_client_init(uartx,recv_bufsz);
	
	at_client_port_init();
	
	rt_kprintf("nb_iot init...\n");
	rt_thread_mdelay(5000);
	
	if(flag==0)
	/* ȷ��ģ���������� */
	/*����ʱѭ������ AT ���ֱ��ģ����Ӧ����,��ʱʱ��Ϊ 0.5 ��*/
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
* �������ƣ�NB_IOT_GetCGSN
* �������ܣ���ȡ�豸Ψһ��ʶ��
* ��ڲ�������
* ���ز�����RT_EOK-��ȡ�ɹ�
			RT_ERROR-��ȡʧ��
* ˵����    
***************************************************
*/
int NB_IOT_GetCGSN(IOT_DATA_t *IOT_DATA)
{
	rt_err_t result = RT_EOK;
	const char *CGSN;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ���ȡ�豸Ψһ��ʶ�� */
	rt_thread_mdelay(20);
    if ( at_exec_cmd(resp, "AT+CGSN=1") != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
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
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
}


/*
**************************************************
* �������ƣ�NB_IOT_GetCIMI
* �������ܣ���ȡ�豸SIM��IMSI��
* ��ڲ�������
* ���ز�����RT_EOK-��ȡ�ɹ�
			RT_ERROR-��ȡʧ��
* ˵����    
***************************************************
*/
int NB_IOT_GetCIMI(IOT_DATA_t *IOT_DATA)
{
	rt_err_t result = RT_EOK;
	const char *CIMI;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ���ȡSIM��IMSI�� */
	rt_thread_mdelay(20);
    if (at_exec_cmd(resp, "AT+CIMI")!= RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
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
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
}

/*
**************************************************
* �������ƣ�NB_IOT_GetSignal
* �������ܣ���ȡNB_IOT�豸���ź�ǿ��
* ��ڲ�������
* ���ز�����RT_EOK-���ӳɹ�
			RT_ERROR-����ʧ��
* ˵����    ģ�鷵�� +CSQ:30,99 ,ǰ��31�ź�����
			��Χ��12~31 ������12���ݿ��ܷ���ʧ��
***************************************************
*/
int NB_IOT_GetSignal(IOT_DATA_t *IOT_DATA)
{
	rt_err_t result = RT_EOK;
	const char *signal;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ���ѯע����������� +CEREG:0,1 ��ע���ɹ� */
	rt_thread_mdelay(20);
    if (at_exec_cmd(resp, "AT+CSQ") != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
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
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
}

/*
**************************************************
* �������ƣ�NB_IOT_ConsultNet
* �������ܣ���ѯע�����
* ��ڲ�������
* ���ز�����RT_EOK-ע���ɹ�
			RT_ERROR-��δע��
* ˵����    ģ�鷵�� +CEREG:0,1 ��ʾע���ɹ�
***************************************************
*/
int NB_IOT_ConsultNet(void)
{
	rt_err_t result = RT_ERROR;
//	const char *status;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ���ѯע����������� +CEREG:0,1 ��ע���ɹ� */
	rt_thread_mdelay(20);
    if (at_exec_cmd(resp, "AT+CEREG?") != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
		
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		if(at_resp_get_line_by_kw(resp,"+CEREG:0,1")!=NULL)
			result=RT_EOK;
	}
	
__exit:
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
}

/*
**************************************************
* �������ƣ�NB_IOT_HTTPCREATE
* �������ܣ�
* ��ڲ�����	
* ���ز�����
* ˵����    
***************************************************
*/
int NB_IOT_HTTPCREATE(IOT_DATA_t *args)
{
	rt_err_t result = RT_EOK;
	char tmp[50];
	const char *example;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ�����httpʵ�� */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPCREATE=\"%s\"",args->HTTP_URL);
    if (at_exec_cmd(resp, tmp) != RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		example=at_resp_get_line_by_kw(resp,"+HTTPCREATE:");
		
		args->HTTP_EXAMPLE=*(example+12);
	}
	
__exit:
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* �������ƣ�NB_IOT_HTTPHEADER
* �������ܣ�
* ��ڲ�����	
* ���ز�����
* ˵����    
***************************************************
*/
int NB_IOT_HTTPHEADER(IOT_DATA_t *args)
{
	rt_err_t result = RT_EOK;
	char tmp[AT_CMD_MAX_LEN];
	int i=0;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ���������ͷ */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPHEADER=%c,\"%s\"",args->HTTP_EXAMPLE,args->HTTP_HEADER);
	
	i=at_exec_cmd(resp, tmp);
	if(i!=RT_EOK)
    {
		if(i==-RT_ETIMEOUT)
			result=RT_ETIMEOUT;
		else
			result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{	
		result=RT_EOK;
	}
	
__exit:
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* �������ƣ�NB_IOT_HTTPCONTENT
* �������ܣ�
* ��ڲ�����	
* ���ز�����
* ˵����    
***************************************************
*/
int NB_IOT_HTTPCONTENT(IOT_DATA_t *args)
{
	rt_err_t result = RT_ERROR;
	char tmp[AT_CMD_MAX_LEN];
	int i;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ�����������Ϣ���������� */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPCONTENT=%c,\"%s\"",args->HTTP_EXAMPLE,args->HTTP_CONTENT);
	
	i=at_exec_cmd(resp, tmp);
	if(i!=RT_EOK)
    {
		if(i==-RT_ETIMEOUT)
			result=RT_ETIMEOUT;
		else
			result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		result=RT_EOK;
	}
	
__exit:
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* �������ƣ�NB_IOT_HTTPSEND
* �������ܣ�
* ��ڲ�����	
* ���ز�����
* ˵����    
***************************************************
*/
int NB_IOT_HTTPSEND(IOT_DATA_t *args,int mode)
{
	rt_err_t result = RT_ERROR;
	char tmp[AT_CMD_MAX_LEN];
	int i;
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 64 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(64, 2, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ�����http���� */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPSEND=%c,%d,\"%s\"",args->HTTP_EXAMPLE,mode,args->HTTP_SEND_URL);
	
	i=at_exec_cmd(resp, tmp);
	if(i!=RT_EOK)
    {
		if(i==-RT_ETIMEOUT)
			result=RT_ETIMEOUT;
		else
			result=RT_ERROR;;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		result = RT_EOK;
	}
	
__exit:
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* �������ƣ�NB_IOT_HTTPCLOSE
* �������ܣ�
* ��ڲ�����	
* ���ز�����
* ˵����    
***************************************************
*/
int NB_IOT_HTTPCLOSE(IOT_DATA_t *args)
{
	rt_err_t result = RT_ERROR;
	char tmp[30];
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 32 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(32, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ��ر�HTTPʵ�� */
	rt_thread_mdelay(20);
	rt_sprintf(tmp,"AT+HTTPCLOSE=%c",args->HTTP_EXAMPLE);
	
	if(at_exec_cmd(resp, tmp)!=RT_EOK)
    {
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		result=RT_EOK;
	}
	
__exit:
	/*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
	
	return result;
	
}

/*
**************************************************
* �������ƣ�NB_IOT_Connect
* �������ܣ��豸���ӻ�Ϊ��
* ��ڲ�����IP����Ϊ���ṩ������IP
* ���ز�����0-���ӳɹ�
			1-����ʧ��
* ˵����    
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
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 16 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(16, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
	/* ����ATָ�ȷ���豸�����ӷ����� */
    rt_thread_mdelay(1000);
    if (at_exec_cmd(resp, cmd) != RT_EOK)
    {
        rt_kprintf("ָ���ʧ��!\n");
		result=RT_ERROR;
        goto __exit;		//����ָ��ʧ�ܣ���ת��ɾ����Ӧ�ṹ��
    }else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
		rt_kprintf("���������ӳɹ�!\n");
	else if(at_resp_get_line_by_kw(resp,"ERROR")!=NULL)
	{
		rt_kprintf("����������ʧ��!\n");
		result=RT_ERROR;
	}
	
__exit:
    /*ɾ����Ӧ�ṹ��*/
    at_delete_resp(resp);
    	
	return result;
}

///////////////////////////////////////////
//����ת������
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
* �������ƣ�NB_IOT_SendData
* �������ܣ��豸��Ϊ�Ʒ�������
* ��ڲ�����arr��Ҫ���͵�����
			length���������ݵĳ���
* ���ز�����0-���ݷ��ͳɹ�
			1-���ݷ���ʧ��
* ˵����    
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
	
	
	/*������Ӧ�ṹ����*/
	at_response_t resp=RT_NULL;
	
	/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 512 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
    resp=at_create_resp(512, 0, rt_tick_from_millisecond(5000));
	if (resp == RT_NULL)
    {
		rt_kprintf("No memory for response structure!");		
		return RT_ERROR;
    }
	
    /* ���� AT ������� AT Server ��Ӧ���ݣ����ݼ���Ϣ����� resp �ṹ���� */
	rt_thread_mdelay(1000);
    if (at_exec_cmd(resp, cmd) != RT_EOK)
    {
        rt_kprintf("ָ���ʧ��!\n");
		at_delete_resp(resp);
		return RT_ERROR;
    }else if(at_resp_get_line_by_kw(resp,"ERROR")!=NULL)
	{
		rt_kprintf("���ݷ���ʧ��!\n");
		at_delete_resp(resp);
		return RT_ERROR;
	}else if(at_resp_get_line_by_kw(resp,"OK")!=NULL)
	{
		rt_kprintf("���ݷ��ͳɹ�!\n");		
	}
	
	/* ɾ����Ӧ�ṹ�� */
    at_delete_resp(resp);
	
	return 0;
}
/////////////////////////////////////////////////////////
// URC ����ִ�к���
static void urc_func(const char *data, rt_size_t size)
{
//	rt_kprintf("�յ�����Ϊ��\n");
//	rt_kprintf("%s\n",data);
	if(strcmp(data,"+NNMI:4,AAAA0000"))
	{
		rt_kprintf("�������������ݳɹ�!\n");
	}else
	{
		rt_kprintf("��������������ʧ��!\n");
	}
	
}

static void urc_Inited_func(const char *data, rt_size_t size)
{
    /* iot ģ���ϵ��ʼ����� */
	rt_kprintf("nb_iot connect!\n");
	flag=1;
}

static void urc_connect_func(const char *data, rt_size_t size)
{
    /* ���������ӳɹ���Ϣ */
    rt_kprintf("Server connect success!\n");
	rt_kprintf("%s\n",data);
}

static void urc_discon_func(const char *data, rt_size_t size)
{
    /* �������Ͽ�������Ϣ */
    rt_kprintf("Server disconnect!\n");
	rt_kprintf("%s\n",data);
}

static void urc_httpHeader_func(const char *data, rt_size_t size)
{
    /* ���յ� HTTP ͷ����Ϣ */
	char tmp[142]={0};
	
//	at_client_recv(tmp,118,5000);
//    rt_kprintf("���յ� HTTP ͷ����Ϣ:\n");
	rt_kprintf("%s",data);
//	rt_kprintf("%s",tmp);
	rt_kprintf("\n");
	
	
}

static void urc_httpContent_func(const char *data, rt_size_t size)
{
    /* ���յ� HTTP content ������Ϣ */
	char tmp[30],tmp1[4]={0};
	
	at_client_recv(tmp1,3,5000);
//    rt_kprintf("���յ� HTTP content ������Ϣ:\n");
	rt_kprintf("%s",data);
	rt_kprintf("%s\n",tmp1);
	
	if(tmp1[0]=='o'&&tmp1[1]=='k')
		rt_kprintf("send data sucefull!\n");
	if(*(data+12)=='0')
	{
		/*������Ӧ�ṹ����*/
		at_response_t resp=RT_NULL;
		
		/* ������Ӧ�ṹ�壬�������֧����Ӧ���ݳ���Ϊ 32 �ֽڣ���Ӧ�������������ƣ���ʱʱ��Ϊ 5 �� */
		resp=at_create_resp(64, 2, rt_tick_from_millisecond(500));
		if (resp == RT_NULL)
		{
			rt_kprintf("No memory for response structure!\n");		
		}
		
		/* ����ATָ��ر�HTTPʵ�� */
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
		/*ɾ����Ӧ�ṹ��*/
		at_delete_resp(resp);
		
	}
}

/* URC���ݱ�ǰ׺ ����׺ ��ִ�к��� */
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
    /*  ��Ӷ��� URC ������ URC �б��У������յ�ͬʱƥ�� URC ǰ׺�ͺ�׺�����ݣ�ִ�� URC ����  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
	
    return RT_EOK;
}

