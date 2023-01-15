#ifndef TEMP_SENSOR
#define TEMP_SENSOR


#define TEMP_HUMI_SENSOR_ADDR1 4
#define TEMP_HUMI_SENSOR_ADDR2 2
#define TEMP_HUMI_SENSOR_ADDR3 3
#define TRANSFORMER_THERMO_ADDR 1
#define THERMO_DELAY_MSEC 1000

typedef struct temp_humi_sensor 
{
    uint8_t address;
    ModbusMaster mstatus;
    uint16_t startAddr;
    uint16_t endAddr;
    uint16_t id;
    
    REG_T temp;
    REG_T humi;


} TempHumiSensor;

typedef struct transformer_thermo 
{
    uint8_t address;
    ModbusMaster mstatus;
    uint16_t startAddr;
    uint16_t endAddr;

    REG_T status;
    REG_T aPhaseTemp;
    REG_T bPhaseTemp;
    REG_T cPhaseTemp;
    REG_T fanPeriod;

} TransThermo;

extern TempHumiSensor gTempHumiSensor1;
extern TempHumiSensor gTempHumiSensor2;
extern TempHumiSensor gTempHumiSensor3;
extern TransThermo gTransThermo;


void* ThermoThread(void *param);







#endif
