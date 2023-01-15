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
    if (ptm->tm_hour >= 23 || ptm->tm_hour <= 7) {

        if (gPcs.isRecharge) {
            
            if (gPcs.bpSOC.value < CHARGE_THRESH - RECHARGE_DIFF) {
                
                setValue = MAX_LOAD_POWER - gOffGridWhMeter.activePower.value;

                if (setValue > MAX_CHARGE_POWER)
                    setValue = MAX_CHARGE_POWER;
        
                if (setValue > abs(gPcs.maxChargePower.value))
                    setValue = abs(gPcs.maxChargePower.value);
        
                SetPcsActPower(&gPcs, 0 - setValue);
                if (gPcs.stopCmd.value) {
                    
                    startPcs(&gPcs);
                    gPcs.stopCmd.value = 0;
                    
                }
                
            }
            
        } else {
            if (gPcs.bpSOC.value < CHARGE_THRESH) {
                
                setValue = MAX_LOAD_POWER - gOffGridWhMeter.activePower.value;

                if (setValue > MAX_CHARGE_POWER)
                    setValue = MAX_CHARGE_POWER;
        
                if (setValue > abs(gPcs.maxChargePower.value))
                    setValue = abs(gPcs.maxChargePower.value);
        
                SetPcsActPower(&gPcs, 0 - setValue);
                if (gPcs.stopCmd.value) {
                    startPcs(&gPcs);
                    gPcs.stopCmd.value = 0;
                }
                
                
            } else {

                gPcs.isRecharge = 1;
                if (gPcs.startCmd.value) {
                    stopPcs(&gPcs);
                    gPcs.startCmd.value = 0;
                    
                    
                }
                
            }
            
        


        }

    }


    if ((ptm->tm_hour == 8 && ptm->tm_min > 30)
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
            if (gPcs.stopCmd.value) {
                startPcs(&gPcs);
                gPcs.stopCmd.value = 0;
            }
                
                
        } else {

            if (gPcs.startCmd.value) {
                stopPcs(&gPcs);
                gPcs.startCmd.value = 0;
            }
        


        }
        



    }
    



}



void* PcsThread(void *param)
{

    struct timeval tv;
    int err;

    memset(&gPcs, 0, sizeof(gPcs));
    memset(&gPcsT, 0, sizeof(AddrTable));
    
    parseConfig("config/pcs.conf");
    buildAddrBlock(&gPcsT);
    err = ModbusTCPInit(&gPcs.sock, &gPcs.mstatus, PCS_IPADDR);
    if (err < 0)
        pthread_exit(NULL);
    

    while (1) {
        
        tv.tv_sec=PCS_DELAY_MSEC/1000;
        tv.tv_usec=(PCS_DELAY_MSEC%1000)*1000;

    
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        GetPcsStatus(&gPcs);
        /* pcsControl(); */
        
        if (gStopPcsFromIO && gPcs.stopCmd.value != 1)
            stopPcs(&gPcs);
        
    }
    
    pthread_exit(NULL);
}
