/**************************************************
*
*����    �����͡�Ф��������̳�
*
*�������������������Ŀ����
*
*version ��V1.0
*��д���ڣ�2019.11.2
***************************************************/

#include "BYH_M.h"
#include "NB_IOT.h"
#include "LED_12.h"
#include <stdio.h>

/* ָ���ź�����ָ�� */
static rt_sem_t sem_one = RT_NULL;
static rt_sem_t sem_two = RT_NULL;

/* ������ƿ� */
static struct rt_mailbox iot_mb;
static struct rt_mailbox led_mb;
/* ���ڷ��ʼ����ڴ�� */
static char iot_mb_pool[20];
static char led_mb_pool[20];


/*
**************************************************
* �������ƣ�BYH_M_thread_entry
* �������ܣ�BYH_M�߳���ں���
* ��ڲ�����parameter
* ���ز�������
* ˵����    
***************************************************
*/
static void BYH_M_thread_entry(void *parameter)
{
	int i=0,times=0;
	
	/* ���÷�ʽ�ȴ��ź�������ȡ���ź��� */
    rt_sem_take(sem_one, RT_WAITING_FOREVER);
	
	//��ַ�룺0x0b�������룺0x03����ʼ��ַ��0x01f4����ȡ�Ĵ�����ʼ���ȣ�0x0000
	BYH_M_t BYH_DATA={{0x0b,0x03,0x01,0xf4,0x00,0x00},0,0,0,0,0};
	
	BYH_M_init("uart3",BAUD_RATE_4800);
	
	BYH_M_get_humiture(&BYH_DATA);
	rt_thread_mdelay(1000);
	
	BYH_M_get_noise(&BYH_DATA);
	rt_thread_mdelay(1000);
	
	BYH_M_get_PM_Data(&BYH_DATA);
	rt_thread_mdelay(1000);
	
	rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.Temperature_x10);
	rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.Humidity_x10);
	rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.Noise_x10);
	rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.PM2_5);
	rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.PM10);
	rt_sem_release(sem_two);
	while(1)
	{
		/*��ʱ*/
		for(i=0;i<60;i++)
		{
			rt_thread_mdelay(1000);
		}
		times++;
		
		if(times==1)
		{
			times=0;
			
			if(BYH_M_get_humiture(&BYH_DATA)==RT_ERROR)
				rt_kprintf("[BYH_M]Get humiture Error!\n");
			else
				rt_kprintf("�¶ȣ�%d C,ʪ�ȣ�%d %\n",BYH_DATA.Temperature_x10,BYH_DATA.Humidity_x10);
			rt_thread_mdelay(1000);
			
			if(BYH_M_get_noise(&BYH_DATA)==RT_ERROR)
				rt_kprintf("[BYH_M]Get noise Error!\n");
			else
				rt_kprintf("������%d db\n",BYH_DATA.Noise_x10);
			rt_thread_mdelay(1000);
			
			if(BYH_M_get_PM_Data(&BYH_DATA)==RT_ERROR)
				rt_kprintf("[BYH_M]Get PM Data Error!\n");
			else
				rt_kprintf("PM2.5��%d ug/m3,PM10��%d ug/m3\n",BYH_DATA.PM2_5,BYH_DATA.PM10);
			
			
			/* �������ݵ������� */
			rt_mb_send(&iot_mb, (rt_uint32_t)BYH_DATA.Temperature_x10);
			rt_mb_send(&iot_mb, (rt_uint32_t)BYH_DATA.Humidity_x10);
			rt_mb_send(&iot_mb, (rt_uint32_t)BYH_DATA.Noise_x10);
			rt_mb_send(&iot_mb, (rt_uint32_t)BYH_DATA.PM2_5);
			rt_mb_send(&iot_mb, (rt_uint32_t)BYH_DATA.PM10);
			
			rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.Temperature_x10);
			rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.Humidity_x10);
			rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.Noise_x10);
			rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.PM2_5);
			rt_mb_send(&led_mb, (rt_uint32_t)BYH_DATA.PM10);
			
			/*�ͷ�һ���ź��������ݲɼ����*/
			rt_sem_release(sem_one);
			rt_sem_release(sem_two);
		}
		
	}
	
}

/*
**************************************************
* �������ƣ�NB_IOT_thread_entry
* �������ܣ�NB_IOT�߳���ں���
* ��ڲ�����parameter
* ���ز�������
* ˵����    
***************************************************
*/
static void NB_IOT_thread_entry(void *parameter)
{
	rt_uint32_t i=1;
	int t,h,n,p2,p10;
	char tmp[100]={0};
	
	IOT_DATA_t IOT_Data={.HTTP_URL="www.ycdemo.cn",
	.HTTP_HEADER="Content-Type: application/x-www-form-urlencoded; charset=UTF-8\\r\\n",
	//.HTTP_CONTENT="a=1&b=2",
	//.HTTP_CONTENT="user=xiaochunming&pawd=qwertyuiop&t= 25.6 &h=15&n=45&p2=32&p10=45",
	//.HTTP_SEND_URL="www.ycdemo.cn/returndata",
	.HTTP_SEND_URL="www.ycdemo.cn/api/postdata"};
	
						
	NB_IOT_Init(&IOT_Data,"uart2",512);
	
	/*�ͷ�һ���ź�����ʹBYH_M ��ɳ�ʼ�����Ҳɼ�����*/
	rt_sem_release(sem_one);
							
	while(1)
	{
		/*�ȴ�BYH_M ���һ�����ݲɼ�*/
		rt_sem_take(sem_one, RT_WAITING_FOREVER);
		
		rt_kprintf("\n\n�������� %d �Ρ�\n\n",i);
		 
		/* ����������ȡ�ʼ� */
		rt_mb_recv(&iot_mb, (rt_uint32_t *)&t, RT_WAITING_FOREVER);
		rt_mb_recv(&iot_mb, (rt_uint32_t *)&h, RT_WAITING_FOREVER);
		rt_mb_recv(&iot_mb, (rt_uint32_t *)&n, RT_WAITING_FOREVER);
		rt_mb_recv(&iot_mb, (rt_uint32_t *)&p2, RT_WAITING_FOREVER);
		rt_mb_recv(&iot_mb, (rt_uint32_t *)&p10, RT_WAITING_FOREVER);
		
		rt_sprintf(tmp,"user=xiaochunming&pawd=qwertyuiop&t=%d&h=%d&n=%d&p2=%d&p10=%d",t,h,n,p2,p10);
		IOT_Data.HTTP_CONTENT=tmp;
		
		NB_IOT_HTTPCREATE(&IOT_Data);
		
		NB_IOT_HTTPCONTENT(&IOT_Data);
		NB_IOT_HTTPHEADER(&IOT_Data);
			
		NB_IOT_HTTPSEND(&IOT_Data,1);
		
		i++;
	}
	
}
/*
**************************************************
* �������ƣ�
* �������ܣ�
* ��ڲ�����parameter
* ���ز�������
* ˵����    
***************************************************
*/

static void LED_thread_entry(void *parameter)
{
	int i=0,j=0;
	int t,h,n,p2,p10;
	char t1[12]=":--.- C",
		 h1[12]=":--.-",
		 z1[12]=":--.-db",
		 pm2_5[12]=" 2.5:--ug/m3",
		 pm10[12]= "  10:--ug/m3";
	
	LED_12_Init();
	
	LED_ShowNChese(3,0,12,4,"�ﳾ���");
//	LED_ShowString(0,3,12,"PM");
//	LED_ShowChese(2,3,12,"ֵ");
//	LED_ShowString(0,6,12," 2.5:--ug/m3");
//	LED_ShowString(0,9,12," 10 :--ug/m3");
	LED_ShowNChese(0,3,12,2,"�¶�");
	LED_ShowString(4,3,12,": --.- C");
	LED_ShowNChese(0,6,12,2,"ʪ��");
	LED_ShowString(4,6,12,":--.-%RH");
	LED_ShowNChese(0,9,12,2,"����");
	LED_ShowString(4,9,12,":--.-db");
	while(1)
	{
		if(rt_sem_trytake(sem_two)==RT_EOK)
		{
			/* ����������ȡ�ʼ� */
			if(rt_mb_recv(&led_mb, (rt_uint32_t *)&t,  500)==RT_EOK)
			{
				sprintf(t1,":%0.1f C",t/10.0);
				rt_kprintf("%s\n",t1);
			}
			if(rt_mb_recv(&led_mb, (rt_uint32_t *)&h,  500)==RT_EOK)
			{
				sprintf(h1,":%0.1f",h/10.0);
				rt_kprintf("%s\n",h1);
			}
			if(rt_mb_recv(&led_mb, (rt_uint32_t *)&n,  500)==RT_EOK)
			{
				sprintf(z1,":%0.1fdb",n/10.0);
				rt_kprintf("%s\n",z1);
			}
			if(rt_mb_recv(&led_mb, (rt_uint32_t *)&p2,  500)==RT_EOK)
			{
				sprintf(pm2_5," 2.5:%dug/m3",p2);
				rt_kprintf("%s\n",pm2_5);
			}
			if(rt_mb_recv(&led_mb, (rt_uint32_t *)&p10,  500)==RT_EOK)
			{
				sprintf(pm10, "  10:%dug/m3",p10);
				rt_kprintf("%s\n",pm10);
			}
		}
		if(i==6000)
		{
			i=0;
		}
		if((i%500)==0)
		{
			LED_Clear();
			LED_ShowNChese(3,0,12,4,"�ﳾ���");
			LED_ShowNChese(0,3,12,2,"�¶�");
			LED_ShowString(4,3,12,t1);
			LED_ShowNChese(0,6,12,2,"ʪ��");
			LED_ShowString(4,6,12,h1);
			LED_ShowString(9,6,12,"%RH");
			LED_ShowNChese(0,9,12,2,"����");
			LED_ShowString(4,9,12,z1);
//			rt_kprintf("%s\n",t1);
//			rt_kprintf("%s\n",h1);
//			rt_kprintf("%s\n",z1);
		}
		if((i%1000)==0)
		{
			LED_Clear();
			LED_ShowNChese(3,0,12,4,"�ﳾ���");
			LED_ShowString(0,3,12,"PM");
			LED_ShowChese(2,3,12,"ֵ");
			LED_ShowString(0,6,12,pm2_5);
			LED_ShowString(0,9,12,pm10);
//			rt_kprintf("%s\n",pm2_5);
//			rt_kprintf("%s\n",pm10);
		}
		LED_SendData();
		rt_thread_mdelay(10);
		i++;
	}
	
}

int Enviroment(void)
{
	rt_err_t ret = RT_EOK;
	
	 /* ����һ����̬�ź�������ʼֵ�� 0 */
    sem_one = rt_sem_create("evn_sem", 0, RT_IPC_FLAG_FIFO);
    if (sem_one == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = 0.\n");
    }
	sem_two = rt_sem_create("LED_sem", 0, RT_IPC_FLAG_FIFO);
    if (sem_one == RT_NULL)
    {
        rt_kprintf("create LED_dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. LED_dynamic semaphore value = 0.\n");
    }
	
	/* ��ʼ��һ�� mailbox */
    ret = rt_mb_init(&iot_mb,
                        "mbt",                      /* ������ mbt */
                        &iot_mb_pool[0],                /* �����õ����ڴ���� mb_pool */
                        sizeof(iot_mb_pool) / 4,        /* �����е��ʼ���Ŀ����Ϊһ���ʼ�ռ 4 �ֽ� */
                        RT_IPC_FLAG_FIFO);          /* ���� FIFO ��ʽ�����̵߳ȴ� */
    if (ret != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
	ret = rt_mb_init(&led_mb,
                        "mb",                      /* ������ mb */
                        &led_mb_pool[0],                /* �����õ����ڴ���� mb_pool */
                        sizeof(led_mb_pool) / 4,        /* �����е��ʼ���Ŀ����Ϊһ���ʼ�ռ 4 �ֽ� */
                        RT_IPC_FLAG_FIFO);          /* ���� FIFO ��ʽ�����̵߳ȴ� */
    if (ret != RT_EOK)
    {
        rt_kprintf("init mailbox failed.\n");
        return -1;
    }
	/*�����߳�*/
	rt_thread_t iot_thread=rt_thread_create("NB_IOT",NB_IOT_thread_entry,RT_NULL,1024,24,10);
	if(iot_thread !=RT_NULL)
	{
		rt_thread_startup(iot_thread);
	}
	else
	{
		ret = RT_ERROR;
	}
	
	/*�����߳�*/
	rt_thread_t byh_thread=rt_thread_create("BYH_M",BYH_M_thread_entry,RT_NULL,1024,24,10);
	if(byh_thread !=RT_NULL)
	{
		rt_thread_startup(byh_thread);
	}
	else
	{
		ret = RT_ERROR;
	}
	/*�����߳�*/
	rt_thread_t led_thread=rt_thread_create("LED",LED_thread_entry,RT_NULL,1024,25,10);
	if(led_thread !=RT_NULL)
	{
		rt_thread_startup(led_thread);
	}
	else
	{
		ret = RT_ERROR;
	}
	
	return ret;
}
/*������ msh �����б���*/
MSH_CMD_EXPORT(Enviroment, Enviroment Thread);
INIT_COMPONENT_EXPORT(Enviroment);

