#ifndef __RFSMART_TIME_H__
#define __RFSMART_TIME_H__

/* Э������� */
#define INITYUNDATA_CMD             (0x01)      /* ��ʼ���ƶ����� */
#define SETWIFINETWORK_CMD          (0x02)      /* WIFI���� */
#define SETWIFINETWORK_CMD_RET      (0x82)
#define CLEARWIFICONFIG_CMD         (0x03)      /* ������� */
#define CLEARWIFICONFIG_CMD_RET     (0x83)
#define SYNCCURTIME_CMD             (0x04)      /* ͬ��ʱ�� */
#define SYNCCURTIME_CMD_RET         (0x84)
#define CHECKWIFISTATUS_CMD         (0x05)      /* ��ѯwifi״̬ */
#define CHECKWIFISTATUS_CMD_RET     (0x85)

#define DOOROPENRECORD_CMD          (0x15)      /* ���ż�¼ */
#define PUSHALARM_CMD               (0x16)      /* �������� */
#define DOORLOCKINFO_CMD            (0x17)      /* ������Ϣ���� */
#define TEMPPAWSS_CMD_RET           (0x97)      /* ��ʱ����,APP���豸���� */
#define TEMPPAWSS_CMD               (0x87)      /* �����г�ͻ,�豸��wifiģ��Ļظ� */

struct rftm {
    int tm_sec; /* �� �C ȡֵ����Ϊ[0,59] */
    int tm_min; /* �� - ȡֵ����Ϊ[0,59] */
    int tm_hour; /* ʱ - ȡֵ����Ϊ[0,23] */
    int tm_mday; /* һ�����е����� - ȡֵ����Ϊ[1,31] */
    int tm_mon; /* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
    int tm_year; /* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
};

#endif

