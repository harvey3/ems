#ifndef PCS_HEADER
#define PCS_HEADER


#define PCS_ADDR 20
#define PCS_IPADDR "192.168.1.20"

#define PCS_DELAY_MSEC 400

#define MAX_LOAD_POWER 4500             /* 450kw */
#define MAX_CHARGE_POWER 3000           /* 300kw */
#define MAX_DISCHARGE_POWER 3000           /* 300kw */
#define CHARGE_THRESH  950              /* 95% */
#define RECHARGE_DIFF 30              /* 3% */
#define DISCHARGE_THRESH 100              /* 10% */

#define PCS_STATE_STOP 0
#define PCS_STATE_READY 1
#define PCS_STATE_DISCHARGING 2
#define PCS_STATE_CHARGING 3
#define PCS_STATE_ZERO_POWER 4

typedef struct pcs
{

    int sock;
    uint16_t address;
    ModbusMaster mstatus;
    
    uint16_t startAddr;
    uint16_t endAddr;

    int isRecharge;
    
    REG_T maxChargePower;
    REG_T maxDischargePower;

    REG_T busOverVoltValue;

    REG_T busLowVoltValue;
    REG_T halfBusErrValue;
    REG_T dcOverCurrValue;

    REG_T environOverHeatValue;
    REG_T lcOverHeatValue;

    REG_T igbtOverHeatValue;
    REG_T environLowTempValue;
    REG_T igbtLowTempValue;

    REG_T ratedPower;
    REG_T ratedVolt;
    REG_T ratedCurr;
    
    REG_T acBreaker;
    REG_T acBreakerTrip;

    REG_T dcBreaker;
    REG_T outputPowerFactor;
    REG_T acCurr;

    REG_T aPhaseIgbtTemp;
    REG_T bPhaseIgbtTemp;
    REG_T cPhaseIgbtTemp;
    
    REG_T lcTemp;
    REG_T environTemp;

    REG_T lowHalfBusVolt;
    REG_T highHalfBusVolt;

    REG_T totalOutApparentPower;
    REG_T maxAllowedApparentPower;

    REG_T runState;
    
    REG_T ctrlMode;
    REG_T workMode;
    REG_T runStateSet;

    REG_T startCmd;
    REG_T stopCmd;
    REG_T readyCmd;
    REG_T setActPower;

    
    REG_T totalBpVolt;
    REG_T totalBpCurr;
    
    REG_T maxChargeCurr;
    REG_T maxDischargeCurr;

    REG_T maxSingleBpVolt;
    REG_T minSingleBpVolt;

    REG_T bpSOC;
    REG_T chargePower;

    REG_T abVolt;
    REG_T bcVolt;
    REG_T caVolt;

    REG_T aPhaseCurr;
    REG_T bPhaseCurr;
    REG_T cPhaseCurr;
} Pcs;


extern Pcs gPcs;
extern int gStopPcsFromIO;

void* PcsThread(void *param);
void SetPcsActPower(Pcs *pcs, int16_t power);
void GetPcsStatus(Pcs *pcs);
void SetReadyPcs(Pcs *pcs);


#endif
