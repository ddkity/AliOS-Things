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
    //取秒时间
    t->tm_sec=(int)(time % 60);
    time /= 60;
    //取分钟时间
    t->tm_min=(int)(time % 60);
    time /= 60;
    //取过去多少个四年，每四年有 1461*24 小时
    Pass4year=((unsigned int)time / (1461L * 24L));
    //计算年份
    t->tm_year=(Pass4year << 2) + 1970;
    //四年中剩下的小时数
    time %= 1461L * 24L;
    //校正闰年影响的年份，计算一年中剩下的小时数
    for (;;)
    {
        //一年的小时数
        hours_per_year = 365 * 24;
        //判断闰年
        if ((t->tm_year & 3) == 0)
        {
        //是闰年，一年则多24小时，即一天
        hours_per_year += 24;
        }
        if (time < hours_per_year)
        {
        break;
        }
        t->tm_year++;
        time -= hours_per_year;
    }
    //小时数
    t->tm_hour=(int)(time % 24);
    //一年中剩下的天数
    time /= 24;
    //假定为闰年
    time++;
    //校正闰年的误差，计算月份，日期
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
    //计算月日
    for (t->tm_mon = 0; Days[t->tm_mon] < time;t->tm_mon++)
    {
        time -= Days[t->tm_mon];
    }

    t->tm_mday = (int)(time);
    return;
}
/***********************************************************************
*   以下为串口相关的处理函数
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
　　{0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
　　{0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

int isleap(int year)
{
　　return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

time_t mktime(struct tm dt)
{
　　time_t result;
　　int i =0;
　　// 以平年时间计算的秒数
　　result = (dt.tm_year - 1970) * 365 * 24 * 3600 +
　　(mon_yday[isleap(dt.tm_year)][dt.tm_mon-1] + dt.tm_mday - 1) * 24 * 3600 +
　　dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
　　// 加上闰年的秒数
　　for(i=1970; i < dt.tm_year; i++)
　　{
　　　　if(isleap(i))
　　　　{
　　　　　　result += 24 * 3600;
　　　　}
　　}
　　return(result);
}

void main()
{
　　time_t time = 0;
　　time_t time2 = 0;
　　long i = 0;
　　struct tm t;
　　//2018-01-01 01:01:01
　　time = 1514768461;
　　// 验证一个周期4年 一天打印一次
　　for(i=0;i<(4*365+1);i++)
　　{
　　　　localtime(time,&t);
　　　　printf("A time:%d\r\n",time);
　　　　printf("A %04d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);

　　　　t.tm_mon+=1;    //转换时月份需要加1，因为月份是从0开始的
　　　　time2 = mktime(t);  //将localtime得到年月日时分秒再次转换成时间戳，验证算法是否正确
　　　　printf("B time:%d\r\n",time2);
　　　　memset((void*)&t,0x00,sizeof(t));
　　　　localtime(time2,&t);
　　　　printf("B %04d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
　　　　memset((void*)&t,0x00,sizeof(t));
　　　　time += 24*3600;
　　}

　　return;
}
#endif

