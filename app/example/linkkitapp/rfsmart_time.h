#ifndef __RFSMART_TIME_H__
#define __RFSMART_TIME_H__

struct rftm {
    int tm_sec; /* �� �C ȡֵ����Ϊ[0,59] */
    int tm_min; /* �� - ȡֵ����Ϊ[0,59] */
    int tm_hour; /* ʱ - ȡֵ����Ϊ[0,23] */
    int tm_mday; /* һ�����е����� - ȡֵ����Ϊ[1,31] */
    int tm_mon; /* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
    int tm_year; /* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
};

#endif

