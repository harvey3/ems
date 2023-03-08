#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdint.h>
#include <string.h>
#include <stdio.h>
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

extern pthread_t gThermoThread;

TransThermo gTransThermo;
TempHumiSensor gTempHumiSensor1;
TempHumiSensor gTempHumiSensor2;
TempHumiSensor gTempHumiSensor3;



void* ThermoThread(void *param)
{

    struct timeval tv;
    int err;
    
    pthread_setname_np(gThermoThread, "Thermo");
    
    memset(&gTransThermo, 0, sizeof(gTransThermo));
    memset(&gTempHumiSensor1, 0, sizeof(gTempHumiSensor1));
    memset(&gTempHumiSensor2, 0, sizeof(gTempHumiSensor2));
    memset(&gTempHumiSensor3, 0, sizeof(gTempHumiSensor3));

    memset(&gSensorT, 0, sizeof(AddrTable));
    memset(&gThermoT, 0, sizeof(AddrTable));
    
    err = parseConfig("config/sensors.conf");
    if (err < 0)
        pthread_exit(NULL);

    buildAddrBlock(&gSensorT);
    err = parseConfig("config/thermo.conf");
    if (err < 0)
        pthread_exit(NULL);

    
    buildAddrBlock(&gThermoT);
    
    gTransThermo.address = TRANSFORMER_THERMO_ADDR;
    gTempHumiSensor1.address = TEMP_HUMI_SENSOR_ADDR1;
    gTempHumiSensor2.address = TEMP_HUMI_SENSOR_ADDR2;
    gTempHumiSensor3.address = TEMP_HUMI_SENSOR_ADDR3;
    gTempHumiSensor1.id = 1;
    gTempHumiSensor2.id = 2;
    gTempHumiSensor3.id = 3;
    
    err = openSerial(TRANS_THERMO_PORT, 9600, 1, 0);
    if (err < 0)
        pthread_exit(NULL);
    
    ModbusInit(&gTransThermo.mstatus);
    ModbusInit(&gTempHumiSensor1.mstatus);
    ModbusInit(&gTempHumiSensor2.mstatus);
    ModbusInit(&gTempHumiSensor3.mstatus);
    
    while (1) {
        
        tv.tv_sec=THERMO_DELAY_MSEC/1000;
        tv.tv_usec=(THERMO_DELAY_MSEC%1000)*1000;

    
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        GetTempSensorStatus(&gTempHumiSensor1);
        GetTempSensorStatus(&gTempHumiSensor2);
        GetTempSensorStatus(&gTempHumiSensor3);
        
        GetTransThermoStatus(&gTransThermo);
        

    }
    
    pthread_exit(NULL);
}
