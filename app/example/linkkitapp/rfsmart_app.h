#ifndef __RFSMART_APP_H__
#define __RFSMART_APP_H__

//#define TESTALLPROP   /* ���������ϴ����ܿ��� */
//#define TESTALLEVENT   /* �����¼��ϱ����ܿ��� */

typedef struct _sample_context {
    const void *thing;
    int         cloud_connected;
    int         local_connected;
    int         thing_enabled;
} sample_context_t;
extern sample_context_t *RfsmartSampleCtx;


/* �����б��ʶ */
#define VolumePROPID            "Volume"            /* ����������ö���� (0���ͣ�1���У�2���ߣ�3���޴˹���) */
#define BatteryPROPID           "BatteryPercentage" /* ��ص��� ���� (0--100) */
#define LockStaPROPID           "LockState"         /* ����״̬ ö���� (0:���� 1:����) */
#define TempPassPROPID          "TemporaryPassword" /* ��ʱ���� �ַ������� (����60) */
#define TotalPassPROPID         "TotalPassword"     /* �������� ���� (0--255) */
#define AutoLKStaPROPID         "AutoLockState"     /* �Զ�����״̬ ö���� (0:�ر� 1:���� 3:�޴˹���) */
#define AntCodePROPID           "AntiChannelingCode"/* �ܻ��� �ַ��� (����60) */
#define TotalFinPROPID          "TotalFingerprint"  /* ָ������ ���� (0--255) */
#define TotalCardPROPID         "TotalCard"         /* ������ ���� (0--255) */
#define ChanLkPROPID            "ChannelLock"       /* ͨ���� ö���� (0:�ر� 1:���� 3:�޴˹���) */
#define DoorOWayPROPID          "DoorOpenWay"       /* ���ŷ�ʽ ö���� (0:ָ�ƻ򿨻����� 1:ָ�Ƽ����뿪��) */
#define ActDefStaPROPID         "ActiveDefenseState"/* ��������״̬ ö���� (0:�ر� 1:���� 3:�޴˹���) */
#define MuteStaPROPID           "MuteState"         /* ����״̬ ö���� (0:�ر� 1:���� 3:�޴˹���) */
#define InnLkStaPROPID          "InnerLockState"    /* ����״̬ ö���� (0:�ر� 1:���� 3:�޴˹���) */
#define FwVerPROPID             "FirmwareVersion"   /* �̼��汾 ���� (0--65535) */
#define DevModPROPID            "DeviceModel"       /* �豸�ͺ� ö���� (1:K8, 2:V8, 3:V6) */

/* �¼��б��ʶ */
#define DoorONotEVENID          "DoorOpenNotification"  /* ����֪ͨ ���:Կ��ID��������ʽ����¼ID */
#define HijackAlaEVENID         "HijackingAlarm"        /* �ٳֱ��� ���:Կ��ID���������͡�*/
#define ErrorEVENID             "Error"                 /* �����ϱ� ���:���ϴ��� */
#define KeyAddedNotEVENID       "KeyAddedNotification"  /* ���Կ��֪ͨ ���:Կ��ID��������ʽ���û�Ȩ�� */
#define KeyDelNotEVENID         "KeyDeletedNotification"/* ɾ��Կ��֪ͨ ���:Կ��ID��������ʽ*/
#define KeyInfoNotEVENID        "KeyInformationNotification"    /* Կ����Ϣ�ϱ� ���:Կ��ID��������ʽ���û�Ȩ��*/
#define TamperAlarmEVENID       "TamperAlarm"           /* ���˱��� ��ʱ��������� */
#define LowEleAlarmEVENID       "LowElectricityAlarm"   /* �͵������� ��ʱ��������� */
#define DoorUnlkAlarmEVENID     "DoorUnlockedAlarm"     /* ��δ���ñ��� ��ʱ��������� */
#define TempSetCallEVENID       "TemporarySetCallback"  /* ��ʱ�������ûص� ���:�ص�״̬ */
#define DoorOpenInfoEVENID      "DoorOpenInfo"          /* ������Ϣ���� ��ʱ��������� */
#define ActDefeAlarmEVENID      "ActiveDefenseAlarm"    /* ������������ ��ʱ��������� */
#define ForbiAlarmEVENID        "ForbiddenAlarm"        /* ��ʾ���� ��ʱ��������� */

/* �����б��ʶ */
#define AddKeySERVEID           "AddKey"        /* ���Կ�� */
#define DeleteKeySERVEID        "DeleteKey"     /* ɾ��Կ�� */
#define DeleteKeySERVEID        "GetKeyList"    /* ��ȡԿ���б� */



#endif

