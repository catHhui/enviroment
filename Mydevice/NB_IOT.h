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

#ifndef __NB_IOT_H
#define	__NB_IOT_H

#include <rtthread.h>
#include <at.h>

typedef	struct
{
	char NB_IMSI[16];//���NB_IOT�豸SIM����IMSI��
	char NB_CGSN[16];//���NB_IOT�豸Ψһ��ʶ��
	char NB_SIGNAL[2];//���NB_IOT�豸�ź�ǿ��
	rt_int16_t net_sta;//ע������״��
	
	char  HTTP_EXAMPLE;	//http��ʵ��
	char *HTTP_URL;		//http��URL����
	char *HTTP_HEADER;	//http������ͷ
	char *HTTP_CONTENT;	//http��������
	char *HTTP_SEND_URL;//http������·��
	
}IOT_DATA_t;

int NB_IOT(void);
	
int NB_IOT_Init(IOT_DATA_t *IOT_DATA,const char *uartx,rt_size_t recv_bufsz);

int NB_IOT_GetCIMI(IOT_DATA_t *IOT_DATA);		//��ȡ�豸SIM��IMSI��
int NB_IOT_GetCGSN(IOT_DATA_t *IOT_DATA);		//��ȡ�豸Ψһ��ʶ��
int NB_IOT_GetSignal(IOT_DATA_t *IOT_DATA);		//��ȡNB_IOT�豸���ź�ǿ��
int NB_IOT_ConsultNet(void);	//��ѯ�豸ע�����

int NB_IOT_HTTPCREATE(IOT_DATA_t *args);
int NB_IOT_HTTPHEADER(IOT_DATA_t *args);
int NB_IOT_HTTPCONTENT(IOT_DATA_t *args);
int NB_IOT_HTTPSEND(IOT_DATA_t *args,int mode);
int NB_IOT_HTTPCLOSE(IOT_DATA_t *args);

int NB_IOT_Connect(char *IP);	//�豸���ӻ�Ϊ��

int NB_IOT_SendData(char *arr,int length);
int at_client_port_init(void);	// URC���ݴ����ʼ��

#endif

