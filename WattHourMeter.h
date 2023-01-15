#ifndef WATT_HOUR_METER
#define WATT_HOUR_METER

#define OFFGRID_WH_METER_ADDR 3
#define ONGRID_WH_METER_ADDR 1

#define WH_METER_DELAY_MSEC 400

typedef struct offgrid_wh_meter 
{
    uint8_t address;
    ModbusMaster mstatus;
    uint16_t startAddr;
    uint16_t endAddr;

    REG_T activePower;
    REG_T activePowerH;
    REG_T activePowerL;
    REG_T reactivePower;
    REG_T reactivePowerH;
    REG_T reactivePowerL;
    REG_T apparentPower;
    REG_T apparentPowerH;
    REG_T apparentPowerL;
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
    REG_T consumedActPowerH;
    REG_T consumedActPowerL;
    REG_T producedActPower;
    REG_T producedActPowerH;
    REG_T producedActPowerL;

    REG_T capReactPower;
    REG_T capReactPowerH;
    REG_T capReactPowerL;
    REG_T inductReactPower;
    REG_T inductReactPowerH;
    REG_T inductReactPowerL;
    
    REG_T totalActPower;
    REG_T totalActPowerH;
    REG_T totalActPowerL;
    REG_T netActPower;
    REG_T netActPowerH;
    REG_T netActPowerL;
    
    
    REG_T totalReactPower;
    REG_T totalReactPowerH;
    REG_T totalReactPowerL;
    REG_T netReactPower;
    REG_T netReactPowerH;
    REG_T netReactPowerL;
    


} OffGridWhMeter;

typedef struct ongrid_wh_meter 
{
    uint8_t address;
    ModbusMaster mstatus;
    uint16_t startAddr;
    uint16_t endAddr;

    REG_T activePower;
    REG_T activePowerH;
    REG_T activePowerL;
    REG_T reactivePower;
    REG_T reactivePowerH;
    REG_T reactivePowerL;
    REG_T apparentPower;
    REG_T apparentPowerH;
    REG_T apparentPowerL;
    
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

    REG_T posActPower;
    REG_T posActPowerH;
    REG_T posActPowerL;
    REG_T negActPower;
    REG_T negActPowerH;
    REG_T negActPowerL;

    REG_T posReactPower;
    REG_T posReactPowerH;
    REG_T posReactPowerL;
    REG_T negReactPower;
    REG_T negReactPowerH;
    REG_T negReactPowerL;
    
    

} OnGridWhMeter;
    


extern OffGridWhMeter gOffGridWhMeter;
extern OnGridWhMeter gOnGridWhMeter;

void* OnGridWhMeterThread(void *param);
void* OffGridWhMeterThread(void *param);



#endif
