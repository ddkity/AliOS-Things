#ifndef __RFSMART_APP_H__
#define __RFSMART_APP_H__

//#define TESTALLPROP   /* 测试属性上传性能开关 */
//#define TESTALLEVENT   /* 测试事件上报性能开关 */

typedef struct _sample_context {
    const void *thing;
    int         cloud_connected;
    int         local_connected;
    int         thing_enabled;
} sample_context_t;
extern sample_context_t *RfsmartSampleCtx;


/* 属性列表标识 */
#define VolumePROPID            "Volume"            /* 门锁音量，枚举型 (0：低，1：中，2：高，3：无此功能) */
#define BatteryPROPID           "BatteryPercentage" /* 电池电量 整型 (0--100) */
#define LockStaPROPID           "LockState"         /* 门锁状态 枚举型 (0:关门 1:开门) */
#define TempPassPROPID          "TemporaryPassword" /* 临时密码 字符串类型 (长度60) */
#define TotalPassPROPID         "TotalPassword"     /* 密码总数 整型 (0--255) */
#define AutoLKStaPROPID         "AutoLockState"     /* 自动上锁状态 枚举型 (0:关闭 1:开启 3:无此功能) */
#define AntCodePROPID           "AntiChannelingCode"/* 窜货吗 字符串 (长度60) */
#define TotalFinPROPID          "TotalFingerprint"  /* 指纹总数 整型 (0--255) */
#define TotalCardPROPID         "TotalCard"         /* 卡总数 整型 (0--255) */
#define ChanLkPROPID            "ChannelLock"       /* 通道锁 枚举型 (0:关闭 1:开启 3:无此功能) */
#define DoorOWayPROPID          "DoorOpenWay"       /* 开门方式 枚举型 (0:指纹或卡或密码 1:指纹加密码开锁) */
#define ActDefStaPROPID         "ActiveDefenseState"/* 主动防御状态 枚举型 (0:关闭 1:开启 3:无此功能) */
#define MuteStaPROPID           "MuteState"         /* 静音状态 枚举型 (0:关闭 1:开启 3:无此功能) */
#define InnLkStaPROPID          "InnerLockState"    /* 反锁状态 枚举型 (0:关闭 1:开启 3:无此功能) */
#define FwVerPROPID             "FirmwareVersion"   /* 固件版本 整型 (0--65535) */
#define DevModPROPID            "DeviceModel"       /* 设备型号 枚举型 (1:K8, 2:V8, 3:V6) */

/* 事件列表标识 */
#define DoorONotEVENID          "DoorOpenNotification"  /* 开门通知 输出:钥匙ID、开锁方式、记录ID */
#define HijackAlaEVENID         "HijackingAlarm"        /* 劫持报警 输出:钥匙ID、报警类型、*/
#define ErrorEVENID             "Error"                 /* 故障上报 输出:故障代码 */
#define KeyAddedNotEVENID       "KeyAddedNotification"  /* 添加钥匙通知 输出:钥匙ID、开锁方式、用户权限 */
#define KeyDelNotEVENID         "KeyDeletedNotification"/* 删除钥匙通知 输出:钥匙ID、开锁方式*/
#define KeyInfoNotEVENID        "KeyInformationNotification"    /* 钥匙信息上报 输出:钥匙ID、开锁方式、用户权限*/
#define TamperAlarmEVENID       "TamperAlarm"           /* 防撬报警 暂时无输出参数 */
#define LowEleAlarmEVENID       "LowElectricityAlarm"   /* 低电量报警 暂时无输出参数 */
#define DoorUnlkAlarmEVENID     "DoorUnlockedAlarm"     /* 门未锁好报警 暂时无输出参数 */
#define TempSetCallEVENID       "TemporarySetCallback"  /* 临时密码设置回调 输出:回调状态 */
#define DoorOpenInfoEVENID      "DoorOpenInfo"          /* 门锁信息推送 暂时无输出参数 */
#define ActDefeAlarmEVENID      "ActiveDefenseAlarm"    /* 主动防御报警 暂时无输出参数 */
#define ForbiAlarmEVENID        "ForbiddenAlarm"        /* 禁示报警 暂时无输出参数 */

/* 服务列表标识 */
#define AddKeySERVEID           "AddKey"        /* 添加钥匙 */
#define DeleteKeySERVEID        "DeleteKey"     /* 删除钥匙 */
#define DeleteKeySERVEID        "GetKeyList"    /* 获取钥匙列表 */



#endif

