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
#include "IO.h"
#include "Pcs.h"
#include "Bms.h"
#include "modbus.h"
#include "mqtt.h"
#include "config.h"
#include "log.h"


pthread_t gAirConThread;
pthread_t gOnGridWhMeterThread;
pthread_t gOffGridWhMeterThread;
pthread_t gThermoThread;
pthread_t gIoThread;
pthread_t gMqttThread;
pthread_t gPcsThread;
pthread_t gBmsThread;


void ioInputCheck()
{

    if (~gIoData.input1 & (WATER_IN1_MASK | WATER_IN2_MASK))
    {
        gStopPcsFromIO = 1;
        /* ensure pcs is off */
        /* if (gPcs.runState.value == 1) */
        /*     gStopBmsFromIO = 1; */
        /* sotp aircon */
        gStopAirconFromIO = 1;
        
        
    }
    

    if (~gIoData.input1 & (AC_SPD_ALARM_MASK |  DC_SPD_ALARM_MASK))
    {
        
        gStopPcsFromIO = 1;

        
    }
    
    if (~gIoData.input1 & EMERG_STOP_MASK)
    {
        gStopPcsFromIO = 1;
        /* sotp aircon */
        gStopAirconFromIO = 1;

        
    }

    if (~gIoData.input2 & (FIRE_ALARM1_MASK | FIRE_ALARM2_MASK))
    {
        gStopPcsFromIO = 1;
        /* sotp aircon */
        gStopAirconFromIO = 1;


        
    }
    


}

int main()
{
    int err;
    FILE   *output;
    char buf[100];
    
    
    log_init("/tmp/ems.log", LOG_DEBUG);
    /* ntp update */
#if 0
ntpupdate:    
    memset(buf, '\0', sizeof(buf));
    output = popen("/usr/longertek/ntpdate.sh", "r");
    fread(buf, sizeof(char), sizeof(buf), output);
    if (!strcmp(buf, "ntpdate fail\n")) {

        log_error("ntpdate fail...");
        pclose(output);
        MsecSleep(3000);
        goto ntpupdate;
        
    }
    
    
#endif
    
#if 1    
    err = pthread_create(&gAirConThread, NULL, AirConThread, NULL);
    if (err < 0) {
        
        log_error("create aircon thread fail");
        return -1;
        
        
    }

#endif

    err = pthread_create(&gOnGridWhMeterThread, NULL, OnGridWhMeterThread, NULL);
    if (err < 0) {
        
        log_error("create on grid meter thread fail");
        return -1;
        
    }

#if 1
    err = pthread_create(&gOffGridWhMeterThread, NULL, OffGridWhMeterThread, NULL);
    if (err < 0) {
        
        log_error("create off grid meter thread fail");
        return -1;
        
    }

    
    err = pthread_create(&gThermoThread, NULL, ThermoThread, NULL);
    if (err < 0) {
        
        log_error("create thermo thread fail");
        return -1;
        
    }

    err = pthread_create(&gIoThread, NULL, IoThread, NULL);
    if (err < 0) {
        
        log_error("create IO thread fail");
        return -1;
        
    }

#if 1    
    err = pthread_create(&gMqttThread, NULL, MqttThread, NULL);
    if (err < 0) {
        
        log_error("create mqtt thread fail");
        return -1;
        
    }

#endif
#if 0
    err = pthread_create(&gPcsThread, NULL, PcsThread, NULL);
    if (err < 0) {
        
        log_error("create pcs thread fail");
        return -1;
    }
#endif
#if 1
    err = pthread_create(&gBmsThread, NULL, BmsThread, NULL);
    if (err < 0) {
        
        log_error("create bms thread fail");
        return -1;
        
    }

#endif
#endif

    while (1)
    {
        log_debug("main loop...");
        
        /* ioInputCheck(); */


        MsecSleep(200);
        

    }
    
    return 0;
    


}
