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

#include "BYH_M.h"
#include <rtdevice.h>


/* �����豸��� */
static rt_device_t serial;

/*
**************************************************
* �������ƣ�BYH_M_init
* �������ܣ�BYH_M�豸��ʼ������
* ��ڲ�����baud_rate��������
* ���ز�������
* ˵����    
***************************************************
*/
int BYH_M_init(const char *uartx,rt_uint32_t baud_rate)
{
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT; /* ���ò��� */
	
	/* ���Ҵ����豸 */
	serial = rt_device_find(uartx);
    if (!serial)
    {
        rt_kprintf("find %s failed!\n", uartx);
        return RT_ERROR;
    }
	
	/* ���жϽ��ռ���ѯ����ģʽ�򿪴����豸 */
	rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);
	
	/* ���豸��ſ��޸Ĵ������ò��� */
	config.baud_rate = baud_rate;
	rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
	
	return 0;
}

/*
**************************************************
* �������ƣ�CRC16_ModbusRTU
* �������ܣ��������ݵ�У��ֵ
* ��ڲ�����puchMsg:У�����ݵĵ�ַ
			usDataLen:У�����ݵĳ���
* ���ز�����wCRCin:У��ֵ
* ˵����    
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
* �������ƣ�BYH_M_get_humiture
* �������ܣ�
* ��ڲ�����BYH_M: �����������ݽṹ��ĵ�ַ			
* ���ز�����
* ˵����    
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
	The_CMD[3]+=0;	//�Ĵ�����ַ��0
	The_CMD[5]+=2;	//��ȡ�����Ĵ�����ֵ
					//����ʪ��
	
	/*�õ��������ݵ�crcУ��ֵ*/
	crc=CRC16_ModbusRTU(The_CMD,6);
	The_CMD[6]=(rt_uint8_t)(crc>>8);
	The_CMD[7]=(rt_uint8_t)crc;
	
	rt_device_write(serial, 0, The_CMD, 8);
	
	/*��ȡ���յ�������*/
	rt_thread_mdelay(100);
	rt_device_read(serial,0,temp,9);
	
	/*У����յ����ݣ��������ͷ���*/
	crc=temp[7];
	crc=(crc<<8)|temp[8];
	if(crc!=CRC16_ModbusRTU(temp,7))
		return RT_ERROR;
	
//	rt_device_write(serial, 0, temp,9);
	
	//����ʪ��ֵ
	tmp=temp[3];
	tmp=(tmp<<8)|temp[4];
	BYH_M->Humidity_x10=tmp;
	//�����¶�ֵ
	tmp=temp[5];
	tmp=(tmp<<8)|temp[6];
	BYH_M->Temperature_x10=tmp;
	
	return RT_EOK;
}

/*
**************************************************
* �������ƣ�BYH_M_get_noise
* �������ܣ�
* ��ڲ�����BYH_M: �����������ݽṹ��ĵ�ַ	
* ���ز�����
* ˵����    
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
	
	The_CMD[3]+=2;	//�Ĵ�����ַ��2
	The_CMD[5]+=1;	//��ȡһ�����Ĵ�����ֵ
					//������
	
	/*�õ��������ݵ�crcУ��ֵ*/
	crc=CRC16_ModbusRTU(The_CMD,6);
	The_CMD[6]=(rt_uint8_t)(crc>>8);
	The_CMD[7]=(rt_uint8_t)crc;
	
	rt_device_write(serial, 0, The_CMD, 8);
	
	/*��ȡ���յ�������*/
	rt_thread_mdelay(100);
	rt_device_read(serial,0,temp,7);
	
	/*У����յ����ݣ��������ͷ���*/
	crc=temp[5];
	crc=(crc<<8)|temp[6];
	if(crc!=CRC16_ModbusRTU(temp,5))
		return RT_ERROR;
	else
	{
		//��������ֵ
		tmp=temp[3];
		tmp=(tmp<<8)|temp[4];
		BYH_M->Noise_x10=tmp;
	}
	return RT_EOK;
}

/*
**************************************************
* �������ƣ�BYH_M_get_PM_Data
* �������ܣ�
* ��ڲ�����BYH_M: �����������ݽṹ��ĵ�ַ		
* ���ز�����
* ˵����    
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
	The_CMD[3]+=3;	//�Ĵ�����ַ��0
	The_CMD[5]+=2;	//��ȡ�����Ĵ�����ֵ
					//��PM2.5��PM10��
	
	/*�õ��������ݵ�crcУ��ֵ*/
	crc=CRC16_ModbusRTU(The_CMD,6);
	The_CMD[6]=(rt_uint8_t)(crc>>8);
	The_CMD[7]=(rt_uint8_t)crc;
	
	rt_device_write(serial, 0, The_CMD, 8);
	
	/*��ȡ���յ�������*/
	rt_thread_mdelay(100);
	rt_device_read(serial,0,temp,9);
	
	/*У����յ����ݣ��������ͷ���*/
	crc=temp[7];
	crc=(crc<<8)|temp[8];
	if(crc!=CRC16_ModbusRTU(temp,7))
		return RT_ERROR;
	
//	rt_device_write(serial, 0, temp,9);
	
	//����PM2.5ֵ
	tmp=temp[3];
	tmp=(tmp<<8)|temp[4];
	BYH_M->PM2_5=tmp;
	//����PM10ֵ
	tmp=temp[5];
	tmp=(tmp<<8)|temp[6];
	BYH_M->PM10=tmp;
	
	return RT_EOK;
}

