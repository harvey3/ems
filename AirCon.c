#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <lightmodbus/lightmodbus.h>
#include <lightmodbus/master.h>
#include <lightmodbus/slave.h>

#include "RegAddr.h"
#include "TempSensor.h"
#include "WattHourMeter.h"
#include "AirCon.h"
#include "Uart.h"
#include "Pcs.h"
#include "Bms.h"
#include "modbus.h"
#include "config.h"
#include "Utils.h"
#include "log.h"

extern pthread_t gAirConThread;

AirCon gAirCon;
AirConWhMeter gAirConWhMeter;

int gStopAirconFromIO = 0;


void AirConProcessCmd(CMDT *cmd)
{
    switch (cmd->cmd) {

    case AIRCON_SET_WORKMODE_CMD:
        AirConSetWorkMode(&gAirCon, cmd->value);    
        break;
    case AIRCON_SET_HEAT_TEMP_CMD:
        SetHeatTargetTemp(&gAirCon, cmd->value);
        break;
    case AIRCON_SET_COOL_TEMP_CMD:
        SetCoolTargetTemp(&gAirCon, cmd->value);
        break;
    case AIRCON_SET_HUMI_CMD:
        SetTargetHumi(&gAirCon, cmd->value);
        break;
    case AIRCON_SET_DEHUMI_OFFSET_CMD:
        SetDehumiOffset(&gAirCon, cmd->value);
        break;
    case AIRCON_SET_COOL_OFFSET_CMD:
        SetCoolOffset(&gAirCon, cmd->value);
        break;
    case AIRCON_SET_HEAT_OFFSET_CMD:
        SetHeatOffset(&gAirCon, cmd->value);
        break;
    case AIRCON_SET_HIGH_ALARM_TEMP_CMD:
        SetHighAlarmTemp(&gAirCon, cmd->value);
        
        break;
    case AIRCON_SET_LOW_ALARM_TEMP_CMD:
        SetLowAlarmTemp(&gAirCon, cmd->value);
        
        break;
    case AIRCON_SET_MAX_ELECHEAT_TEMP_CMD:
        SetMaxElecHeatTemp(&gAirCon, cmd->value);
        
        break;
    case AIRCON_SET_MIN_ELECHEAT_TEMP_CMD:
        SetMinElecHeatTemp(&gAirCon, cmd->value);
        
        break;
    case AIRCON_SET_REMOTE_CTRL_CMD:
        SetRemoteCtrlMode(&gAirCon, cmd->value);
        
        break;
    default:
        log_debug("aircon cmd not support %d", cmd->cmd);
        
    }
    


}

void* AirConThread(void *param)
{

    struct timeval tv;
    int err;
    int i;
    CMDT *cmd;
    
    pthread_setname_np(gAirConThread, "AirCon");
    memset(&gAirCon, 0, sizeof(gAirCon));
    memset(&gAirConWhMeter, 0, sizeof(gAirConWhMeter));
    memset(&gAirConT, 0, sizeof(AddrTable));
    memset(&gAirConWhMeterT, 0, sizeof(AddrTable));

    gAirCon.cmdRq = rqueue_init(RQUEUE_SIZE);
    
    err = parseConfig("config/aircon.conf");
    if (err < 0)
        pthread_exit(NULL);
    
    buildAddrBlock(&gAirConT);
    

    gAirCon.address = AIRCON_ADDR;
    
    parseConfig("config/airconWattMeter.conf");
    buildAddrBlock(&gAirConWhMeterT);
    gAirConWhMeter.address = AIRCON_WH_METER_ADDR;
    log_debug("###############aircon wh addr table");
    
    for (i = 0; i < gAirConWhMeterT.cnt; i++)
        log_debug("addr %d", gAirConWhMeterT.addr[i]);

    log_debug("###############aircon wh addr block table");
    for (i = 0; i < gAirConWhMeterT.abCnt; i++)
        log_debug("addr %d count %d", gAirConWhMeterT.ab[i].addr, gAirConWhMeterT.ab[i].count);
    
    err = openSerial(AIRCON_PORT, 9600, 1, 0);
    if (err < 0)
        pthread_exit(NULL);
    
    ModbusInit(&gAirCon.mstatus);
    ModbusInit(&gAirConWhMeter.mstatus);
    
    while (1) {
        
        tv.tv_sec=AIRCON_DELAY_MSEC/1000;
        tv.tv_usec=(AIRCON_DELAY_MSEC%1000)*1000;
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);


        
        log_debug("get aircon status");
        GetAirConStatus(&gAirCon);

        while ((cmd = rqueue_read(gAirCon.cmdRq))) {

            AirConProcessCmd(cmd);
            log_debug("process aircon cmd %d", cmd->cmd);
            MsecSleep(400);

        }
        
#if 1
        
        if (gStopAirconFromIO && gAirCon.workMode.value)
            AirConSetWorkMode(&gAirCon, 0);
#endif

        log_debug("get aircon wh status");
        MsecSleep(200);
        GetAirConWhMeterStatus(&gAirConWhMeter);

    }
    rqueue_free(gAirCon.cmdRq);
    pthread_exit(NULL);
}
