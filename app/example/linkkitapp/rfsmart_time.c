#include <aos/aos.h>
#include <k_api.h>
#include <aos/kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


ksem_t g_uart_sem;
void RfsmartInit(void)
{
    /* 初始化信号量 */
    krhino_sem_create(&g_uart_sem, "uart_sem", 0);
}


/***********************************************************************
*   以下为串口相关的处理函数
***********************************************************************/


/************串口通信命令宏定义***************/
#define UARTNOP     (0)         /* 串口接收错误或者空闲 */
#define UARTSOP     (1)         /* 接收起始位 */
#define UARTDEVTYPE (2)         /* 设备类型 */
#define UARTRES1    (3)         /* 保留字段1 */
#define UARTRES2    (4)         /* 保留字段2 */
#define UARTRES3    (5)         /* 保留字段3 */
#define UARTRES4    (6)         /* 保留字段4 */
#define UARTCMD     (7)         /* 命令代码 */
#define UARTDATALEN (8)         /* 消息长度 */
#define UARTDATA    (9)         /* 消息体 */
#define UARTCRC     (10)        /* 校验和 */

//串口帧结构定义
#define F_HEAD              (0xBB)	    /* 帧头 */
#define F_HEAD_RET          (0xCC)      /* WIFI返回设备的帧头 */
#define RECVMAXLEN          (128)       /* 串口接收的buffer */
#define SENDMAXLEN          (128)       /* 串口接收的buffer */
#define DEVICEMODETYPE      (0x10)      /* 设备类型，智能锁的类型为0x10 */
#define RESERVE1            (0x00)      /* 保留字段值 */
#define RESERVE2            (0x00)      /* 保留字段值 */
#define RESERVE3            (0x00)      /* 保留字段值 */
#define RESERVE4            (0x00)      /* 保留字段值 */

/* 测试串口数据: bb 10 00 00 00 00 01 0d aa bb cc dd 19 */

unsigned char UartStatus = UARTNOP;     /* 接收状态机 */
unsigned char UartRxOkFlag = 0;         /* 接收完成标志 */
unsigned char RecvBuffer[RECVMAXLEN] = {0}; /* 接收buffer */
unsigned char UartRecvLen = 0;          /* 接收到的数据长度 */
unsigned char UartDataLen = 0;         /* 接收的数据段的长度 */
unsigned char CalCRC = 0;	/* 计算出来的CRC数据 */
unsigned char RecvCRC = 0;	/* 接收到的CRC数据 */

extern uart_dev_t uart_1;

void InitAllUartValue(void)
{
    UartStatus = UARTNOP;
    UartRxOkFlag = 0;
    memset(RecvBuffer, 0x00, RECVMAXLEN);
    UartRecvLen = 0;
    UartDataLen = 0;
    CalCRC = 0;
    RecvCRC = 0;
}

/*******************************************
*   串口发送数据接口
*   SENDCMD: 需要发送的命令
*   SendData:   需要发送的消息体
*   Len: 需要发送的消息体的长度，即是SendData的长度
*******************************************/
void UartSendFormData(unsigned char SendCMD, const unsigned char *SendData, unsigned char Len)
{
    int i;
    unsigned int SendTimeout = 0;
    unsigned char SendBuffer[SENDMAXLEN] = {0};
    unsigned char SendDataLen = 0;
    unsigned char SendCRC = 0;

    memset(SendBuffer, 0x00, SENDMAXLEN);
    SendTimeout = 300;  /* 发送超时 */

    SendBuffer[SendDataLen++] = F_HEAD_RET;         /* 帧头 */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = DEVICEMODETYPE;     /* 设备类型 */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE1;           /* 保留字段 */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE2;
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE3;
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE4;
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = SendCMD;        /* 命令代码 */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = Len;            /* 消息体的长度 */
    SendCRC += SendBuffer[SendDataLen - 1];

    for(i = 0; i < Len; i++){
        SendBuffer[SendDataLen++] = SendData[i];
        SendCRC += SendBuffer[SendDataLen - 1];
    }

    SendCRC = 0 - SendCRC;

    SendBuffer[SendDataLen++] = SendCRC;


    printf("SendData:");
    for(i = 0; i < SendDataLen; i++){
        printf("%02x ", SendBuffer[i]);
    }
    printf("\n");

    hal_uart_send(&uart_1, SendBuffer, SendDataLen, SendTimeout);

}


/* 串口接收数据处理，传进来的数据包括帧头帧尾的一整帧数据 */
void ProtocalUartData(unsigned char *Data, unsigned char Length)
{
    int i;
    unsigned char CMD = 0x00;

    /* 开门记录变量相关变量 */
    int EvenRecordID;
    int EvenLockType;
    char* EvenKeyID;
    unsigned char *AlarmTime;
    int EvenBatteryValue;
    /***********************/

    /* 报警推送相关变量 */
    int AlarmType;
    int HijackKeyID;
    //unsigned char *AlarmTime;
    //int EvenBatteryValue;
    /***********************/

    unsigned char cmd;
    unsigned char senddata;
    unsigned char sendlen;

    printf("RecvData:");
    for(i = 0; i < Length; i++){
        printf("%02x ", Data[i]);
    }
    printf("\n");

    CMD = Data[6];

    switch(CMD)
    {
        case INITYUNDATA_CMD:
            InitAllYunProp();
            break;

        case SETWIFINETWORK_CMD:
            if(Data[8] == 0x01){
                WIFISetNetwork();
            }else{
                cmd = SETWIFINETWORK_CMD_RET;
                sendlen = 0x01;
                senddata = 0x01;
                UartSendFormData(cmd, &senddata, sendlen);
                printf("====>Error: uart recvdata is invalable.\n");
            }
            break;

        case CLEARWIFICONFIG_CMD:
            ClearWifiAPConfig();
            break;

        case SYNCCURTIME_CMD:
            GetCurTime();
            break;

        case CHECKWIFISTATUS_CMD:
            GetWifiStatus();
            break;

        case DOOROPENRECORD_CMD:
            EvenRecordID = Data[8];     /* 记录ID */
            EvenLockType = Data[9];     /* 开锁方式 */
            sprintf(EvenKeyID,"%d",Data[10]);                  /* 钥匙ID */
            AlarmTime = &Data[11];                  /* 开锁时间 */
            EvenBatteryValue = Data[17];           /* 电池电量 */

            trigger_DoorOpenNotific_event(EvenRecordID, EvenLockType, EvenKeyID, AlarmTime, EvenBatteryValue);
            break;

        case PUSHALARM_CMD:
            AlarmType = Data[8];
            HijackKeyID = Data[9];
            AlarmTime = &Data[10];
            EvenBatteryValue = Data[16];
            trigger_PushAlarm_event(AlarmType, HijackKeyID, AlarmTime, EvenBatteryValue);
            break;

        case DOORLOCKINFO_CMD:
            if(Length == 26){   /* 数据够长才能处理 */
                trigger_PushDoorLockInfo_event(&Data[8]);
            }else{
                printf("=====>Error: uart Recv Data maybe error, check it.....\n");
            }
            break;

        case TEMPPAWSS_CMD:
            /* TODO 作为接收到的设备端返回的应答，可以不做处理 */
            break;

        default:
            printf("====>Error: unkonw CMD\n");
            break;
    }
}

aos_timer_t uart1_timer;
void uart1_timer_func(void *arg1, void *arg2)
{
    InitAllUartValue();
    printf("====>time is running ......\n");
}

/* 串口接收程序 */
void UartRecvDataHandler(unsigned char RecvChar)
{
    switch(UartStatus)
    {
        case UARTNOP:
        {
            if(UartRxOkFlag){
				break;
			}else{
				UartStatus = UARTSOP;
			}
        }

        case UARTSOP:
        {
            if(RecvChar == F_HEAD){
                RecvBuffer[UartRecvLen++] = RecvChar;
                CalCRC += RecvChar;
				UartStatus = UARTDEVTYPE;

                /* 开始接收了串口数据，启动一次定时器，100ms接收超时 */
                aos_timer_new(&uart1_timer, uart1_timer_func, NULL, 100, 0);
            }else{
                UartStatus = UARTNOP;
            }
            break;
        }

        case UARTDEVTYPE:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;

            if(DEVICEMODETYPE != RecvChar){     /* 判断设备类型是否是锁具，如果不是智能锁不做处理 */
                InitAllUartValue();
                break;
            }
            CalCRC += RecvChar;
			UartStatus = UARTRES1;
            break;
        }

        case UARTRES1:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC += RecvChar;
			UartStatus = UARTRES2;
            break;
        }

        case UARTRES2:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC += RecvChar;
			UartStatus = UARTRES3;
            break;
        }

        case UARTRES3:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC += RecvChar;
			UartStatus = UARTRES4;
            break;
        }

        case UARTRES4:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC += RecvChar;
			UartStatus = UARTCMD;
            break;
        }

        case UARTCMD:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC += RecvChar;
			UartStatus = UARTDATALEN;
            break;
        }

        case UARTDATALEN:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC += RecvChar;
            UartDataLen = RecvChar;
			UartStatus = UARTDATA;
            break;
        }

        case UARTDATA:
		{
			if(UartDataLen > 0)
			{
				RecvBuffer[UartRecvLen++] = RecvChar;
                CalCRC += RecvChar;
				UartDataLen--;
				break;
			}
			else{
				UartStatus = UARTCRC;
			}
		}

        case UARTCRC:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;
            CalCRC = 0 - CalCRC;
            RecvCRC = RecvChar;

            if(CalCRC == RecvCRC){
                UartRxOkFlag = 0x01;
                /* 业务处理函数 */
                ProtocalUartData(RecvBuffer, UartRecvLen);
            }else{
                printf("====>Error: Recv Data is illegal, CalCRC:%02x, RecvCRC:%02x\n", CalCRC, RecvCRC);
            }

            /* 初始化所有的串口变量 */
            aos_timer_stop(&uart1_timer);   /* 停止定时器 */
            aos_timer_free(&uart1_timer);   /* 释放定时器 */
            InitAllUartValue();
			break;
        }

    }
}

/* 串口处理线程 */
void uarthandler_func(void)
{
    int ret;

    unsigned char RecvCharTemp = 0x00;
    unsigned int expect_size = 1;
    unsigned int recv_size = 0;
    unsigned int timeout = 1;

    while(1)
    {
        //krhino_task_sleep(1000);
        //hal_uart_send(&uart_1, &SendData, recv_size, timeout);
        ret = krhino_sem_take(&g_uart_sem, RHINO_WAIT_FOREVER);
        if(ret != RHINO_SUCCESS){
            printf("=====>take the sem failed.\n\n");
        }

        ret = hal_uart_recv_II(&uart_1, &RecvCharTemp, expect_size, &recv_size, timeout);
        if(ret == 0){
            UartRecvDataHandler(RecvCharTemp);
            RecvCharTemp = 0x00;
        }else{
            printf("=====>WARING:   recv uart data maybe error.\n\n");
        }
    }
}













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

