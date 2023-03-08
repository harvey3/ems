#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "mqtt.h"
#include "log.h"

extern pthread_t gPcsThread;
Pcs gPcs;
int gStopPcsFromIO = 0;

void pcsControl()
{
    struct timeval tv;
    struct tm* ptm;
    int16_t setValue;
    
 
    gettimeofday(&tv, NULL);
 
    ptm = localtime (&(tv.tv_sec));

    /* charge power  */
    if (ptm->tm_hour >= 23 || ptm->tm_hour <= 7 || ptm->tm_hour == 12) {

        if (gPcs.isRecharge) {
            
            if (gPcs.bpSOC.value < CHARGE_THRESH - RECHARGE_DIFF) {
                
                setValue = MAX_LOAD_POWER - gOffGridWhMeter.activePower.value;

                if (setValue > MAX_CHARGE_POWER)
                    setValue = MAX_CHARGE_POWER;
        
                if (setValue > abs(gPcs.maxChargePower.value))
                    setValue = abs(gPcs.maxChargePower.value);
        
                setValue = 0 - setValue;

                SetPcsActPower(&gPcs, setValue);
                if (gPcs.runState.value == PCS_STATE_STOP || gPcs.runState.value == PCS_STATE_READY) {
                    
                    startPcs(&gPcs);
                    log_debug("$$$$$$$$$$$$$charging recharge power %d SOC %d", setValue, gPcs.bpSOC.value);
                }
                
                
            }
            
        } else {
            if (gPcs.bpSOC.value < CHARGE_THRESH) {
                
                setValue = MAX_LOAD_POWER - gOffGridWhMeter.activePower.value;

                if (setValue > MAX_CHARGE_POWER)
                    setValue = MAX_CHARGE_POWER;
        
                if (setValue > abs(gPcs.maxChargePower.value))
                    setValue = abs(gPcs.maxChargePower.value);

                setValue = 0 - setValue;


                SetPcsActPower(&gPcs, setValue);
                if (gPcs.runState.value == PCS_STATE_STOP || gPcs.runState.value == PCS_STATE_READY) {
                    
                    startPcs(&gPcs);
                    log_debug("$$$$$$$$$$$$$charging  power %d SOC %d", setValue, gPcs.bpSOC.value);
                }
                

                
                
            } else {

                gPcs.isRecharge = 1;
                if (gPcs.runState.value != PCS_STATE_READY) {
                    SetPcsActPower(&gPcs, 0);
                    SetReadyPcs(&gPcs);
                    log_debug("$$$$$$$$$$$$$charging  done SOC %d", gPcs.bpSOC.value);
                }
                
                
            }
            
        


        }

    } else if ((ptm->tm_hour == 8 && ptm->tm_min > 30)
        || (ptm->tm_hour > 8 && ptm->tm_hour < 11)
        || (ptm->tm_hour == 2 && ptm->tm_min > 30)
        || (ptm->tm_hour > 2 && ptm->tm_hour < 21))
    
    {


        if (gPcs.bpSOC.value > DISCHARGE_THRESH) {
                
            setValue = gOffGridWhMeter.activePower.value - 500;

            if (setValue > MAX_DISCHARGE_POWER)
                setValue = MAX_DISCHARGE_POWER;
        
            if (setValue > abs(gPcs.maxDischargePower.value))
                setValue = abs(gPcs.maxDischargePower.value);
            

            
            SetPcsActPower(&gPcs, setValue);
            if (gPcs.runState.value == PCS_STATE_STOP || gPcs.runState.value == PCS_STATE_READY) {
                startPcs(&gPcs);
                log_debug("$$$$$$$$$$$$$discharging  power %d SOC %d", setValue, gPcs.bpSOC.value);
            }
            
                
                
        } else {

            if (gPcs.runState.value != PCS_STATE_READY) {
                
                SetPcsActPower(&gPcs, 0);
                SetReadyPcs(&gPcs);
                log_debug("$$$$$$$$$$$$$discharging  done SOC %d", gPcs.bpSOC.value);           
            }
            
        }
        



    } else {
        
        if (gPcs.runState.value != PCS_STATE_READY) {
            SetPcsActPower(&gPcs, 0);
            SetReadyPcs(&gPcs);
        }
            

    }
    


}



void* PcsThread(void *param)
{

    struct timeval tv;
    int err;

    log_debug("&&&&&&&&&&&&&&&&&&&&&&&&enter pcs thread");
    pthread_setname_np(gPcsThread, "Pcs");
    
    memset(&gPcs, 0, sizeof(gPcs));
    memset(&gPcsT, 0, sizeof(AddrTable));
    
    err = parseConfig("config/pcs.conf");
    if (err < 0)
        pthread_exit(NULL);

    log_debug("&&&&&&&&&&&&&&&&&&&&&&&&enter pcs thread 1");
    buildAddrBlock(&gPcsT);
    log_debug("&&&&&&&&&&&&&&&&&&&&&&&&enter pcs thread 2");
    err = ModbusTCPInit(&gPcs.sock, &gPcs.mstatus, PCS_IPADDR);
    if (err < 0) {
        log_error("init modbus TCP fail");
        return NULL;
        
    }
    log_debug("&&&&&&&&&&&&&&&&&&&&&&&&enter pcs thread 3");

    while (1) {
        
        tv.tv_sec=PCS_DELAY_MSEC/1000;
        tv.tv_usec=(PCS_DELAY_MSEC%1000)*1000;

    
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        GetPcsStatus(&gPcs);

        
        /* if (gStopPcsFromIO) { */
            
        /*     if (gPcs.runState.value != PCS_STATE_STOP) */
        /*         stopPcs(&gPcs); */

        
        /* } else { */
            
        /*     if (mqttDown) */
        /*         pcsControl(); */
        /* } */
        
        
    }
    
    pthread_exit(NULL);
}
