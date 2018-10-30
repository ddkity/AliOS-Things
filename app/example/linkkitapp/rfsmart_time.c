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


ksem_t g_uart_sem;
void RfsmartInit(void)
{
    /* ��ʼ���ź��� */
    krhino_sem_create(&g_uart_sem, "uart_sem", 0);
}


/***********************************************************************
*   ����Ϊ������صĴ�����
***********************************************************************/


/************����ͨ������궨��***************/
#define UARTNOP     (0)         /* ���ڽ��մ�����߿��� */
#define UARTSOP     (1)         /* ������ʼλ */
#define UARTDEVTYPE (2)         /* �豸���� */
#define UARTRES1    (3)         /* �����ֶ�1 */
#define UARTRES2    (4)         /* �����ֶ�2 */
#define UARTRES3    (5)         /* �����ֶ�3 */
#define UARTRES4    (6)         /* �����ֶ�4 */
#define UARTCMD     (7)         /* ������� */
#define UARTDATALEN (8)         /* ��Ϣ���� */
#define UARTDATA    (9)         /* ��Ϣ�� */
#define UARTCRC     (10)        /* У��� */

//����֡�ṹ����
#define F_HEAD              (0xBB)	    /* ֡ͷ */
#define F_HEAD_RET          (0xCC)      /* WIFI�����豸��֡ͷ */
#define RECVMAXLEN          (128)       /* ���ڽ��յ�buffer */
#define SENDMAXLEN          (128)       /* ���ڽ��յ�buffer */
#define DEVICEMODETYPE      (0x10)      /* �豸���ͣ�������������Ϊ0x10 */
#define RESERVE1            (0x00)      /* �����ֶ�ֵ */
#define RESERVE2            (0x00)      /* �����ֶ�ֵ */
#define RESERVE3            (0x00)      /* �����ֶ�ֵ */
#define RESERVE4            (0x00)      /* �����ֶ�ֵ */

/* ���Դ�������: bb 10 00 00 00 00 01 0d aa bb cc dd 19 */

unsigned char UartStatus = UARTNOP;     /* ����״̬�� */
unsigned char UartRxOkFlag = 0;         /* ������ɱ�־ */
unsigned char RecvBuffer[RECVMAXLEN] = {0}; /* ����buffer */
unsigned char UartRecvLen = 0;          /* ���յ������ݳ��� */
unsigned char UartDataLen = 0;         /* ���յ����ݶεĳ��� */
unsigned char CalCRC = 0;	/* ���������CRC���� */
unsigned char RecvCRC = 0;	/* ���յ���CRC���� */

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
*   ���ڷ������ݽӿ�
*   SENDCMD: ��Ҫ���͵�����
*   SendData:   ��Ҫ���͵���Ϣ��
*   Len: ��Ҫ���͵���Ϣ��ĳ��ȣ�����SendData�ĳ���
*******************************************/
void UartSendFormData(unsigned char SendCMD, const unsigned char *SendData, unsigned char Len)
{
    int i;
    unsigned int SendTimeout = 0;
    unsigned char SendBuffer[SENDMAXLEN] = {0};
    unsigned char SendDataLen = 0;
    unsigned char SendCRC = 0;

    memset(SendBuffer, 0x00, SENDMAXLEN);
    SendTimeout = 300;  /* ���ͳ�ʱ */

    SendBuffer[SendDataLen++] = F_HEAD_RET;         /* ֡ͷ */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = DEVICEMODETYPE;     /* �豸���� */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE1;           /* �����ֶ� */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE2;
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE3;
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = RESERVE4;
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = SendCMD;        /* ������� */
    SendCRC += SendBuffer[SendDataLen - 1];

    SendBuffer[SendDataLen++] = Len;            /* ��Ϣ��ĳ��� */
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


/* ���ڽ������ݴ��������������ݰ���֡ͷ֡β��һ��֡���� */
void ProtocalUartData(unsigned char *Data, unsigned char Length)
{
    int i;
    unsigned char CMD = 0x00;

    /* ���ż�¼������ر��� */
    int EvenRecordID;
    int EvenLockType;
    char* EvenKeyID;
    unsigned char *AlarmTime;
    int EvenBatteryValue;
    /***********************/

    /* ����������ر��� */
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
            EvenRecordID = Data[8];     /* ��¼ID */
            EvenLockType = Data[9];     /* ������ʽ */
            sprintf(EvenKeyID,"%d",Data[10]);                  /* Կ��ID */
            AlarmTime = &Data[11];                  /* ����ʱ�� */
            EvenBatteryValue = Data[17];           /* ��ص��� */

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
            if(Length == 26){   /* ���ݹ������ܴ��� */
                trigger_PushDoorLockInfo_event(&Data[8]);
            }else{
                printf("=====>Error: uart Recv Data maybe error, check it.....\n");
            }
            break;

        case TEMPPAWSS_CMD:
            /* TODO ��Ϊ���յ����豸�˷��ص�Ӧ�𣬿��Բ������� */
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

/* ���ڽ��ճ��� */
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

                /* ��ʼ�����˴������ݣ�����һ�ζ�ʱ����100ms���ճ�ʱ */
                aos_timer_new(&uart1_timer, uart1_timer_func, NULL, 100, 0);
            }else{
                UartStatus = UARTNOP;
            }
            break;
        }

        case UARTDEVTYPE:
        {
            RecvBuffer[UartRecvLen++] = RecvChar;

            if(DEVICEMODETYPE != RecvChar){     /* �ж��豸�����Ƿ������ߣ���������������������� */
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
                /* ҵ������ */
                ProtocalUartData(RecvBuffer, UartRecvLen);
            }else{
                printf("====>Error: Recv Data is illegal, CalCRC:%02x, RecvCRC:%02x\n", CalCRC, RecvCRC);
            }

            /* ��ʼ�����еĴ��ڱ��� */
            aos_timer_stop(&uart1_timer);   /* ֹͣ��ʱ�� */
            aos_timer_free(&uart1_timer);   /* �ͷŶ�ʱ�� */
            InitAllUartValue();
			break;
        }

    }
}

/* ���ڴ����߳� */
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

