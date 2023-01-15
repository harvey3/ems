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
#include "log.h"

OffGridWhMeter gOffGridWhMeter;
OnGridWhMeter gOnGridWhMeter;



void* OnGridWhMeterThread(void *param)
{

    struct timeval tv;
    int err;
    int i;
    
    memset(&gOnGridWhMeter, 0, sizeof(gOnGridWhMeter));
    memset(&gOnGridWhMeterT, 0, sizeof(AddrTable));
    
    parseConfig("config/ongridWattMeter.conf");
    buildAddrBlock(&gOnGridWhMeterT);
    gOnGridWhMeter.address = ONGRID_WH_METER_ADDR;
    log_debug("###############addr table");
    
    for (i = 0; i < gOnGridWhMeterT.cnt; i++)
        log_debug("addr %d", gOnGridWhMeterT.addr[i]);

    log_debug("###############addr block table");
    
    for (i = 0; i < gOnGridWhMeterT.abCnt; i++)
        log_debug("addr %d count %d", gOnGridWhMeterT.ab[i].addr, gOnGridWhMeterT.ab[i].count);

    err =  openSerial(ONGRID_WH_METER_PORT, 9600, 1, 0);
    if (err < 0)
        pthread_exit(NULL);
    
    ModbusInit(&gOnGridWhMeter.mstatus);
    
    while (1) {
        
        tv.tv_sec=WH_METER_DELAY_MSEC/1000;
        tv.tv_usec=(WH_METER_DELAY_MSEC%1000)*1000;

    
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        GetOnGridWhMeterStatus(&gOnGridWhMeter);
        

    }
    
    pthread_exit(NULL);
}


void* OffGridWhMeterThread(void *param)
{

    struct timeval tv;
    int err;

    memset(&gOffGridWhMeter, 0, sizeof(gOffGridWhMeter));
    memset(&gOffGridWhMeterT, 0, sizeof(AddrTable));
    
    parseConfig("config/offgridWattMeter.conf");
    buildAddrBlock(&gOffGridWhMeterT);
    gOffGridWhMeter.address = OFFGRID_WH_METER_ADDR;
    
    err =  openSerial(OFFGRID_WH_METER_PORT, 9600, 1, 0);
    if (err < 0)
        pthread_exit(NULL);
    
    ModbusInit(&gOffGridWhMeter.mstatus);
    
    while (1) {
        
        tv.tv_sec=WH_METER_DELAY_MSEC/1000;
        tv.tv_usec=(WH_METER_DELAY_MSEC%1000)*1000;

    
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        GetOffGridWhMeterStatus(&gOffGridWhMeter);
        

    }
    
    pthread_exit(NULL);
}
