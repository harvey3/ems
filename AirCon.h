#ifndef AIR_CON
#define AIR_CON


#define AIRCON_ADDR 1
#define AIRCON_WH_METER_ADDR 2
#define AIRCON_DELAY_MSEC 1800

typedef struct air_conditioner 
{
    ModbusMaster mstatus;

    uint8_t address;
    struct rqueue *cmdRq;
    
    uint16_t startAddr;
    uint16_t endAddr;
    
    REG_T workMode;

    REG_T indoorTemp;
    REG_T outdoorTemp;
    REG_T indoorHumi;
    
    REG_T runStat;

    REG_T heatTargetTemp;
    REG_T coolTargetTemp;
    REG_T targetHumi;

    REG_T errCode;
    
    REG_T inInterchangeTemp;
    REG_T outInterchangeTemp;
    REG_T airoutTemp;

    REG_T directVolt;
    REG_T powerOn;
    
    REG_T contrlModeSet;
    REG_T workModeSet;
    
    REG_T heatTargetTempSet;
    REG_T coolTargetTempSet;
    REG_T targetHumiSet;
    REG_T dehumiOffset;
    REG_T coolOffset;
    REG_T heatOffset;
    REG_T highAlarmTemp;
    REG_T lowAlarmTemp;
    REG_T maxElecHeatTemp;
    REG_T minElecHeatTemp;

    REG_T remoteCtrl;
    

} AirCon;







typedef struct aircon_wh_meter 
{

    ModbusMaster mstatus;
    uint8_t address;
    
    uint16_t startAddr;
    uint16_t endAddr;
    
    REG_T activePower;
    REG_T reactivePower;
    REG_T apparentPower;
    REG_T powerFactor;
    REG_T abVolt;
    REG_T bcVolt;
    REG_T caVolt;

    REG_T aPhaseVolt;
    REG_T bPhaseVolt;
    REG_T cPhaseVolt;

    REG_T aPhaseCurr;
    REG_T bPhaseCurr;
    REG_T cPhaseCurr;
    
    REG_T freq;

    REG_T consumedActPower;
    REG_T producedActPower;
    REG_T capReactPower;
    REG_T inductReactPower;
    REG_T totalActPower;
    REG_T totalReactPower;
    

} AirConWhMeter;

extern AirCon gAirCon;
extern AirConWhMeter gAirConWhMeter;
extern int gStopAirconFromIO;

void* AirConThread(void *param);

#endif
