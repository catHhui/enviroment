/**************************************************
*
*����    ������
*
*����������LED��ʾ����12�ӿڣ�����
*
*version ��V1.0
*��д���ڣ�2019.10.14
***************************************************/

#ifndef __LED_12_H
#define	__LED_12_H

#include <rtthread.h>

#define	P12_A		11	//GPIO_PF1
#define	P12_B		12	//GPIO_PF2
#define	P12_C			
#define	P12_D			
#define	P12_OE1		13	//GPIO_PF3
#define	P12_OE2		14	//GPIO_PF4
#define	P12_OE3		15	//GPIO_PF5
#define	P12_CLK		//GPIO_PA5	��spi1�� SCK ����
#define	P12_HS		//GPIO_PA4	��spi1�� OE ����
#define	P12_R		//GPIO_PA7	��spi1�� MOSI ����
#define	P12_G		//GPIO_P

extern rt_uint8_t LED_DATA[4*3][16*3];
int LED_12_Init(void);
void LED_SelectLine(int line);	//ѡ������
void LED_ShowChar(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,rt_uint8_t ch);
void LED_ShowString(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,char *str);
void LED_ShowChese(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,char *str);
void LED_ShowNChese(rt_uint16_t x,rt_uint16_t y,rt_uint16_t size,rt_uint16_t n,char *str);
void LED_Clear(void);
void LED_SendData(void);
#endif

