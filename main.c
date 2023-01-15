#define _GNU_SOURCE             /* See feature_test_macros(7) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
#include "IO.h"
#include "Pcs.h"
#include "Bms.h"
#include "modbus.h"
#include "mqtt.h"
#include "config.h"
#include "log.h"


extern void parseConfig(const char *path);

pthread_t gAirConThread;
pthread_t gOnGridWhMeterThread;
pthread_t gOffGridWhMeterThread;
pthread_t gThermoThread;
pthread_t gIoThread;
pthread_t gMqttThread;
pthread_t gPcsThread;
pthread_t gBmsThread;

void ioOutput()
{
    int temp = gTempHumiSensor3.temp.value/10;
    
    if (temp > 45) {

        gIoData.output1 |= TRANS_FAN_OUTPUT_MASK;
        

    } else if (temp < 40) {

        gIoData.output1 &= ~TRANS_FAN_OUTPUT_MASK;

    }
    


}

void ioInputCheck()
{

    if (gIoData.input1 & (WATER_IN1_MASK | WATER_IN2_MASK))
    {
        gStopPcsFromIO = 1;
        /* ensure pcs is off */
        if (gPcs.runState.value == 1)
            gStopBmsFromIO = 1;
        /* sotp aircon */
        gStopAirconFromIO = 1;
        
        
    }
    

    if (gIoData.input1 & (AC_SPD_ALARM_MASK |  DC_SPD_ALARM_MASK))
    {
        
        gStopPcsFromIO = 1;
        /* ensure pcs is off */
        if (gPcs.runState.value == 1)
            gStopBmsFromIO = 1;




        
    }
    
    if (gIoData.input1 & EMERG_STOP_MASK)
    {
        gStopPcsFromIO = 1;
        /* ensure pcs is off */
        if (gPcs.runState.value == 1)
            gStopBmsFromIO = 1;
        /* sotp aircon */
        gStopAirconFromIO = 1;

        
    }

    if (gIoData.input2 & (FIRE_ALARM1_MASK | FIRE_ALARM2_MASK))
    {
        gStopPcsFromIO = 1;
        /* ensure pcs is off */
        if (gPcs.runState.value == 1)
            gStopBmsFromIO = 1;
        /* sotp aircon */
        gStopAirconFromIO = 1;


        
    }


    


}

int main()
{
    int err;
    
    log_init("/tmp/ems.log", LOG_DEBUG);

#if 1    
    err = pthread_create(&gAirConThread, NULL, AirConThread, NULL);
    if (err < 0) {
        
        log_error("create aircon thread fail");
        exit(0);
        
    }
     pthread_setname_np(gAirConThread, "AirCon");
#endif

    err = pthread_create(&gOnGridWhMeterThread, NULL, OnGridWhMeterThread, NULL);
    if (err < 0) {
        
        log_error("create on grid meter thread fail");
        exit(0);
        
    }
    pthread_setname_np(gOnGridWhMeterThread, "OnGridWhMeter");
#if 1
    err = pthread_create(&gOffGridWhMeterThread, NULL, OffGridWhMeterThread, NULL);
    if (err < 0) {
        
        log_error("create off grid meter thread fail");
        exit(0);
        
    }
    pthread_setname_np(gOffGridWhMeterThread, "OffGridWhMeter");
    
    err = pthread_create(&gThermoThread, NULL, ThermoThread, NULL);
    if (err < 0) {
        
        log_error("create thermo thread fail");
        exit(0);
        
    }
    pthread_setname_np(gThermoThread, "Thermo");
    err = pthread_create(&gIoThread, NULL, IoThread, NULL);
    if (err < 0) {
        
        log_error("create IO thread fail");
        exit(0);
        
    }
    pthread_setname_np(gIoThread, "IO");
#if 1    
    err = pthread_create(&gMqttThread, NULL, MqttThread, NULL);
    if (err < 0) {
        
        log_error("create mqtt thread fail");
        exit(0);
        
    }
    pthread_setname_np(gMqttThread, "Mqtt");
#endif
    err = pthread_create(&gPcsThread, NULL, PcsThread, NULL);
    if (err < 0) {
        
        log_error("create pcs thread fail");
        exit(0);
        
    }
    pthread_setname_np(gPcsThread, "Pcs");
#if 0
    err = pthread_create(&gBmsThread, NULL, BmsThread, NULL);
    if (err < 0) {
        
        log_error("create bms thread fail");
        exit(0);
        
    }
    pthread_setname_np(gBmsThread, "Bms");
#endif
#endif

    while (1)
    {
        log_debug("main loop...");
        
        /* ioInputCheck(); */
        /* ioOutput(); */

        MsecSleep(200);
        

    }
    
    return 0;
    


}
