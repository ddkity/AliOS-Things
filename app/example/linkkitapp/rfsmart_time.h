#ifndef __RFSMART_TIME_H__
#define __RFSMART_TIME_H__

/* 协议命令定义 */
#define INITYUNDATA_CMD             (0x01)      /* 初始化云端命令 */
#define SETWIFINETWORK_CMD          (0x02)      /* WIFI配网 */
#define SETWIFINETWORK_CMD_RET      (0x82)
#define CLEARWIFICONFIG_CMD         (0x03)      /* 清除配置 */
#define CLEARWIFICONFIG_CMD_RET     (0x83)
#define SYNCCURTIME_CMD             (0x04)      /* 同步时间 */
#define SYNCCURTIME_CMD_RET         (0x84)
#define CHECKWIFISTATUS_CMD         (0x05)      /* 查询wifi状态 */
#define CHECKWIFISTATUS_CMD_RET     (0x85)

#define DOOROPENRECORD_CMD          (0x15)      /* 开门记录 */
#define PUSHALARM_CMD               (0x16)      /* 报警推送 */
#define DOORLOCKINFO_CMD            (0x17)      /* 门锁信息推送 */
#define TEMPPAWSS_CMD_RET           (0x97)      /* 临时密码,APP往设备发送 */
#define TEMPPAWSS_CMD               (0x87)      /* 命令有冲突,设备向wifi模组的回复 */

struct rftm {
    int tm_sec; /* 秒 – 取值区间为[0,59] */
    int tm_min; /* 分 - 取值区间为[0,59] */
    int tm_hour; /* 时 - 取值区间为[0,23] */
    int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
    int tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
    int tm_year; /* 年份，其值等于实际年份减去1900 */
};

#endif

