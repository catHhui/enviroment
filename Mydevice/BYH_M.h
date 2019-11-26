/**************************************************
*
*����    ������
*
*���������������Ҫ�ذ�Ҷ��BYH-M���ഫ����������
*
*version ��V1.0
*��д���ڣ�2019.10.14
*
*version ��V1.1
*��д���ڣ�2019.10.28
*�汾˵����	1������BYH_M���������޸Ŀ��ƽṹ�壬ʹ
		��ʹ�ø��ӷ��㣻
			2��ͬʱ�޸���غ��������������
***************************************************/

#ifndef __BYH_M_H
#define	__BYH_M_H

#include <rtthread.h>


/*BYH_M���������ݿ�*/
typedef struct 
{
	rt_uint8_t BYH_M_CMD[6];	//��� BYH_M ��������
	
	rt_int16_t Temperature_x10;	//����¶�ֵ x10
	rt_int16_t Humidity_x10;	//���ʪ��ֵ x10
	rt_int16_t Noise_x10;		//�������ֵ x10
	
	rt_int16_t PM2_5;			//���PM2.5ֵ
	rt_int16_t PM10;			//���PM10ֵ
	
}BYH_M_t;

int BYH_M_init(const char *uartx,rt_uint32_t baud_rate);//BYH_M�豸��ʼ������
	
rt_uint16_t CRC16_ModbusRTU(rt_uint8_t *puchMsg, rt_uint32_t usDataLen);//�������ݵ�У��ֵ

rt_err_t BYH_M_get_humiture(BYH_M_t *BYH_M); //��ȡ��ʪ������
rt_err_t BYH_M_get_noise(BYH_M_t *BYH_M);	//��ȡ����ֵ
rt_err_t BYH_M_get_PM_Data(BYH_M_t *BYH_M);	//��ȡPM2.5��PM10��ֵ
	
#endif

