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
#include "Bms.h"
#include "Pcs.h"
#include "modbus.h"
#include "config.h"
#include "log.h"

extern pthread_t gBmsThread;

Bms gBms;

int gStopBmsFromIO = 0;


void* BmsThread(void *param)
{

    struct timeval tv;
    int err;
    int i;
    
    pthread_setname_np(gBmsThread, "Bms");
    
    memset(&gBms, 0, sizeof(gBms));
    memset(&gBmsT, 0, sizeof(AddrTable));
    err = parseConfig("config/bms.conf");
    if (err < 0)
        pthread_exit(NULL);

    buildAddrBlock(&gBmsT);

    log_debug("###############bms addr table");
    
    for (i = 0; i < gBmsT.cnt; i++)
        log_debug("addr %d", gBmsT.addr[i]);

    log_debug("###############bms addr block table");
    for (i = 0; i < gBmsT.abCnt; i++)
        log_debug("addr %d count %d", gBmsT.ab[i].addr, gBmsT.ab[i].count);


    err = ModbusTCPInit(&gBms.sock, &gBms.mstatus, BMS_IPADDR);
    if (err < 0)
        pthread_exit(NULL);
    

    while (1) {
        
        tv.tv_sec = BMS_DELAY_MSEC/1000;
        tv.tv_usec = (BMS_DELAY_MSEC%1000)*1000;

    
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        GetBmsStatus(&gBms);
        /* MsecSleep(200); */
        
        /* if (gBms.mu[0].powerOnCmd.value != 1) */
        /* BmsPowerDown(&gBms); */
        
        /* if (gStopBmsFromIO && gBms.mu[0].powerOnCmd.value != 2) */
        /*     BmsPowerDown(&gBms); */

    }
    
    pthread_exit(NULL);
}
