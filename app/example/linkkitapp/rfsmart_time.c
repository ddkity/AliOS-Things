#include <aos/aos.h>
#include <k_api.h>
#include <aos/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include "hal/soc/soc.h"

#include"rfsmart_time.h"
#include "uart.h"

const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void rflocaltime(unsigned int time,struct rftm *t)
{
    unsigned int Pass4year;
    int hours_per_year;
    if(time < 0)
    {
        time = 0;
    }
    //ȡ��ʱ��
    t->tm_sec=(int)(time % 60);
    time /= 60;
    //ȡ����ʱ��
    t->tm_min=(int)(time % 60);
    time /= 60;
    //ȡ��ȥ���ٸ����꣬ÿ������ 1461*24 Сʱ
    Pass4year=((unsigned int)time / (1461L * 24L));
    //�������
    t->tm_year=(Pass4year << 2) + 1970;
    //������ʣ�µ�Сʱ��
    time %= 1461L * 24L;
    //У������Ӱ�����ݣ�����һ����ʣ�µ�Сʱ��
    for (;;)
    {
        //һ���Сʱ��
        hours_per_year = 365 * 24;
        //�ж�����
        if ((t->tm_year & 3) == 0)
        {
        //�����꣬һ�����24Сʱ����һ��
        hours_per_year += 24;
        }
        if (time < hours_per_year)
        {
        break;
        }
        t->tm_year++;
        time -= hours_per_year;
    }
    //Сʱ��
    t->tm_hour=(int)(time % 24);
    //һ����ʣ�µ�����
    time /= 24;
    //�ٶ�Ϊ����
    time++;
    //У��������������·ݣ�����
    if((t->tm_year & 3) == 0)
    {
        if (time > 60)
        {
            time--;
        }
        else
        {
            if (time == 60)
            {
                t->tm_mon = 1;
                t->tm_mday = 29;
                return ;
            }
        }
    }
    //��������
    for (t->tm_mon = 0; Days[t->tm_mon] < time;t->tm_mon++)
    {
        time -= Days[t->tm_mon];
    }

    t->tm_mday = (int)(time);
    return;
}
/***********************************************************************
*   ����Ϊ������صĴ�����
***********************************************************************/
/*
extern uart_dev_t uart_1;
void uarthandler_func(void)
{
    int ret = -1;

    unsigned char RecvData[64];
    unsigned char SendData[64];
    unsigned int expect_size = 1;
    unsigned int recv_size = 0;
    unsigned int timeout = 10;

    while(1){
        memset(RecvData, 0x00, 64);
        memset(SendData, 0x00, 64);
        recv_size = 0;
        ret = -1;

        ret = hal_uart_recv_II(&uart_1, RecvData, expect_size, &recv_size, timeout);
        if(ret == 0){
            memcpy(SendData, RecvData, recv_size);
            hal_uart_send(&uart_1, SendData, recv_size, timeout);
        }
    }
}
*/

#if 0
static time_t mon_yday[2][12] =
{
����{0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
����{0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

int isleap(int year)
{
����return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

time_t mktime(struct tm dt)
{
����time_t result;
����int i =0;
����// ��ƽ��ʱ����������
����result = (dt.tm_year - 1970) * 365 * 24 * 3600 +
����(mon_yday[isleap(dt.tm_year)][dt.tm_mon-1] + dt.tm_mday - 1) * 24 * 3600 +
����dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
����// �������������
����for(i=1970; i < dt.tm_year; i++)
����{
��������if(isleap(i))
��������{
������������result += 24 * 3600;
��������}
����}
����return(result);
}

void main()
{
����time_t time = 0;
����time_t time2 = 0;
����long i = 0;
����struct tm t;
����//2018-01-01 01:01:01
����time = 1514768461;
����// ��֤һ������4�� һ���ӡһ��
����for(i=0;i<(4*365+1);i++)
����{
��������localtime(time,&t);
��������printf("A time:%d\r\n",time);
��������printf("A %04d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);

��������t.tm_mon+=1;    //ת��ʱ�·���Ҫ��1����Ϊ�·��Ǵ�0��ʼ��
��������time2 = mktime(t);  //��localtime�õ�������ʱ�����ٴ�ת����ʱ�������֤�㷨�Ƿ���ȷ
��������printf("B time:%d\r\n",time2);
��������memset((void*)&t,0x00,sizeof(t));
��������localtime(time2,&t);
��������printf("B %04d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
��������memset((void*)&t,0x00,sizeof(t));
��������time += 24*3600;
����}

����return;
}
#endif

