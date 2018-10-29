/****************************************************************
*   void InitAllYunProp(void);      初始化云端
*   void WIFISetNetwork(void);      WIFI配网
*   void ClearWifiAPConfig(void);   清除配置
*   void GetCurTime(void);          时间同步
*   void GetWifiStatus(void);       查询wifi状态
*   int trigger_DoorOpenNotific_event(int EvenRecordID, int EvenLockType, char* EvenKeyID,
                                       unsigned char *AlarmTime, int EvenBatteryValue)      开门记录
*   int trigger_PushAlarm_event(int AlarmType, int HijackKeyID, unsigned char *AlarmTime, int EvenBatteryValue);    报警推送
*   int trigger_PushDoorLockInfo_event(unsigned char *DoorLockInfoRecv);    门锁信息推送
*   还没有开发的:APP设置音量大小，APP设置临时密码接口
****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "iot_import.h"
#include "iot_export.h"
#include "linkkit_export.h"
#include "app_entry.h"

#include"rfsmart_time.h"
#include "rfsmart_app.h"

sample_context_t *RfsmartSampleCtx = NULL;

/* 属性值的变量 */
int VolumeValue = 0;
int BatteryValue = 0;
int LockStaValue = 0;
char TempPassValue[64] = {0};
int TotalPassValue = 0;
int AutoLKStaValue = 0;
char AntCodeValue[60] = {0};
int TotalFinValue = 0;
int TotalCardValue = 0;
int ChanLkValue = 0;
int DoorOWayValue = 0;
int ActDefStaValue = 0;
int MuteStaValue = 0;
int InnLkStaValue = 0;
int FwVerValue = 0;
int DevModValue = 0;
/*
*   获取当前时间
*/
void ntp_reply(const char *ntp_offset_time_ms)
{
    unsigned int utc_time = 0;
    unsigned int local_time = 0;
    int time_zone = 8;
    char ntptimestr[11];
    struct rftm t;

    unsigned char cmd;
    unsigned char senddata[6];
    unsigned char sendlen;

    memset(ntptimestr, 0x00, 11);
    memcpy(ntptimestr, ntp_offset_time_ms, 10);
    utc_time = atoi(ntptimestr);
    local_time = utc_time + time_zone*60*60;
    rflocaltime(local_time,&t);
    printf("======>Info: %04d-%02d-%02d %02d:%02d:%02d\r\n",t.tm_year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
    /* TODO 把年月日和时分秒通过串口发送给锁具 */
    cmd = SYNCCURTIME_CMD_RET;
    sendlen = 6;
    senddata[0] = t.tm_year - 2000;
    senddata[1] = t.tm_mon+1;
    senddata[2] = t.tm_mday;
    senddata[3] = t.tm_hour;
    senddata[4] = t.tm_min;
    senddata[5] = t.tm_sec;
    UartSendFormData(cmd, senddata, sendlen);
}

void GetCurTime(void)
{
    int ret;

    ret = linkkit_ntp_time_request(ntp_reply);
    if(0 != ret){
        /* 获取时间失败 */
        printf("=====>Error: get currnt time failed.\n");
    }
}

/**********************************
*   上报属性值和事件的回调函数
**********************************/
void rfsmartpost_property_cb(const void *thing_id, int response_id, int code,
                      const char *response_message, void *ctx)
{
    printf("======>Info: thing@%p: response arrived: id:%d\tcode:%d\tmessage:%s\n",
                  thing_id, response_id, code,
                  response_message == NULL ? "NULL" : response_message);
}

/**********************************
*   上报属性值接口
*   identifier: 属性标识
*   value:  整数型型的属性值，如果是字符串类型的，此值设置为NULL
*   value_str: 字符串型的属性值，如果是整型的数值，此值设置为NULL
*   成功返回0，失败返回非0
**********************************/
int linkkit_data_publish(const char* identifier, const void* value, const char* value_str)
{
    int ret = 0;
    ret = linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, identifier, value, value_str);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_post_property(RfsmartSampleCtx->thing, identifier, rfsmartpost_property_cb);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    return ret;
}

/* 获取wifi的信息 */
void PushGetWifiInfo(void)
{
    int channel;
    channel = hal_wifi_get_channel(NULL);
    printf("\n\n======>channel is : %d\n", channel);

    char ssid[33];
    char passwd[65];
    char bssid[18];
    HAL_Wifi_Get_Ap_Info(ssid, passwd, bssid);
    printf("======>ssid:%s, passwd:%s\n", ssid, passwd);

    int rssi = 0;
	wifi_get_rssi(&rssi);
    printf("rssi:%d\n\n", rssi);

    char *band    = "2.4G";
    int snr = 0;

    linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, "WIFI_AP_BSSID", ssid, NULL);
    linkkit_post_property(RfsmartSampleCtx->thing, "WIFI_AP_BSSID", rfsmartpost_property_cb);

    linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, "WIFI_Band", band, NULL);
    linkkit_post_property(RfsmartSampleCtx->thing, "WIFI_Band", rfsmartpost_property_cb);

    linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, "WIFI_Channel", &channel, NULL);
    linkkit_post_property(RfsmartSampleCtx->thing, "WIFI_Channel", rfsmartpost_property_cb);

    linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, "WiFI_RSSI", &rssi, NULL);
    linkkit_post_property(RfsmartSampleCtx->thing, "WiFI_RSSI", rfsmartpost_property_cb);

    linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, "WiFI_SNR", &snr, NULL);
    linkkit_post_property(RfsmartSampleCtx->thing, "WiFI_SNR", rfsmartpost_property_cb);


}

/*********************************************************************
*   初始化云端
*   把属性的初始设置上传到云端
*********************************************************************/
void InitAllYunProp(void)
{
    int ret;

    VolumeValue = 0;
    BatteryValue = 0;
    LockStaValue = 0;
    memset(TempPassValue, 0x00, sizeof(TempPassValue));
    TotalPassValue = 0;
    AutoLKStaValue = 0;
    memset(AntCodeValue, 0x00, sizeof(AntCodeValue));
    TotalFinValue = 0;
    TotalCardValue = 0;
    ChanLkValue = 0;
    DoorOWayValue = 0;
    ActDefStaValue = 0;
    MuteStaValue = 0;
    InnLkStaValue = 0;
    FwVerValue = 0;
    DevModValue = 1;

    printf("======>Info: InitAllYunProp, Init all prop\n");

    /* 属性值上传 */
    ret = linkkit_data_publish(DevModPROPID, &DevModValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(AntCodePROPID, NULL, AntCodeValue);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(FwVerPROPID, &FwVerValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(LockStaPROPID, &LockStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(InnLkStaPROPID, &InnLkStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(AutoLKStaPROPID, &AutoLKStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(MuteStaPROPID, &MuteStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(ActDefStaPROPID, &ActDefStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(DoorOWayPROPID, &DoorOWayValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(ChanLkPROPID, &ChanLkValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TotalPassPROPID, &TotalPassValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TotalCardPROPID, &TotalCardValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TotalFinPROPID, &TotalFinValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(BatteryPROPID, &BatteryValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(VolumePROPID, &VolumeValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TempPassPROPID, NULL, TempPassValue);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
}

/********************************************************************
*   查询wifi的状态
********************************************************************/
unsigned char CurWiFiStatus = 0x00;
void GetWifiStatus(void)
{
    unsigned char cmd;
    unsigned char senddata;
    unsigned char sendlen;

    printf("======>Info: GetWifiStatus is %d\n", CurWiFiStatus);
    /* TODO 把CurWiFiStatus的状态发给锁具 */
    cmd = CHECKWIFISTATUS_CMD_RET;
    senddata = CurWiFiStatus;
    sendlen = 1;
    UartSendFormData(cmd, &senddata, sendlen);
}

/*********************************************************************
*   wifi配网接口
*********************************************************************/
unsigned char WIFISetNetworkFlash = 0;
void WIFISetNetwork(void)
{
    unsigned char cmd;
    unsigned char senddata;
    unsigned char sendlen;
    printf("======>Info: WIFI Set Network Start\n");
    WIFISetNetworkFlash = 0;
    do_awss_active();
    cmd = SETWIFINETWORK_CMD_RET;
    sendlen = 0x01;
    /* 是否需要延时等一下准备好了再发送? */
    if(WIFISetNetworkFlash == 1){
        /* TODO 通过linkkit_event_monitor函数的事件回调函数查看是否进入配网模式成功，然后
        通过串口把进入配网模式的结果返回给设备 */
        senddata = 0x00;
        UartSendFormData(cmd, &senddata, sendlen);
        WIFISetNetworkFlash = 0;
    }else{
        senddata = 0x01;
        UartSendFormData(cmd, &senddata, sendlen);
    }
}

/*********************************************************************
*   UnbindDeviceClearWifi
*   当设备配网成功后，云端会把设备和用户账户绑定在一块，awss_report_reset函数用于重置云端的设备信息(相当于解除绑定?)
*   返回值:无
*   解除设备绑定标志，解除绑定成功之后擦除wifi信息并重启
*   擦除wifi信息并重启,解除绑定
**********************************************************************/
extern int  awss_report_reset();
void ClearWifiInfo(void *p)
{
    printf("======>Info:UnbindDevice---->ClearWifiInfo\n");
    netmgr_clear_ap_config();
    HAL_Sys_reboot();
}
void UnbindDeviceClearWifi(void)
{
    printf("======>Info:UnbindDevice\n");
    aos_task_new("reset", (void (*)(void *))awss_report_reset, NULL, 2048);
}

/*********************************************************************
*   擦除wifi信息并重启
**********************************************************************/
void ClearWifiAPConfig(void)
{
    unsigned char cmd;
    unsigned char senddata;
    unsigned char sendlen;

    printf("======>Info:ClearWifiAPConfig\n");
    netmgr_clear_ap_config();
    /* 清除完成之后串口把结果返回给设备，1秒后重启 */
    /* TODO */
    cmd = CLEARWIFICONFIG_CMD_RET;
    sendlen = 0x01;
    senddata = 0x00;
    UartSendFormData(cmd, &senddata, sendlen);

    HAL_SleepMs(1000);
    HAL_Sys_reboot();
}

/*****************************************
*   开门记录信息推送
*   EvenRecordID:记录ID     (整型)
*   EvenLockType:开锁方式   (整型)
*   EvenKeyID:钥匙ID        (字符串)
*   AlarmTime:开锁时间  6字节   (数组)
*   EvenBatteryValue:电池电量   (整型)
*   成功返回0，失败返回非0
****************************************/
int trigger_DoorOpenNotific_event(int EvenRecordID, int EvenLockType, char* EvenKeyID,
                                       unsigned char *AlarmTime, int EvenBatteryValue)
{
    int ret;

    printf("======>Info: RecordID:%d, LockType:%d, KeyID:%s, AlarmTime: 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, BatteryValue:%d\n",
        EvenRecordID, EvenLockType, EvenKeyID, AlarmTime[0], AlarmTime[1], AlarmTime[2], AlarmTime[3], AlarmTime[4], AlarmTime[5], EvenBatteryValue);

    /* 参数判断 */
    if((strlen(EvenKeyID) > 11) || (EvenLockType > 7) || (EvenLockType < 1) || (EvenRecordID > 100) || (EvenBatteryValue > 100))
    {
        printf("======>Error: trigger_DoorOpenNotific_event:Error Parameter.\n");
        return -1;
    }

    /* 上报开门通知事件 */
    ret = linkkit_set_value(linkkit_method_set_event_output_value, RfsmartSampleCtx->thing, "DoorOpenNotification.KeyID", NULL, EvenKeyID);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_set_value(linkkit_method_set_event_output_value, RfsmartSampleCtx->thing, "DoorOpenNotification.LockType", &EvenLockType, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_set_value(linkkit_method_set_event_output_value, RfsmartSampleCtx->thing, "DoorOpenNotification.RecordID", &EvenRecordID, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_trigger_event(RfsmartSampleCtx->thing, DoorONotEVENID, rfsmartpost_property_cb);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }

    /* 上报电池电量事件 */
    ret = linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, BatteryPROPID, &EvenBatteryValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_post_property(RfsmartSampleCtx->thing, BatteryPROPID, rfsmartpost_property_cb);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }

    return ret;
}

/***************************************************
*   报警推送
*   报警类型:   AlarmType   (整型)(1:防撬、2:禁试、3:低压、4:假锁、5:主动防御，6:劫持密码、7:劫持指纹)
*   劫持钥匙ID: HijackKeyID (整型)(劫持报警有效，分别对应指纹ID或者密码ID，1个字节，非劫持报警下为0)
*   报警时间:   AlarmTime   (数组)
*   锁具电量:   EvenBatteryValue (整型)   (范围0--100)
***************************************************/
int trigger_PushAlarm_event(int AlarmType, int HijackKeyID, unsigned char *AlarmTime, int EvenBatteryValue)
{
    int ret;
    int TypeCMD;
    char HijackKeyID_char[11];

    printf("======>Info: AlarmType:%d, HijackKeyID:%d, AlarmTime:0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, EvenBatteryValue:%d\n",
        AlarmType, HijackKeyID, AlarmTime[0], AlarmTime[1], AlarmTime[2], AlarmTime[3], AlarmTime[4], AlarmTime[5], EvenBatteryValue);

    /* 参数判断 */
    if((AlarmType < 1) || (AlarmType > 7) || (EvenBatteryValue > 100)){
        printf("======>Error: trigger_PushAlarm_event:Error Parameter.\n");
        return -1;
    }

    /* 报警类型 */
    TypeCMD = AlarmType;
    switch(TypeCMD){
        case 1:
            ret = linkkit_trigger_event(RfsmartSampleCtx->thing, TamperAlarmEVENID, rfsmartpost_property_cb);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            break;

        case 2:
            ret = linkkit_trigger_event(RfsmartSampleCtx->thing, ForbiAlarmEVENID, rfsmartpost_property_cb);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            break;

        case 3:
            ret = linkkit_trigger_event(RfsmartSampleCtx->thing, LowEleAlarmEVENID, rfsmartpost_property_cb);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            break;

        case 4:
            ret = linkkit_trigger_event(RfsmartSampleCtx->thing, DoorUnlkAlarmEVENID, rfsmartpost_property_cb);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            break;

        case 5:
            ret = linkkit_trigger_event(RfsmartSampleCtx->thing, ActDefeAlarmEVENID, rfsmartpost_property_cb);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            break;

        case 6:
        case 7:
            memset(HijackKeyID_char, 0x00, 10);
            ret = sprintf(HijackKeyID_char, "%d", HijackKeyID);
            printf("======>Info: HijackKeyID_char = %s, TypeCMD = %d\n", HijackKeyID_char, TypeCMD);
            if(ret > 11){
                printf("======>Error:trigger_PushAlarm_event: error HijackKeyID value...\n");
                return -1;
            }
            ret = linkkit_set_value(linkkit_method_set_event_output_value, RfsmartSampleCtx->thing, "HijackingAlarm.KeyID", NULL, HijackKeyID_char);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
             ret = linkkit_set_value(linkkit_method_set_event_output_value, RfsmartSampleCtx->thing, "HijackingAlarm.LockType", &TypeCMD, NULL);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            ret = linkkit_trigger_event(RfsmartSampleCtx->thing, HijackAlaEVENID, rfsmartpost_property_cb);
            if(0 != ret) {
                printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
                return ret;
            }
            break;

        default:
            printf("======>Info: default--->trigger_PushAlarm_event:Error Parameter.\n");
            break;
    }

    /* 锁具电量 */
    ret = linkkit_set_value(linkkit_method_set_property_value, RfsmartSampleCtx->thing, BatteryPROPID, &EvenBatteryValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_post_property(RfsmartSampleCtx->thing, BatteryPROPID, rfsmartpost_property_cb);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }

    return ret;
}
/******************************************
*   门锁信息推送
*   DoorLockInfoRecv:   串口接收到的门锁信息数据
*   成功返回0，是被返回非0
******************************************/
int trigger_PushDoorLockInfo_event(unsigned char *DoorLockInfoRecv)
{
    int ret;

    memset(AntCodeValue, 0x00, sizeof(AntCodeValue));

    DevModValue = DoorLockInfoRecv[0];  /* 设备型号 */
    snprintf(AntCodeValue, sizeof(AntCodeValue), "%02x%02x%02x%02x", DoorLockInfoRecv[4],
        DoorLockInfoRecv[3], DoorLockInfoRecv[2], DoorLockInfoRecv[1]);     /* 窜货码 小端模式，低地址先传 */
    FwVerValue = DoorLockInfoRecv[5] | (DoorLockInfoRecv[6] << 8);  /* 固件版本 小端模式，低地址先传 */
    LockStaValue = DoorLockInfoRecv[7]; /* 门状态 */
    InnLkStaValue = DoorLockInfoRecv[8]; /* 反锁状态 */
    AutoLKStaValue = DoorLockInfoRecv[9];   /* 自动上锁状态 */
    MuteStaValue = DoorLockInfoRecv[10];    /* 静音状态 */
    ActDefStaValue = DoorLockInfoRecv[11];  /* 主动防御状态 */
    DoorOWayValue = DoorLockInfoRecv[12];   /* 开门方式 */
    ChanLkValue = DoorLockInfoRecv[13];     /* 通道锁 */
    TotalPassValue = DoorLockInfoRecv[14];  /* 密码总数 */
    TotalCardValue = DoorLockInfoRecv[15];  /* 卡总数 */
    TotalFinValue = DoorLockInfoRecv[16];   /* 指纹总数 */
    BatteryValue = DoorLockInfoRecv[17];    /* 锁具电量 */
    printf("======>Info:DevModValue:%d,AntCodeValue:%s,FwVerValue:%d,LockStaValue:%d,InnLkStaValue:%d,AutoLKStaValue:%d,MuteStaValue:%d,\n ActDefStaValue:%d,DoorOWayValue:%d,ChanLkValue:%d,TotalPassValue:%d,TotalCardValue:%d,TotalFinValue:%d,BatteryValue:%d\n",
        DevModValue, AntCodeValue, FwVerValue, LockStaValue, InnLkStaValue, AutoLKStaValue, MuteStaValue, ActDefStaValue,
        DoorOWayValue, ChanLkValue, TotalPassValue, TotalCardValue, TotalFinValue, BatteryValue);

    /* 参数判断 */
    if((DevModValue > 3) || (DevModValue < 1) || (strlen(AntCodeValue) > 60) || (FwVerValue > 65535) || (FwVerValue < 0)){
        printf("======>Error: trigger_PushDoorLockInfo_event->1:Error Parameter.\n");
        return -1;
    }
    if((LockStaValue < 0) || (LockStaValue > 1) || (InnLkStaValue > 3) || (InnLkStaValue < 0) || (InnLkStaValue == 2)){
        printf("======>Error: trigger_PushDoorLockInfo_event->2:Error Parameter.\n");
        return -1;
    }
    if((AutoLKStaValue < 0) || (AutoLKStaValue > 3) || (AutoLKStaValue == 2) || (MuteStaValue < 0) || (MuteStaValue > 3) || (MuteStaValue == 2)){
        printf("======>Error: trigger_PushDoorLockInfo_event->3:Error Parameter.\n");
        return -1;
    }
    if((ActDefStaValue < 0) || (ActDefStaValue > 3) || (ActDefStaValue == 2) || (DoorOWayValue < 0) || (DoorOWayValue > 1)){
        printf("======>Error: trigger_PushDoorLockInfo_event->4:Error Parameter.\n");
        return -1;
    }
    if((ChanLkValue < 0) || (ChanLkValue > 3) || (ChanLkValue == 2) || TotalPassValue < 0 || TotalPassValue > 255){
        printf("======>Error: trigger_PushDoorLockInfo_event->5:Error Parameter.\n");
        return -1;
    }
    if((TotalCardValue < 0) || (TotalCardValue > 255) || (TotalFinValue < 0) || (TotalFinValue > 255) || (BatteryValue < 0) || (BatteryValue > 100)){
        printf("======>Error: trigger_PushDoorLockInfo_event->6:Error Parameter.\n");
        return -1;
    }

    /* 属性值上传 */
    ret = linkkit_data_publish(DevModPROPID, &DevModValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(AntCodePROPID, NULL, AntCodeValue);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(FwVerPROPID, &FwVerValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(LockStaPROPID, &LockStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(InnLkStaPROPID, &InnLkStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(AutoLKStaPROPID, &AutoLKStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(MuteStaPROPID, &MuteStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(ActDefStaPROPID, &ActDefStaValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(DoorOWayPROPID, &DoorOWayValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(ChanLkPROPID, &ChanLkValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TotalPassPROPID, &TotalPassValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TotalCardPROPID, &TotalCardValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(TotalFinPROPID, &TotalFinValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
    ret = linkkit_data_publish(BatteryPROPID, &BatteryValue, NULL);
    if(0 != ret) {
        printf("======>Error: %s : line:%d   ret = %d, ", __func__, __LINE__, ret);
        return ret;
    }
}


/* 以下是事件的上报测试代码 */
#ifdef TESTALLEVENT
char EvenKeyID[11] = "helloworld";
int EvenLockType = 1;
int EvenRecordID = 1;
int EvenBatteryValue = 0;

int AlarmType = 1;
int HijackKeyID = 0;
unsigned char AlarmTime[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

unsigned char DoorInfoPush1[18] = {0x01, 0x01, 0x02, 0x03, 0x04, 0x12, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x22, 0x23, 0x24};
unsigned char DoorInfoPush2[18] = {0x02, 0x11, 0x12, 0x13, 0x14, 0x12, 0x25, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x22, 0x23, 0x24, 0x25};

char testeventflag = 0;
void testrfsmarteventapp(void)
{
    //trigger_DoorOpenNotific_event(EvenRecordID, EvenLockType, EvenKeyID, AlarmTime, EvenBatteryValue);
    //trigger_PushAlarm_event(AlarmType, HijackKeyID, &AlarmTime[0], EvenBatteryValue);

    if(testeventflag%2 == 0){
        trigger_PushDoorLockInfo_event(DoorInfoPush1);
    }else{
        trigger_PushDoorLockInfo_event(DoorInfoPush2);
    }

    testeventflag++;
    if(testeventflag%2 == 0){
        memset(EvenKeyID, 0x00, 10);
        memcpy(EvenKeyID, "helloworld", 10);
    }else{
        memset(EvenKeyID, 0x00, 10);
        memcpy(EvenKeyID, "testtest", 8);
    }
    EvenLockType++;
    if(EvenLockType > 7){
        EvenLockType = 1;
    }
    EvenRecordID++;
    if(EvenRecordID > 100){
        EvenRecordID = 0;
    }
    EvenBatteryValue++;
    if(EvenBatteryValue > 100){
        EvenBatteryValue = 0;
    }

    AlarmType++;
    if(AlarmType > 7){
        AlarmType = 1;
    }
    HijackKeyID++;
    AlarmTime[5] = AlarmTime[5] + 1;
}
#endif

/* 以下是测试属性上传功能的测试代码 */
#ifdef TESTALLPROP
char teststringflag = 0;
void testrfsmartstringapp(void)
{
    int ret;

    teststringflag++;
    if(teststringflag%2 == 0){
        memcpy(TempPassValue, "TempPassValueTempPassValueTempPassValueTempPassValueTempPas", 50);
        memcpy(AntCodeValue, "AntCodeValueAntCodeValueAntCodeValueAntCodeValueAntCodeValu", 50);
    }else{
        memcpy(TempPassValue, "TempPassValuehellowordhellowordhellowordhelloword", 38);
        memcpy(AntCodeValue, "AntCodeValuehellowordhellowordhellowordhelloword", 38);
    }

    printf("======>Info: AntCodeValue = %s\n", AntCodeValue);
    ret = linkkit_data_publish(AntCodePROPID, NULL, AntCodeValue);
    if(0 != ret) {
        return ret;
    }

    printf("======>Info: TempPassValue = %s\n", TempPassValue);
    ret = linkkit_data_publish(TempPassPROPID, NULL, TempPassValue);
    if(0 != ret) {
        return ret;
    }
}

void testrfsmartemunapp(void)
{
    int ret;
/*****************************************************************************/
    printf("======>Info: VolumeValue = %d\n", VolumeValue);
    ret = linkkit_data_publish(VolumePROPID, &VolumeValue, NULL);
    if(0 != ret) {
        return ret;
    }
    VolumeValue++;
    if(VolumeValue >= 3){
        VolumeValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: LockStaValue = %d\n", LockStaValue);
    ret = linkkit_data_publish(LockStaPROPID, &LockStaValue, NULL);
    if(0 != ret) {
        return ret;
    }
    LockStaValue++;
    if(LockStaValue >= 2){
        LockStaValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: DoorOWayValue = %d\n", DoorOWayValue);
    ret = linkkit_data_publish(DoorOWayPROPID, &DoorOWayValue, NULL);
    if(0 != ret) {
        return ret;
    }
    DoorOWayValue++;
    if(DoorOWayValue >= 2){
        DoorOWayValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: AutoLKStaValue = %d\n", AutoLKStaValue);
    ret = linkkit_data_publish(AutoLKStaPROPID, &AutoLKStaValue, NULL);
    if(0 != ret) {
        return ret;
    }
    AutoLKStaValue++;
    if(AutoLKStaValue == 2){
        AutoLKStaValue = 3;
    }

    if(AutoLKStaValue > 3){
        AutoLKStaValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: ChanLkValue = %d\n", ChanLkValue);
    ret = linkkit_data_publish(ChanLkPROPID, &ChanLkValue, NULL);
    if(0 != ret) {
        return ret;
    }
    ChanLkValue++;
    if(ChanLkValue == 2){
        ChanLkValue = 3;
    }

    if(ChanLkValue > 3){
        ChanLkValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: ActDefStaValue = %d\n", ActDefStaValue);
    ret = linkkit_data_publish(ActDefStaPROPID, &ActDefStaValue, NULL);
    if(0 != ret) {
        return ret;
    }
    ActDefStaValue++;
    if(ActDefStaValue == 2){
        ActDefStaValue = 3;
    }

    if(ActDefStaValue > 3){
        ActDefStaValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: MuteStaValue = %d\n", MuteStaValue);
    ret = linkkit_data_publish(MuteStaPROPID, &MuteStaValue, NULL);
    if(0 != ret) {
        return ret;
    }
    MuteStaValue++;
    if(MuteStaValue == 2){
        MuteStaValue = 3;
    }

    if(MuteStaValue > 3){
        MuteStaValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: InnLkStaValue = %d\n", InnLkStaValue);
    ret = linkkit_data_publish(InnLkStaPROPID, &InnLkStaValue, NULL);
    if(0 != ret) {
        return ret;
    }
    InnLkStaValue++;
    if(InnLkStaValue == 2){
        InnLkStaValue = 3;
    }

    if(InnLkStaValue > 3){
        InnLkStaValue = 0;
    }

/*****************************************************************************/
    printf("======>Info: DevModValue = %d\n", DevModValue);
    ret = linkkit_data_publish(DevModPROPID, &DevModValue, NULL);
    if(0 != ret) {
        return ret;
    }
    DevModValue++;
    if(DevModValue >= 3){
        DevModValue = 0;
    }

}


void testrfsmartintapp(void)
{
    int ret = 0;

    BatteryValue++;
    printf("======>Info: testrfsmartapp, BatteryValue = %d\n", BatteryValue);
    ret = linkkit_data_publish(BatteryPROPID, &BatteryValue, NULL);
    if(0 != ret) {
        return ret;
    }
    printf("======>Info: post OK\n");

    TotalPassValue++;
    printf("======>Info: testrfsmartapp, TotalPassValue = %d\n", TotalPassValue);
    ret = linkkit_data_publish(TotalPassPROPID, &TotalPassValue, NULL);
    if(0 != ret) {
        return ret;
    }
    printf("======>Info: post OK\n");

    TotalFinValue++;
    printf("======>Info: testrfsmartapp, TotalFinValue = %d\n", TotalFinValue);
    ret = linkkit_data_publish(TotalFinPROPID, &TotalFinValue, NULL);
    if(0 != ret) {
        return ret;
    }
    printf("======>Info: post OK\n");

    TotalCardValue++;
    printf("======>Info: testrfsmartapp, TotalCardValue = %d\n", TotalCardValue);
    ret = linkkit_data_publish(TotalCardPROPID, &TotalCardValue, NULL);
    if(0 != ret) {
        return ret;
    }
    printf("======>Info: post OK\n");

    FwVerValue++;
    printf("======>Info: testrfsmartapp, BatteryValue = %d\n", FwVerValue);
    ret = linkkit_data_publish(FwVerPROPID, &FwVerValue, NULL);
    if(0 != ret) {
        return ret;
    }
    printf("======>Info: post OK\n");

    return 0;
}
#endif


