#ifndef BMS_HEADER
#define BMS_HEADER


#define BMS_ADDR 30
#define BMS_IPADDR "192.168.1.30"

#define BMS_DELAY_MSEC 400

typedef struct sbmu_temp_data
{
    uint16_t startAddr;
    uint16_t endAddr;

    REG_T bpTemp[128];
} SbmuTempData;

typedef struct sbmu_volt_data
{
    uint16_t startAddr;
    uint16_t endAddr;

    REG_T bpVolt[512];
} SbmuVoltData;

    
typedef struct sbmu
{

    uint16_t startAddr;
    uint16_t endAddr;
    
    REG_T totalVolt;
    REG_T totalCurr;

    REG_T SOC;
    REG_T SOH;
    REG_T maxSingleBpVolt;
    REG_T minSingleBpVolt;

    REG_T avgSingleBpVolt;
    
    REG_T maxSingleBpTemp;
    REG_T minSingelBpTemp;
    REG_T avgSingelBpTemp;

    REG_T maxAllowedChargeCurr;
    REG_T maxAllowedDischrgCurr;

    REG_T EnvironTemp;
    
    REG_T mainRelaystat;
    
    SbmuVoltData voltData;
    SbmuTempData tempData;
    
    
} Sbmu;

typedef struct mbmu
{

    uint16_t startAddr;
    uint16_t endAddr;
    
    REG_T totalVolt;
    REG_T totalCurr;

    REG_T SOC;
    REG_T SOH;
    REG_T maxSingleBpVolt;
    REG_T minSingleBpVolt;

    REG_T avgSingleBpVolt;
    
    REG_T maxSingleBpTemp;
    REG_T minSingleBpTemp;
    REG_T avgSingleBpTemp;

    REG_T maxAllowedChargeCurr;
    REG_T maxAllowedDischrgCurr;

    REG_T avgEnvironTemp;
    REG_T heartBeat;
    REG_T avgEnvironHumi;

    REG_T bpSysStatus;
    REG_T maxSingleBpVoltPos;
    REG_T minSingleBpVoltPos;
    REG_T maxSingleBpTempPos;
    REG_T minSingleBpTempPos;
    
    REG_T cycleCnt;
    REG_T powerOnCmd;
    
    Sbmu su[8];
    
    
} Mbmu;

typedef struct bms 
{
    int sock;
    ModbusMaster mstatus;

    Mbmu mu[2];


} Bms;

extern Bms gBms;
extern int gStopBmsFromIO;

void* BmsThread(void *param);


#endif
