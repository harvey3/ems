#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

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

#define MODBUS_TCP_PORT 502

//Registers and coils

//For storing exit codes
uint8_t sec, mec;

int ModbusTCPInit(int *sock, ModbusMaster *mstatus, const char *ip)
{

    struct sockaddr_in s_addr;
	//Init master
	modbusMasterInit( mstatus );
    /* load modbus addr from config file */

    
    *sock = socket(AF_INET, SOCK_STREAM, 0);
    if(*sock < 0)
    {
        log_error("socket fail !");
        return -1;
    }

    bzero(&s_addr,sizeof(struct sockaddr_in));
    s_addr.sin_family=AF_INET;
    s_addr.sin_addr.s_addr= inet_addr(ip);
    s_addr.sin_port=htons(MODBUS_TCP_PORT);

    if((connect(*sock, (struct sockaddr *)&s_addr, sizeof(struct sockaddr))) < 0)
    {
        if(errno != EINPROGRESS)
        {
            log_error("connect failed.");
            close(*sock);
            return -1;
        }
        else
        {
            log_error("connect send.");
        }
    }
    else
    {
        log_info(" modbusTCP %s connect success---------------.", ip);
    }
    
    return 0;
    


}

int TcpSendRecvFrame(int sock, ModbusMaster *Ms)
{
    struct timeval tv;
    fd_set rfds;
    int ret;
    int len;
    char *packet;
    int i;
    
    packet = malloc(6 + Ms->request.length - 2);
    if (!packet) {
        log_error("mem alloc failed");
        return -1;
        
    }
    
    memset(packet, 0, 4);
    memcpy(packet + 6, Ms->request.frame, Ms->request.length - 2); /* remove crc feild */
    packet[4] = (Ms->request.length - 2) >> 8;
    packet[5] = (Ms->request.length - 2) & 0xFF;
    /* log_debug("dump modbus tcp request"); */
    /* for (i = 0; i < Ms->request.length + 4; i++) */
    /* { */
    /*     log_debug("%x", packet[i]); */
                    
    /* } */

    ret = write(sock, packet, Ms->request.length + 4);
    if (ret < 0)
    {
        log_error("write uart error");
        return ret;

    }
    free(packet);
            
    FD_ZERO(&rfds);
    FD_SET(sock, &rfds);
    tv.tv_sec=MODBUS_RESP_TIMEOUT_MSEC/1000;
    tv.tv_usec=(MODBUS_RESP_TIMEOUT_MSEC%1000)*1000;
        
    ret = select(sock + 1, &rfds, NULL, NULL, &tv);

    if (ret > 0) {

        if (FD_ISSET(sock, &rfds)) {

            len = read(sock, Ms->response.frame, Ms->predictedResponseLength + 4);
            if (len == Ms->predictedResponseLength + 4) {

                memcpy(Ms->response.frame, Ms->response.frame + 6, Ms->predictedResponseLength - 2);
                Ms->response.length = Ms->predictedResponseLength;
            } else {
                log_error("modbus resp data lost, rx %d, expected %d", len, Ms->predictedResponseLength + 4);
                for (i = 0; i < len; i++)
                {
                    log_debug("%x", Ms->response.frame[i]);
                    
                }
                
                return -1;

            }
            

        }
        


    } else if (ret == 0) {

        log_error("modbus resp timeout");
        return -1;
        
    } else {

        log_error("select on modbusTCP socket failed");
        return -1;
        
    }
    
    return 0;
    

}
int writeModbusTCPData(ModbusMaster *mstatus, int sock, uint16_t address, REG_T *r)
{
    int ret;
    ModbusError err;
    AddrTable t;
    int i;
    
    ret = reverseCount(r->value, r->opQ, r->flag, &t);
    if (ret < 0) {
        log_error("reverse count fail");
        return ret;
    }
    
    for (i = 0; i < t.cnt; i++) {

        //Build frame to write 1 registers
        modbusBuildRequest06(mstatus, address, t.addr[i], t.value[i]);

        ret = TcpSendRecvFrame(sock, mstatus);
        if (ret < 0) {
            log_error("modbus send recv fail");
            return ret;

        }
    
        err = modbusParseResponse(mstatus , 0);

        if (err != MODBUS_ERROR_OK )
            log_error("modbusTCP write resp error");
    }

    return 0;
    
}


int readModbusTCPData(ModbusMaster *mstatus, int sock, uint16_t address, AddrTable *t)
{
    int ret;
    ModbusError err;
    int i, j;
    int start, end;
    
    for (i = 0; i < t->abCnt; i++) {
        log_debug("read modbus addr %d count %d", t->ab[i].addr, t->ab[i].count);
        
        modbusBuildRequest03( mstatus, address, t->ab[i].addr, t->ab[i].count);

        ret = TcpSendRecvFrame(sock, mstatus);
        if (ret < 0) {
            log_error("modbusTCP send recv fail\n");
            return ret;

        }
    
        err = modbusParseResponse(mstatus , 0);
        if (err == MODBUS_ERROR_OK) {
            log_debug("read modbus success");
            start = lookupAddrTable(t, t->ab[i].addr);
            if (i == t->abCnt - 1)
                end = t->cnt;
            else
                end = lookupAddrTable(t, t->ab[i + 1].addr);
            log_debug("start %d end %d", start, end);
            
            for (j = start; j < end; j++) {
                /* log_debug("j = %d", j); */
                t->value[j] = mstatus->data.regs[ADDR2INDEX(t->ab[i].addr, t->addr[j])];
                
            }
            

        } else {

            log_error("parse modbusTCP resp fail");
            return -1;
            
        }
        
        
    }
    return 0;
    


}

void GetPcsStatus(Pcs *pcs)
{

    int ret;

    ret = readModbusTCPData(&pcs->mstatus, pcs->sock, PCS_ADDR, &gPcsT);
    if (ret < 0) {
        log_error("read aircon modbusTCP data fail");
        return;
            
    } else {

        log_debug("########recv pcs resp ok");

        pcs->maxChargePower.value = count(pcs->maxChargePower.opQ, pcs->maxChargePower.flag, &gPcsT);
        pcs->maxDischargePower.value = count(pcs->maxDischargePower.opQ, pcs->maxDischargePower.flag, &gPcsT);
        log_debug("maxChargePower %d maxDischargePower %d", pcs->maxChargePower.value, pcs->maxDischargePower.value);
        
        pcs->busOverVoltValue.value = count(pcs->busOverVoltValue.opQ, pcs->busOverVoltValue.flag, &gPcsT);
        pcs->busLowVoltValue.value = count(pcs->busLowVoltValue.opQ, pcs->busLowVoltValue.flag, &gPcsT);
        pcs->halfBusErrValue.value = count(pcs->halfBusErrValue.opQ, pcs->halfBusErrValue.flag, &gPcsT);
        pcs->dcOverCurrValue.value = count(pcs->dcOverCurrValue.opQ, pcs->dcOverCurrValue.flag, &gPcsT);
        pcs->environOverHeatValue.value = count(pcs->environOverHeatValue.opQ, pcs->environOverHeatValue.flag, &gPcsT);
        pcs->lcOverHeatValue.value = count(pcs->lcOverHeatValue.opQ, pcs->lcOverHeatValue.flag, &gPcsT);
        pcs->igbtOverHeatValue.value = count(pcs->igbtOverHeatValue.opQ, pcs->igbtOverHeatValue.flag, &gPcsT);

        pcs->environLowTempValue.value = count(pcs->environLowTempValue.opQ, pcs->environLowTempValue.flag, &gPcsT);
        pcs->igbtLowTempValue.value = count(pcs->igbtLowTempValue.opQ, pcs->igbtLowTempValue.flag, &gPcsT);
        pcs->ratedPower.value = count(pcs->ratedPower.opQ, pcs->ratedPower.flag, &gPcsT);
        pcs->ratedVolt.value = count(pcs->ratedVolt.opQ, pcs->ratedVolt.flag, &gPcsT);
        pcs->ratedCurr.value = count(pcs->ratedCurr.opQ, pcs->ratedCurr.flag, &gPcsT);
        
        pcs->acBreaker.value = count(pcs->acBreaker.opQ, pcs->acBreaker.flag, &gPcsT);
        pcs->acBreakerTrip.value = count(pcs->acBreakerTrip.opQ, pcs->acBreakerTrip.flag, &gPcsT);
        pcs->dcBreaker.value = count(pcs->dcBreaker.opQ, pcs->dcBreaker.flag, &gPcsT);
        pcs->outputPowerFactor.value = count(pcs->outputPowerFactor.opQ, pcs->outputPowerFactor.flag, &gPcsT);
        pcs->acCurr.value = count(pcs->acCurr.opQ, pcs->acCurr.flag, &gPcsT);
        
        pcs->aPhaseIgbtTemp.value = count(pcs->aPhaseIgbtTemp.opQ, pcs->aPhaseIgbtTemp.flag, &gPcsT);
        pcs->bPhaseIgbtTemp.value = count(pcs->bPhaseIgbtTemp.opQ, pcs->bPhaseIgbtTemp.flag, &gPcsT);
        pcs->cPhaseIgbtTemp.value = count(pcs->cPhaseIgbtTemp.opQ, pcs->cPhaseIgbtTemp.flag, &gPcsT);

        pcs->lcTemp.value = count(pcs->lcTemp.opQ, pcs->lcTemp.flag, &gPcsT);
        pcs->environTemp.value = count(pcs->environTemp.opQ, pcs->environTemp.flag, &gPcsT);

        pcs->lowHalfBusVolt.value = count(pcs->lowHalfBusVolt.opQ, pcs->lowHalfBusVolt.flag, &gPcsT);
        pcs->highHalfBusVolt.value = count(pcs->highHalfBusVolt.opQ, pcs->highHalfBusVolt.flag, &gPcsT);
        pcs->totalOutApparentPower.value = count(pcs->totalOutApparentPower.opQ, pcs->totalOutApparentPower.flag, &gPcsT);
        pcs->maxAllowedApparentPower.value = count(pcs->maxAllowedApparentPower.opQ, pcs->maxAllowedApparentPower.flag, &gPcsT);

        pcs->ctrlMode.value = count(pcs->ctrlMode.opQ, pcs->ctrlMode.flag, &gPcsT);
        pcs->workMode.value = count(pcs->workMode.opQ, pcs->workMode.flag, &gPcsT);
        pcs->runState.value = count(pcs->runState.opQ, pcs->runState.flag, &gPcsT);

        pcs->totalBpVolt.value = count(pcs->totalBpVolt.opQ, pcs->totalBpVolt.flag, &gPcsT);
        pcs->totalBpCurr.value = count(pcs->totalBpVolt.opQ, pcs->totalBpVolt.flag, &gPcsT);

        pcs->maxChargeCurr.value = count(pcs->maxChargeCurr.opQ, pcs->maxChargeCurr.flag, &gPcsT);
        pcs->maxDischargeCurr.value = count(pcs->maxDischargeCurr.opQ, pcs->maxDischargeCurr.flag, &gPcsT);

        pcs->maxSingleBpVolt.value = count(pcs->maxSingleBpVolt.opQ, pcs->maxSingleBpVolt.flag, &gPcsT);
        
        pcs->bpSOC.value = count(pcs->bpSOC.opQ, pcs->bpSOC.flag, &gPcsT);
        
        pcs->abVolt.value = count(pcs->abVolt.opQ, pcs->abVolt.flag, &gPcsT);
        pcs->bcVolt.value = count(pcs->bcVolt.opQ, pcs->bcVolt.flag, &gPcsT);
        pcs->caVolt.value = count(pcs->caVolt.opQ, pcs->caVolt.flag, &gPcsT);

        pcs->aPhaseCurr.value = count(pcs->aPhaseCurr.opQ, pcs->aPhaseCurr.flag, &gPcsT);
        pcs->bPhaseCurr.value = count(pcs->bPhaseCurr.opQ, pcs->bPhaseCurr.flag, &gPcsT);
        pcs->cPhaseCurr.value = count(pcs->cPhaseCurr.opQ, pcs->cPhaseCurr.flag, &gPcsT);

    }

    
    
}

void PcsSetWorkMode(Pcs *pcs, uint16_t mode)
{
    int ret;

    pcs->workMode.value = mode;
    ret = writeModbusTCPData(&pcs->mstatus, pcs->sock, PCS_ADDR, &pcs->workMode);
    
    if (ret < 0) {
            
        log_error("pcs set work modefail");
        return;

    }

    
}

void startPcs(Pcs *pcs)
{

    int ret;

    pcs->startCmd.value = 1;
    
    ret = writeModbusTCPData(&pcs->mstatus, pcs->sock, PCS_ADDR, &pcs->startCmd);
    
    if (ret < 0) {
            
        log_error("pcs set work modefail");
        return;

    }

}


void stopPcs(Pcs *pcs)
{

    
    int ret;
    
    pcs->stopCmd.value = 1;
    
    ret = writeModbusTCPData(&pcs->mstatus, pcs->sock, PCS_ADDR, &pcs->stopCmd);
    
    if (ret < 0) {
            
        log_error("pcs set work modefail");
        return;

    }

}

void SetPcsActPower(Pcs *pcs, int16_t power)
{
    int ret;


    pcs->setActPower.value = power;
    ret = writeModbusTCPData(&pcs->mstatus, pcs->sock, PCS_ADDR, &pcs->setActPower);
    
    if (ret < 0) {
            
        log_error("pcs set work modefail");
        return;

    }


}

/* BMS */
void GetBmsStatus(Bms *bms)
{
    int ret;
    int i;

    /* get bmu status */

    for (i = 0; i < 1; i++) {
        ret = readModbusTCPData(&bms->mstatus, bms->sock, BMS_ADDR, &gBmsT);
        if (ret < 0) {
            log_error("read bms modbusTCP data fail");
            return;
            
        } else {
        
            log_debug("rx bms resp");
            
            bms->mu[i].totalVolt.value = count(bms->mu[i].totalVolt.opQ, bms->mu[i].totalVolt.flag, &gBmsT);
            bms->mu[i].totalCurr.value = count(bms->mu[i].totalCurr.opQ, bms->mu[i].totalCurr.flag, &gBmsT);
            bms->mu[i].SOC.value = count(bms->mu[i].SOC.opQ, bms->mu[i].SOC.flag, &gBmsT);
            bms->mu[i].SOH.value = count(bms->mu[i].SOH.opQ, bms->mu[i].SOH.flag, &gBmsT);
            bms->mu[i].maxSingleBpVolt.value = count(bms->mu[i].maxSingleBpVolt.opQ, bms->mu[i].maxSingleBpVolt.flag, &gBmsT);
            bms->mu[i].minSingleBpVolt.value = count(bms->mu[i].minSingleBpVolt.opQ, bms->mu[i].minSingleBpVolt.flag, &gBmsT);
            bms->mu[i].avgSingleBpVolt.value = count(bms->mu[i].avgSingleBpVolt.opQ, bms->mu[i].avgSingleBpVolt.flag, &gBmsT);

            bms->mu[i].maxSingleBpTemp.value = count(bms->mu[i].maxSingleBpTemp.opQ, bms->mu[i].maxSingleBpTemp.flag, &gBmsT);
            bms->mu[i].minSingleBpTemp.value = count(bms->mu[i].minSingleBpTemp.opQ, bms->mu[i].minSingleBpTemp.flag, &gBmsT);
            bms->mu[i].avgSingleBpTemp.value = count(bms->mu[i].avgSingleBpTemp.opQ, bms->mu[i].avgSingleBpTemp.flag, &gBmsT);
            bms->mu[i].powerOnCmd.value = count(bms->mu[i].powerOnCmd.opQ, bms->mu[i].powerOnCmd.flag, &gBmsT);
            bms->mu[i].maxAllowedChargeCurr.value = count(bms->mu[i].maxAllowedChargeCurr.opQ, bms->mu[i].maxAllowedChargeCurr.flag, &gBmsT);
            bms->mu[i].maxAllowedDischrgCurr.value = count(bms->mu[i].maxAllowedDischrgCurr.opQ, bms->mu[i].maxAllowedDischrgCurr.flag, &gBmsT);



        }
    }
    

}
void BmsPowerDown(Bms *bms)
{

    int ret;
    int i;
    

    for (i = 0; i < 1; i++) {
        
        bms->mu[i].powerOnCmd.value = 2;
        ret = writeModbusTCPData(&bms->mstatus, bms->sock, BMS_ADDR, &bms->mu[i].powerOnCmd);
    
        if (ret < 0) {
            
            log_error("bms set power down fail");
            return;

        }


    }
    

}

void BmsPowerOn(Bms *bms)
{

    int ret;
    int i;

    for (i = 0; i < 1; i++) {
        
        bms->mu[i].powerOnCmd.value = 1;
        ret = writeModbusTCPData(&bms->mstatus, bms->sock, BMS_ADDR, &bms->mu[i].powerOnCmd);
    
        if (ret < 0) {
            
            log_error("bms set power on fail");
            return;

        }


    }
    


}

