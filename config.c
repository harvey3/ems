#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

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
#include "IO.h"
#include "config.h"
#include "log.h"

AddrTable gAirConT;
AddrTable gAirConWhMeterT;
AddrTable gOffGridWhMeterT;
AddrTable gOnGridWhMeterT;
AddrTable gPcsT;
AddrTable gBmsT;
AddrTable gThermoT;
AddrTable gSensorT;

    
unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;
    
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}


inline int checkRegName(char s)
{
    if (isalpha(s) || isdigit(s))
        return 1;
    else
        return 0;


}
inline int checkHexChar(char s)
{
    if ((s >= 'A' && s <= 'F') || isdigit(s))
        return 1;
    else
        return 0;
    


}

int findNextReturnChar(char *p)
{
    int i = 0;
    while (p[i] != '\n')
        i++;

    return i;


}

void addAddrToTable(AddrTable *t, unsigned short addr)
{
    int i;
    int j;

    if (t->cnt == TABLE_SIZE) {
        log_error("table overflow!");
        return;
        
    } else if (t->cnt == 0) {
        t->addr[0] = addr;
        t->cnt++;

    } else {
        
        for (i = 0; i < t->cnt; i++) {
            if (addr == t->addr[i])
                return;
            
            else if (addr < t->addr[i]) {
                for(j = t->cnt - 1; j >= i; j--) 
                    t->addr[j + 1] = t->addr[j];
                break;
            
            }
        

        }

        t->addr[i] = addr;
        t->cnt++;
    }
    
}


void buildAddrTable(AddrTable *t, ParseResult *pr)
{
    unsigned short addr;
    int i;

    for (i = 0; pr->flag[i] > 0; i++)
    {
        if (pr->flag[i] == 3 || pr->flag[i] == 4) {
            
            addr = pr->opQ[i].addr;
            addAddrToTable(t, addr);
        
        }


    }

}
int buildAddrBlock(AddrTable *t)
{
    int i;
    unsigned short start;
    int j = 0;
    int k = 0;
     
    if (!t->cnt)
        return -1;

    start = t->addr[j];    
    for (i = 1; i < t->cnt; i++) {
        if (t->addr[i] - start > 120) {
            t->ab[k].addr = start;
            t->ab[k].count = t->addr[i - 1] - start + 1; 
            k++;
            j = i;
            start = t->addr[j];
            
            t->abCnt++;

        }
            
    }
    t->ab[k].addr = start;
    t->ab[k].count = t->addr[i - 1] - start + 1;
    t->abCnt++;
    return 0;
    

}


int lookupAddrTable(AddrTable *t, unsigned short addr)
{
    int high, low, mid;
    
    low = 0;
    high = t->cnt - 1;
    
    while (low <= high)
        {
            mid = low + (high - low)/2;
            if(t->addr[mid] > addr)
                high = mid - 1;
            else if(t->addr[mid] < addr)
                low = mid + 1;
            else {
                
                return mid;
                
            }
            
        }
    
    return -1;

}

void parseBmsMeter(Bms *bms, ParseResult *pr)
{
    if (!bms || !pr)
        return;

    buildAddrTable(&gBmsT, pr);
    
    if (!strcmp("powerOnCmd", pr->regName)) {
        memcpy(bms->mu[0].powerOnCmd.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(bms->mu[0].powerOnCmd.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("totalVolt", pr->regName)) {
        memcpy(bms->mu[0].totalVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(bms->mu[0].totalVolt.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("totalCurr", pr->regName)) {
        memcpy(bms->mu[0].totalCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(bms->mu[0].totalCurr.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("SOC", pr->regName)) {
        memcpy(bms->mu[0].SOC.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(bms->mu[0].SOC.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("SOH", pr->regName)) {
        memcpy(bms->mu[0].SOH.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(bms->mu[0].SOH.flag, pr->flag, sizeof(pr->flag));


    } 






}

void parsePcsMeter(Pcs *pcs, ParseResult *pr)
{
    if (!pcs || !pr)
        return;

    buildAddrTable(&gPcsT, pr);
    
    if (!strcmp("maxChargePower", pr->regName)) {
        memcpy(pcs->maxChargePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->maxChargePower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("maxDischargePower", pr->regName)) {
        memcpy(pcs->maxDischargePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->maxDischargePower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("busOverVoltValue", pr->regName)) {
        memcpy(pcs->busOverVoltValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->busOverVoltValue.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("busLowVoltValue", pr->regName)) {
        memcpy(pcs->busLowVoltValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->busLowVoltValue.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("halfBusErrValue", pr->regName)) {
        memcpy(pcs->halfBusErrValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->halfBusErrValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("dcOverCurrValue", pr->regName)) {
        memcpy(pcs->dcOverCurrValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->dcOverCurrValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("environOverHeatValue", pr->regName)) {
        memcpy(pcs->environOverHeatValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->environOverHeatValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("lcOverHeatValue", pr->regName)) {
        memcpy(pcs->lcOverHeatValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->lcOverHeatValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("igbtOverHeatValue", pr->regName)) {
        memcpy(pcs->igbtOverHeatValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->igbtOverHeatValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("environLowTempValue", pr->regName)) {
        memcpy(pcs->environLowTempValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->environLowTempValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("igbtLowTempValue", pr->regName)) {
        memcpy(pcs->igbtLowTempValue.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->igbtLowTempValue.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("ratedPower", pr->regName)) {
        memcpy(pcs->ratedPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->ratedPower.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("ratedVolt", pr->regName)) {
        memcpy(pcs->ratedVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->ratedVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("ratedCurr", pr->regName)) {
        memcpy(pcs->ratedCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->ratedCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("acBreaker", pr->regName)) {
        memcpy(pcs->acBreaker.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->acBreaker.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("acBreakerTrip", pr->regName)) {
        memcpy(pcs->acBreakerTrip.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->acBreakerTrip.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("dcBreaker", pr->regName)) {
        memcpy(pcs->dcBreaker.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->dcBreaker.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("outputPowerFactor", pr->regName)) {
        memcpy(pcs->outputPowerFactor.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->outputPowerFactor.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("acCurr", pr->regName)) {
        memcpy(pcs->acCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->acCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("aPhaseIgbtTemp", pr->regName)) {
        memcpy(pcs->aPhaseIgbtTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->aPhaseIgbtTemp.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("bPhaseIgbtTemp", pr->regName)) {
        memcpy(pcs->bPhaseIgbtTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->bPhaseIgbtTemp.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("cPhaseIgbtTemp", pr->regName)) {
        memcpy(pcs->cPhaseIgbtTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->cPhaseIgbtTemp.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("lcTemp", pr->regName)) {
        memcpy(pcs->lcTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->lcTemp.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("environTemp", pr->regName)) {
        memcpy(pcs->environTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->environTemp.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("lowHalfBusVolt", pr->regName)) {
        memcpy(pcs->lowHalfBusVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->lowHalfBusVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("highHalfBusVolt", pr->regName)) {
        memcpy(pcs->highHalfBusVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->highHalfBusVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("totalOutApparentPower", pr->regName)) {
        memcpy(pcs->totalOutApparentPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->totalOutApparentPower.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("maxAllowedApparentPower", pr->regName)) {
        memcpy(pcs->maxAllowedApparentPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->maxAllowedApparentPower.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("ctrlMode", pr->regName)) {
        memcpy(pcs->ctrlMode.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->ctrlMode.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("workMode", pr->regName)) {
        memcpy(pcs->workMode.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->workMode.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("runState", pr->regName)) {
        memcpy(pcs->runState.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->runState.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("startCmd", pr->regName)) {
        memcpy(pcs->startCmd.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->startCmd.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("stopCmd", pr->regName)) {
        memcpy(pcs->stopCmd.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->stopCmd.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("readyCmd", pr->regName)) {
        memcpy(pcs->readyCmd.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->readyCmd.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("setActPower", pr->regName)) {
        memcpy(pcs->setActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->setActPower.flag, pr->flag, sizeof(pr->flag));
        

        
    } else if (!strcmp("totalBpVolt", pr->regName)) {
        memcpy(pcs->totalBpVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->totalBpVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("totalBpCurr", pr->regName)) {
        memcpy(pcs->totalBpCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->totalBpCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("maxChargeCurr", pr->regName)) {
        memcpy(pcs->maxChargeCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->maxChargeCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("maxDischargeCurr", pr->regName)) {
        memcpy(pcs->maxDischargeCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->maxDischargeCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("maxSingleBpVolt", pr->regName)) {
        memcpy(pcs->maxSingleBpVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->maxSingleBpVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("minSingleBpVolt", pr->regName)) {
        memcpy(pcs->minSingleBpVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->minSingleBpVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("bpSOC", pr->regName)) {
        memcpy(pcs->bpSOC.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->bpSOC.flag, pr->flag, sizeof(pr->flag));
        
    } else if (!strcmp("abVolt", pr->regName)) {
        memcpy(pcs->abVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->abVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("bcVolt", pr->regName)) {
        memcpy(pcs->bcVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->bcVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("caVolt", pr->regName)) {
        memcpy(pcs->caVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->caVolt.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("aPhaseCurr", pr->regName)) {
        memcpy(pcs->aPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->aPhaseCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("bPhaseCurr", pr->regName)) {
        memcpy(pcs->bPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->bPhaseCurr.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("cPhaseCurr", pr->regName)) {
        memcpy(pcs->cPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(pcs->cPhaseCurr.flag, pr->flag, sizeof(pr->flag));
        

    } 
    

}

void parseAirConWattMeter(AirConWhMeter *whm, ParseResult *pr)
{

    if (!whm || !pr)
        return;
    
    buildAddrTable(&gAirConWhMeterT, pr);
    
    if (!strcmp("activePower", pr->regName)) {
        memcpy(whm->activePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->activePower.flag, pr->flag, sizeof(pr->flag));
    } else if (!strcmp("reactivePower", pr->regName)) {
        memcpy(whm->reactivePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->reactivePower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("apparentPower", pr->regName)) {
        memcpy(whm->apparentPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->apparentPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("powerFactor", pr->regName)) {
        memcpy(whm->powerFactor.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->powerFactor.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("abVolt", pr->regName)) {
        memcpy(whm->abVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->abVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bcVolt", pr->regName)) {
        memcpy(whm->bcVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bcVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("caVolt", pr->regName)) {
        memcpy(whm->caVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->caVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseVolt", pr->regName)) {
        memcpy(whm->aPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->aPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseVolt", pr->regName)) {
        memcpy(whm->bPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseVolt", pr->regName)) {
        memcpy(whm->cPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->cPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseCurr", pr->regName)) {
        memcpy(whm->aPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->aPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseCurr", pr->regName)) {
        memcpy(whm->bPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseCurr", pr->regName)) {
        memcpy(whm->cPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->cPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("freq", pr->regName)) {
        memcpy(whm->freq.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->freq.flag, pr->flag, sizeof(pr->flag));

    }

    else if (!strcmp("consumedActPower", pr->regName)) {
        memcpy(whm->consumedActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->consumedActPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("producedActPower", pr->regName)) {
        memcpy(whm->producedActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->producedActPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("capReactPower", pr->regName)) {
        memcpy(whm->capReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->capReactPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("inductReactPower", pr->regName)) {
        memcpy(whm->inductReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->inductReactPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("totalActPower", pr->regName)) {
        memcpy(whm->totalActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->totalActPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("totalReactPower", pr->regName)) {
        memcpy(whm->totalReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->totalReactPower.flag, pr->flag, sizeof(pr->flag));
    }

}

void parseOngridWattMeter(OnGridWhMeter *whm, ParseResult *pr)
{

    if (!whm || !pr)
        return;
    buildAddrTable(&gOnGridWhMeterT, pr);
    
    if (!strcmp("activePower", pr->regName)) {
        memcpy(whm->activePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->activePower.flag, pr->flag, sizeof(pr->flag));
    } else if (!strcmp("reactivePower", pr->regName)) {
        memcpy(whm->reactivePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->reactivePower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("apparentPower", pr->regName)) {
        memcpy(whm->apparentPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->apparentPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("powerFactor", pr->regName)) {
        memcpy(whm->powerFactor.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->powerFactor.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("abVolt", pr->regName)) {
        memcpy(whm->abVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->abVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bcVolt", pr->regName)) {
        memcpy(whm->bcVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bcVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("caVolt", pr->regName)) {
        memcpy(whm->caVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->caVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseVolt", pr->regName)) {
        memcpy(whm->aPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->aPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseVolt", pr->regName)) {
        memcpy(whm->bPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseVolt", pr->regName)) {
        memcpy(whm->cPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->cPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseCurr", pr->regName)) {
        memcpy(whm->aPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->aPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseCurr", pr->regName)) {
        memcpy(whm->bPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseCurr", pr->regName)) {
        memcpy(whm->cPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->cPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("freq", pr->regName)) {
        memcpy(whm->freq.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->freq.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("posActPower", pr->regName)) {
        memcpy(whm->posActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->posActPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("negActPower", pr->regName)) {
        memcpy(whm->negActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->negActPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("posReactPower", pr->regName)) {
        memcpy(whm->posReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->posReactPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("negReactPower", pr->regName)) {
        memcpy(whm->negReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->negReactPower.flag, pr->flag, sizeof(pr->flag));


    } 
    
    
}
void parseOffgridWattMeter(OffGridWhMeter *whm, ParseResult *pr)
{

    if (!whm || !pr)
        return;

    buildAddrTable(&gOffGridWhMeterT, pr);
    
    if (!strcmp("activePower", pr->regName)) {
        memcpy(whm->activePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->activePower.flag, pr->flag, sizeof(pr->flag));
    } else if (!strcmp("reactivePower", pr->regName)) {
        memcpy(whm->reactivePower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->reactivePower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("apparentPower", pr->regName)) {
        memcpy(whm->apparentPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->apparentPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("powerFactor", pr->regName)) {
        memcpy(whm->powerFactor.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->powerFactor.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("abVolt", pr->regName)) {
        memcpy(whm->abVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->abVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bcVolt", pr->regName)) {
        memcpy(whm->bcVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bcVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("caVolt", pr->regName)) {
        memcpy(whm->caVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->caVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseVolt", pr->regName)) {
        memcpy(whm->aPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->aPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseVolt", pr->regName)) {
        memcpy(whm->bPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseVolt", pr->regName)) {
        memcpy(whm->cPhaseVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->cPhaseVolt.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseCurr", pr->regName)) {
        memcpy(whm->aPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->aPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseCurr", pr->regName)) {
        memcpy(whm->bPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->bPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseCurr", pr->regName)) {
        memcpy(whm->cPhaseCurr.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->cPhaseCurr.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("freq", pr->regName)) {
        memcpy(whm->freq.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->freq.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("consumedActPower", pr->regName)) {
        memcpy(whm->consumedActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->consumedActPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("producedActPower", pr->regName)) {
        memcpy(whm->producedActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->producedActPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("capReactPower", pr->regName)) {
        memcpy(whm->capReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->capReactPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("inductReactPower", pr->regName)) {
        memcpy(whm->inductReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->inductReactPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("totalActPower", pr->regName)) {
        memcpy(whm->totalActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->totalActPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("netActPower", pr->regName)) {
        memcpy(whm->netActPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->netActPower.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("totalReactPower", pr->regName)) {
        memcpy(whm->totalReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->totalReactPower.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("netReactPower", pr->regName)) {
        memcpy(whm->netReactPower.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(whm->netReactPower.flag, pr->flag, sizeof(pr->flag));

    }
    
    
}

void parseThermo(TransThermo *tth, ParseResult *pr)
{

    if (!tth || !pr)
        return;
    
    buildAddrTable(&gThermoT, pr);
    

    if (!strcmp("status", pr->regName)) {
        memcpy(tth->status.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(tth->status.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("aPhaseTemp", pr->regName)) {
        memcpy(tth->aPhaseTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(tth->aPhaseTemp.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("bPhaseTemp", pr->regName)) {
        memcpy(tth->bPhaseTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(tth->bPhaseTemp.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("cPhaseTemp", pr->regName)) {
        memcpy(tth->cPhaseTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(tth->cPhaseTemp.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("fanPeriod", pr->regName)) {
        memcpy(tth->fanPeriod.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(tth->fanPeriod.flag, pr->flag, sizeof(pr->flag));

    }
    
}

void parseSensors(TempHumiSensor *ths, ParseResult *pr)
{

    if (!ths || !pr)
        return;

    buildAddrTable(&gSensorT, pr);
    

    if (!strcmp("temp", pr->regName)) {
        memcpy(ths->temp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ths->temp.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("humi", pr->regName)) {
        memcpy(ths->humi.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ths->humi.flag, pr->flag, sizeof(pr->flag));

    }
    
}

void parseAirCon(AirCon *ac, ParseResult *pr)
{

    if (!ac || !pr)
        return;

    buildAddrTable(&gAirConT, pr);
    
    if (!strcmp("workModeSet", pr->regName)) {
        memcpy(ac->workModeSet.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->workModeSet.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("workMode", pr->regName)) {
        memcpy(ac->workMode.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->workMode.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("indoorTemp", pr->regName)) {
        memcpy(ac->indoorTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->indoorTemp.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("outdoorTemp", pr->regName)) {
        memcpy(ac->outdoorTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->outdoorTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("indoorHumi", pr->regName)) {
        memcpy(ac->indoorHumi.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->indoorHumi.flag, pr->flag, sizeof(pr->flag));
        

    } else if (!strcmp("runStat", pr->regName)) {
        memcpy(ac->runStat.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->runStat.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("heatTargetTemp", pr->regName)) {
        memcpy(ac->heatTargetTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->heatTargetTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("coolTargetTemp", pr->regName)) {
        memcpy(ac->coolTargetTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->coolTargetTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("targetHumi", pr->regName)) {
        memcpy(ac->targetHumi.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->targetHumi.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("errCode", pr->regName)) {

        memcpy(ac->errCode.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->errCode.flag, pr->flag, sizeof(pr->flag));

    } else if (!strcmp("inInterchangeTemp", pr->regName)) {
        memcpy(ac->inInterchangeTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->inInterchangeTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("outInterchangeTemp", pr->regName)) {
        memcpy(ac->outInterchangeTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->outInterchangeTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("airoutTemp", pr->regName)) {
        memcpy(ac->airoutTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->airoutTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("directVolt", pr->regName)) {
        memcpy(ac->directVolt.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->directVolt.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("heatTargetTempSet", pr->regName)) {
        memcpy(ac->heatTargetTempSet.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->heatTargetTempSet.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("coolTargetTempSet", pr->regName)) {
        memcpy(ac->coolTargetTempSet.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->coolTargetTempSet.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("targetHumiSet", pr->regName)) {
        memcpy(ac->targetHumiSet.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->targetHumiSet.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("dehumiOffset", pr->regName)) {
        memcpy(ac->dehumiOffset.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->dehumiOffset.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("coolOffset", pr->regName)) {
        memcpy(ac->coolOffset.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->coolOffset.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("heatOffset", pr->regName)) {
        memcpy(ac->heatOffset.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->heatOffset.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("highAlarmTemp", pr->regName)) {
        memcpy(ac->highAlarmTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->highAlarmTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("lowAlarmTemp", pr->regName)) {
        memcpy(ac->lowAlarmTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->lowAlarmTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("maxElecHeatTemp", pr->regName)) {
        memcpy(ac->maxElecHeatTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->maxElecHeatTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("minElecHeatTemp", pr->regName)) {
        memcpy(ac->minElecHeatTemp.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->minElecHeatTemp.flag, pr->flag, sizeof(pr->flag));


    } else if (!strcmp("remoteCtrl", pr->regName)) {
        memcpy(ac->remoteCtrl.opQ, pr->opQ, sizeof(pr->opQ));
        memcpy(ac->remoteCtrl.flag, pr->flag, sizeof(pr->flag));


    }
    
        



}
int element(char *res, union counter *q, char *flag)
{
    char stack_op[50] = {0};//运算符栈
    char temp[50] = {0};
    int j = 0;
    int m = 0;//运算符栈下标
    int i;
    int k;
    int n = 0;//循环后重置flag下标
    int index;
    char *end;
    char type;
    
    
    memset(flag, 0, MAX_ELEMENT);
    
    index = findNextReturnChar(res);
    
    for (i = 0; i < index + 1; i++)//strlen(res)+1确保最后一位数据被写入
    {

        
        if (res[i] == ' ') {
            
            continue;
        }
        
        if (res[i] == '@') {
            i++;
            
            if (res[i] == 'S') {
                type = 4;
                i++;
            } else {
                type = 3;
                
            }
            
            while (checkHexChar(res[i])) {
                temp[j++] = res[i];
                i++;

                
            } 
            temp[j] = '\0';
            log_debug("temp %s\n", temp);
            q[n].addr  = strtol(temp, &end, 16);
            flag[n] = type;
            n++;                
            memset(temp,'\0',sizeof(temp));
            j = 0;
            if (res[i] == '\n') {
                continue;
                /* break; */
            }
            
        }
        
        if (isdigit(res[i]) || res[i] == '.')
        {
            temp[j++] = res[i];
        }
        else if (!isdigit(res[i]) && (strcmp(temp,"")!=0))//将temp中存储的数据转换为float类型写给new_res
        {
            temp[j] = '\0';

            q[n].num = atoi(temp);
            
            flag[n] = 1;
            n++;
            memset(temp,'\0',sizeof(temp));
            j = 0;
        }
        switch (res[i])//判断运算符的优先级决定出栈入栈
        {
        case '(':
            stack_op[m++] = res[i];
            break;
        case '+':
        case '-':
            if (m == 0 || stack_op[m-1] == '(')
            {
                stack_op[m++] = res[i];
            }
            else
            {
                m--;
                q[n].ch = stack_op[m];
                flag[n] = 2;
                n++;
                i--;
            }
            break;
        case '*':
        case '/':
            if (m == 0 || (stack_op[m-1] !='*' && stack_op[m-1] != '/'))
            {

                stack_op[m++] = res[i];
            }
            else
            {
                m--;

                q[n].ch = stack_op[m];
                flag[n] = 2;
                n++;
                i--;
            }
            break;
        case ')':
            m--;
            while(stack_op[m] != '(')
            {
                q[n].ch = stack_op[m];
                flag[n] = 2;
                n++;
                m--;
            }
            stack_op[m] = 0;
            break;
        default:
            break;
        }
    }
    for (k = m-1; k >= 0; k--)//将栈中剩余的运算符全部写入new_res
    {
        q[n].ch = stack_op[k];
        
        flag[n] = 2;
        n++;
    }

    
    return i;
}

inline char getReverseOp(char ch)
{
    char r;
    
    switch (ch) {
    case '+':
        r = '-';
        break;
        
    case '-':
        r = '+';
        break;
        
    case '*':
        r = '/';
        break;
        
    case '/':
        r = '*';
        break;
    default:
        r = 0xFF;
        break;
    }
    
    return r;
    

}

int reverseCount(int value, union counter *new_res, char *flag, AddrTable *t)
{
    int i;
    char c;
    int result;
    
    memset(t, 0, sizeof(AddrTable));
    
    for (i = MAX_ELEMENT; i >= 0; i--) {

        if (flag[i] == 0)
            continue;
        else
            break;
    
    }
    
    if ((flag[i] == 3 || flag[i] == 4) && i == 0) {
    
        
        t->cnt = 1;
        t->addr[0] = new_res[0].addr;
        t->value[0] = value;
        return 0;
        

    } else if (flag[i] == 2) {
        result = value;
        
        for (; i >= 0; i--) {
            if (flag[i] == 2) {
                
                c = getReverseOp(new_res[i].ch);
                switch (c)
                {
                case '+':
                    i--;
                    if (flag[i] == 1)
                        result = result + new_res[i].num;
                
                    break;
                case '-':
                    i--;
                    if (flag[i] == 1)   
                        result = result - new_res[i].num;
                    else if (flag[i] == 3) {
                        
                        /* check if a*65536 + b */
                        if (i == 3 && (flag[i - 1] == 2 && new_res[i - 1].ch == '*') &&
                            (flag[i - 2] == 1 && new_res[i - 2].num == 65536) &&
                            (flag[i - 3] == 3)) {
                            t->cnt = 2;
                            t->addr[0] = new_res[i - 3].addr;
                            t->value[0] = result / 65536;
                            t->addr[1] = new_res[i].addr;
                            t->value[1] = result - t->value[0] * 65536;
                            log_debug("a * 65536 + b parsed");
                            
                            return 0;

                        }
                        
                    
                    }
                
                    break;
                case '*':
                    i--;
                    if (flag[i] == 1)
                        result = result * new_res[i].num;

                
                    break;
                case '/':
                    i--;
                    if (flag[i] == 1)
                    {

                        result = result / new_res[i].num;
                
                    }
                    else
                    {
                        log_error("divid zero!\n");
                        return -1;
                    }
                    break;
                default:
                    break;
                }
            } else if (flag[i] == 3 || flag[i] == 4) {

                t->addr[0] = new_res[i].addr;
                t->value[0] = result;
                t->cnt++;
                
                
            }
            
             
        }
        

    } else {

        log_error("first element not valid\n");
        return -1;
        
    }
    
    return 0;
}

int count(union counter *new_res, char *flag, AddrTable *t)
{
    int x = 0;//数据栈下标
    int stack_num[20] = {0};//数据栈
    int i;
    int index;
    
    
        
    for (i = 0; flag[i] > 0; i++)
    {

        switch (flag[i])//判断new_res中的数据类型
        {
        case 1://将数据写入到栈中
            
            stack_num[x++] = new_res[i].num;
            break;
        case 2://运算

            switch (new_res[i].ch)
            {
            case '+':
                x --;
                stack_num[x-1] += stack_num[x];
                break;
            case '-':
                x --;
                stack_num[x-1] -= stack_num[x];
                break;
            case '*':
                x --;
                stack_num[x-1] *= stack_num[x];
                break;
            case '/':
                x --;
                if (stack_num[x] != 0)
                {
                    stack_num[x-1] /= stack_num[x];
                    
                }
                else
                {
                    log_debug("divid zero error!");
                    return -1;
                }
                break;
            default:
                break;
            }
            break;
            
        case 3:
            if ((index = lookupAddrTable(t, new_res[i].addr)) >= 0) {

                /* log_debug("###count: index %d addr %d value %d", index, new_res[i].addr, t->value[index]); */
                stack_num[x++] = t->value[index];
            }
            
            else {
                
                log_error("lookup addr %x for value fail", new_res[i].addr);
                return -1;
                
            }
            
            break;
        case 4:
            if ((index = lookupAddrTable(t, new_res[i].addr)) >= 0) {

                log_debug("###count: index %d addr %d value %u", index, new_res[i].addr, t->value[index]); 
                stack_num[x++] = (signed short)t->value[index];
            }
            
            else {
                
                log_error("lookup addr %x for value fail", new_res[i].addr);
                return -1;
                
            }
            
            break;
            
            
        default:
            break;
        }
    }
    /* log_debug("result: %d", stack_num[0]); */
    return stack_num[0];
}


char *parseLine(char *buf, ParseResult *pr)
{
    int i, j;
    enum STATE s;
    int len;
    

    if (!pr || !buf)
        return NULL;
    
    i = 0;
    j = 0;
    
    memset(pr->regName, 0, 30);
    s = CHECK_REG_NAME;

    while (s != CHECK_DONE)
    {
        
        if (buf[i] == ' ') {
            
            i++;
            continue;
            
        }

        switch (s) {

        case CHECK_REG_NAME:
            
            if (checkRegName(buf[i])) {
                if (j < 30)
                    pr->regName[j] = buf[i];
                else {
                    log_error("reg name too long");
                    return NULL;

                }
                
                i++;
                j++;
                
            } else if (buf[i] == '=') {
                if (j == 0) {
                    log_error("no reg name before =\n", buf[i]);
                    return NULL;
                    
                }
     
                pr->regName[j] = '\0';
                i++;
                s = CHECK_OP;
            
            } else {

                if (buf[i] == '\n')
                    return buf + i + 1;
                
                log_error("%d unexpected\n", buf[i]);
                return NULL;
     
            
            }
            break;
                
        case CHECK_OP:

            len = element(buf + i, pr->opQ, pr->flag);
            i = i + len;
            s = CHECK_DONE;
            
            break;
        
        default:
            break;
            
            

        }
            
            




    }

    return (buf + i);

    
        
        
        


    


}


int parseConfig(const char *path)
{

    int fd;
    unsigned int size;
    char *buf;
    int len;
    ParseResult pr;
    char *p;

    log_debug("open %s", path);
    
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        
        log_error("open %s fail", path);
        return -1;
    }
    
    size = get_file_size(path);
    if (size < 0) {
        close(fd);
        log_error("get file %s size fail", path);
        return -1;
        
    }
    
    buf = malloc(size + 1);
    if (!buf) {
        close(fd);
        log_error("malloc mem fail");
        return -1;
        
    }
    
    len = read(fd, buf, size);
    if (len != size) {
        close(fd);
        free(buf);
        log_error("read file fail %d", len);
        return -1;
        
    }
    

    buf[size] = '\n';
    p = buf;
    
    while (p) {
        memset(&pr, 0, sizeof(pr));
        p = parseLine(p, &pr);
        if (!p) {
            
            log_error("parse line fail");
            break;
            
        }

        log_debug("parse %s", pr.regName);

        if (!strcmp(path, "config/aircon.conf")) {
            
            parseAirCon(&gAirCon, &pr);

        } else if (!strcmp(path, "config/sensors.conf")) {
            
            parseSensors(&gTempHumiSensor1, &pr);
            parseSensors(&gTempHumiSensor2, &pr);
            parseSensors(&gTempHumiSensor3, &pr);
        } else if (!strcmp(path, "config/thermo.conf"))
            parseThermo(&gTransThermo, &pr);
        else if (!strcmp(path, "config/airconWattMeter.conf"))
            parseAirConWattMeter(&gAirConWhMeter, &pr);
        else if (!strcmp(path, "config/ongridWattMeter.conf"))
            parseOngridWattMeter(&gOnGridWhMeter, &pr);
        else if (!strcmp(path, "config/offgridWattMeter.conf"))
            parseOffgridWattMeter(&gOffGridWhMeter, &pr);
        else if (!strcmp(path, "config/pcs.conf"))
            parsePcsMeter(&gPcs, &pr);
        else if (!strcmp(path, "config/bms.conf"))
            parseBmsMeter(&gBms, &pr);
    }
    
    free(buf);
    close(fd);
    return 0;
    
    

}



inline void MsecSleep(int msec)
{
    struct timeval tv;
    int err;
    
    tv.tv_sec = msec/1000;
    tv.tv_usec = (msec%1000)*1000;
    
    do{
        err = select(0,NULL,NULL,NULL,&tv);
    }while(err < 0 && errno == EINTR);

}

