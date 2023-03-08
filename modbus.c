#include <stdio.h>
#include <inttypes.h>

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

//Registers and coils
uint8_t coils[2] = { 0 };
uint16_t regs[32] = { 0 };

//For storing exit codes
uint8_t sec, mec;

//Dump master information
void masterdump( ModbusMaster *mstatus)
{
	int i;
	printf( "==MASTER DUMP==\n" );

	printf( "Received data: slave: %d, addr: %d, count: %d, type: %d\n",
		mstatus->data.address, mstatus->data.index, mstatus->data.count, mstatus->data.type );
	printf( "\t\tvalues:" );
	switch ( mstatus->data.type )
	{
		case MODBUS_HOLDING_REGISTER:
		case MODBUS_INPUT_REGISTER:
			for ( i = 0; i < mstatus->data.count; i++ )
				printf( " %d", mstatus->data.regs[i] );
			break;

		case MODBUS_COIL:
		case MODBUS_DISCRETE_INPUT:
			for ( i = 0; i < mstatus->data.count; i++ )
				printf( " %d", modbusMaskRead( mstatus->data.coils, mstatus->data.length, i ) );
			break;
	}
	printf( "\n" );

	printf( "Request:" );
	for ( i = 0; i < mstatus->request.length; i++ )
		printf( " %d", mstatus->request.frame[i] );
	printf( "\n" );

	printf( "Response:" );
	for ( i = 0; i < mstatus->response.length; i++ )
		printf( " %d", mstatus->response.frame[i] );
	printf( "\n" );

	printf( "Exit code: %d\n\n", mec );
}
void ModbusInit(ModbusMaster *mstatus)
{

	//Init master
	modbusMasterInit( mstatus );
    /* load modbus addr from config file */
    


}
int writeModbusRTUData(ModbusMaster *mstatus, int port, uint16_t address, REG_T *r)
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

        ret = UartSendRecvFrame(port, mstatus);
        if (ret < 0) {
            log_error("modbus send recv fail");
            return ret;

        }
    
        err = modbusParseResponse(mstatus , 1);

        if (err != MODBUS_ERROR_OK )
            log_error("modbusRTU write resp error");
    }

    return 0;
    
}

int readModbusRTUData(ModbusMaster *mstatus, int port, uint16_t address, AddrTable *t)
{
    int ret;
    ModbusError err;
    int i, j;
    int start, end;
    
    for (i = 0; i < t->abCnt; i++) {
        
        
        log_debug("read modbus addr %d count %d", t->ab[i].addr, t->ab[i].count);
        
        modbusBuildRequest03( mstatus, address, t->ab[i].addr, t->ab[i].count);

        ret = UartSendRecvFrame(port, mstatus);
        if (ret < 0) {
            log_error("modbusRTU send recv fail\n");
            return ret;

        }
    
        err = modbusParseResponse(mstatus , 1);
        if (err == MODBUS_ERROR_OK) {
            log_debug("read modbus success");
            start = lookupAddrTable(t, t->ab[i].addr);
            if (i == t->abCnt - 1)
                end = t->cnt;
            else
                end = lookupAddrTable(t, t->ab[i + 1].addr);
                
            for (j = start; j < end; j++) {
                /* log_debug("j = %d", j); */
                t->value[j] = mstatus->data.regs[ADDR2INDEX(t->ab[i].addr, t->addr[j])];
    
                
            }
            

        } else {

            log_error("parse modbusRTU resp fail");
            return -1;
            
        }
     
        
    }
    return 0;
    


}

/* air-conditioner */
void GetAirConStatus(AirCon *ac)
{

    int ret;
    /* int i; */
    
	//Build frame to read 4 registers
    ret = readModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &gAirConT);
#if 0
    log_debug("@@@@@@@@@@@@@@@@@@@@@@@read aircon modbus");
    for (i = 0; i < gAirConT.cnt; i++)
        log_debug("addr %d value %d", gAirConT.addr[i], gAirConT.value[i]);
#endif
    if (ret < 0) {
        log_error("read aircon modbus data fail");
        return;
            
    } else {

        log_debug("***************get aircon resp");

        ac->workMode.value = count(ac->workMode.opQ, ac->workMode.flag, &gAirConT);

        ac->indoorTemp.value = count(ac->indoorTemp.opQ, ac->indoorTemp.flag, &gAirConT);


        ac->outdoorTemp.value = count(ac->outdoorTemp.opQ, ac->outdoorTemp.flag, &gAirConT);

        ac->indoorHumi.value = count(ac->indoorHumi.opQ, ac->indoorHumi.flag, &gAirConT);

        ac->runStat.value = count(ac->runStat.opQ, ac->runStat.flag, &gAirConT);

        
        ac->workMode.value = count(ac->workMode.opQ, ac->workMode.flag, &gAirConT);

        ac->heatTargetTemp.value = count(ac->heatTargetTemp.opQ, ac->heatTargetTemp.flag, &gAirConT);

        ac->coolTargetTemp.value = count(ac->coolTargetTemp.opQ, ac->coolTargetTemp.flag, &gAirConT);
        ac->targetHumi.value = count(ac->targetHumi.opQ, ac->targetHumi.flag, &gAirConT);
        
        ac->errCode.value = count(ac->errCode.opQ, ac->errCode.flag, &gAirConT);
        
        ac->inInterchangeTemp.value = count(ac->inInterchangeTemp.opQ, ac->inInterchangeTemp.flag, &gAirConT);
        ac->outInterchangeTemp.value = count(ac->outInterchangeTemp.opQ, ac->outInterchangeTemp.flag, &gAirConT);
        ac->airoutTemp.value = count(ac->airoutTemp.opQ, ac->airoutTemp.flag, &gAirConT);
        ac->directVolt.value = count(ac->directVolt.opQ, ac->directVolt.flag, &gAirConT);
        ac->powerOn.value = count(ac->powerOn.opQ, ac->powerOn.flag, &gAirConT);
        ac->maxElecHeatTemp.value = count(ac->maxElecHeatTemp.opQ, ac->maxElecHeatTemp.flag, &gAirConT);
        ac->minElecHeatTemp.value = count(ac->minElecHeatTemp.opQ, ac->minElecHeatTemp.flag, &gAirConT);
        ac->remoteCtrl.value = count(ac->remoteCtrl.opQ, ac->remoteCtrl.flag, &gAirConT);
        log_debug("get aircon status %d %d %d %d %d", ac->workMode.value, ac->indoorTemp.value, ac->outdoorTemp.value, ac->indoorHumi.value, ac->runStat.value);
        
    }
    
    
    
}

void AirConSetWorkMode(AirCon *ac, uint16_t mode)
{
    
    int ret;
        
    ac->workModeSet.value = mode;

    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->workModeSet);
    
    if (ret < 0) {
        log_error("set aircon mode fail");
        return;
            
    }

}

void SetHeatTargetTemp(AirCon *ac, uint16_t temp)
{

    int ret;

    ac->heatTargetTempSet.value = temp;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->heatTargetTempSet);
    
    if (ret < 0) {
        log_error("set aircon target heat temp fail");
        return;
            
    }


}

void SetCoolTargetTemp(AirCon *ac, uint16_t temp)
{

    int ret;


    ac->coolTargetTempSet.value = temp;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->coolTargetTempSet);
    
    if (ret < 0) {
        log_error("set aircon target cool temp fail");
        return;
            
    }


}
void SetTargetHumi(AirCon *ac, uint16_t humi)
{

    int ret;


    ac->targetHumiSet.value = humi;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->targetHumiSet);
    
    if (ret < 0) {
        log_error("set aircon target humi fail");
        return;
            
    }


}

void SetDehumiOffset(AirCon *ac, uint16_t offset)
{

    int ret;


    ac->dehumiOffset.value = offset;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->dehumiOffset);
    
    if (ret < 0) {
        log_error("set aircon dehumi offset fail");
        return;
            
    }


}
void SetCoolOffset(AirCon *ac, uint16_t offset)
{

    int ret;

    ac->coolOffset.value = offset;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->coolOffset);
    
    if (ret < 0) {
        log_error("set aircon cool offset fail");
        return;
            
    }


}
void SetHeatOffset(AirCon *ac, uint16_t offset)
{

    int ret;


    ac->heatOffset.value = offset;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->heatOffset);
    
    if (ret < 0) {
        log_error("set aircon heat offset fail");
        return;
            
    }


}
void SetHighAlarmTemp(AirCon *ac, uint16_t temp)
{

    int ret;


    ac->highAlarmTemp.value = temp;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->highAlarmTemp);
    
    if (ret < 0) {
        log_error("set aircon high alarm temp fail");
        return;
            
    }


}
void SetLowAlarmTemp(AirCon *ac, uint16_t temp)
{

    int ret;


    ac->lowAlarmTemp.value = temp;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->lowAlarmTemp);
    
    if (ret < 0) {
        log_error("set aircon low alarm temp fail");
        return;
            
    }


}
void SetMaxElecHeatTemp(AirCon *ac, uint16_t temp)
{

    int ret;


    ac->maxElecHeatTemp.value = temp;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->maxElecHeatTemp);
    
    if (ret < 0) {
        log_error("set aircon max electronic heat temp fail");
        return;
            
    }


}
void SetMinElecHeatTemp(AirCon *ac, uint16_t temp)
{

    int ret;


    ac->minElecHeatTemp.value = temp;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->minElecHeatTemp);
    
    if (ret < 0) {
        log_error("set aircon min electronic heat temp fail");
        return;
            
    }


}
void SetRemoteCtrlMode(AirCon *ac, uint16_t mode)
{

    int ret;


    ac->remoteCtrl.value = mode;
    ret = writeModbusRTUData(&ac->mstatus, AIRCON_PORT, ac->address, &ac->remoteCtrl);
    
    if (ret < 0) {
        log_error("set aircon remote control mode fail");
        return;
            
    }


}


/* aircon wat-hour meter */
void GetAirConWhMeterStatus(AirConWhMeter *acm)
{
        
    int ret;
    
    ret = readModbusRTUData(&acm->mstatus, AIRCON_PORT, AIRCON_WH_METER_ADDR, &gAirConWhMeterT);
    
    if (ret < 0) {
        log_error("read aircon watt meter modbus data fail");
        return;
            
    } else {

        log_debug("###get air con wh resp");

        acm->activePower.value = count(acm->activePower.opQ, acm->activePower.flag, &gAirConWhMeterT);

        log_debug("aircon wh actPower %d", acm->activePower.value);
        
        acm->reactivePower.value = count(acm->reactivePower.opQ, acm->reactivePower.flag, &gAirConWhMeterT);

        log_debug("aircon wh reactPower %d", acm->reactivePower.value);

        acm->apparentPower.value = count(acm->apparentPower.opQ, acm->apparentPower.flag, &gAirConWhMeterT);

        log_debug("aircon wh appaPower %d", acm->apparentPower.value);

        acm->powerFactor.value = count(acm->powerFactor.opQ, acm->powerFactor.flag, &gAirConWhMeterT);

        acm->abVolt.value = count(acm->abVolt.opQ, acm->abVolt.flag, &gAirConWhMeterT);

        log_debug("aircon wh abVolt %d", acm->abVolt.value);

        acm->bcVolt.value = count(acm->bcVolt.opQ, acm->bcVolt.flag, &gAirConWhMeterT);
        
        acm->caVolt.value = count(acm->caVolt.opQ, acm->caVolt.flag, &gAirConWhMeterT);
        
        acm->aPhaseVolt.value = count(acm->aPhaseVolt.opQ, acm->aPhaseVolt.flag, &gAirConWhMeterT);

        acm->bPhaseVolt.value = count(acm->bPhaseVolt.opQ, acm->bPhaseVolt.flag, &gAirConWhMeterT);

        acm->cPhaseVolt.value = count(acm->cPhaseVolt.opQ, acm->cPhaseVolt.flag, &gAirConWhMeterT);
        log_debug("aircon wh aPhaseCurr %d", acm->aPhaseCurr.value);
        acm->aPhaseCurr.value = count(acm->aPhaseCurr.opQ, acm->aPhaseCurr.flag, &gAirConWhMeterT);

        acm->bPhaseCurr.value = count(acm->bPhaseCurr.opQ, acm->bPhaseCurr.flag, &gAirConWhMeterT);
        
        acm->cPhaseCurr.value = count(acm->cPhaseCurr.opQ, acm->cPhaseCurr.flag, &gAirConWhMeterT);
        
        acm->freq.value = count(acm->freq.opQ, acm->freq.flag, &gAirConWhMeterT);

        acm->consumedActPower.value = count(acm->consumedActPower.opQ, acm->consumedActPower.flag, &gAirConWhMeterT);

        log_debug("aircon wh consumedActPower.value %d", acm->consumedActPower.value);        

        acm->producedActPower.value = count(acm->producedActPower.opQ, acm->producedActPower.flag, &gAirConWhMeterT);

        acm->capReactPower.value = count(acm->capReactPower.opQ, acm->capReactPower.flag, &gAirConWhMeterT);
        log_debug("aircon wh capReactPower.value %d", acm->capReactPower.value);        

        acm->inductReactPower.value = count(acm->inductReactPower.opQ, acm->inductReactPower.flag, &gAirConWhMeterT);

        acm->totalActPower.value = count(acm->totalActPower.opQ, acm->totalActPower.flag, &gAirConWhMeterT);
        acm->totalReactPower.value = count(acm->totalReactPower.opQ, acm->totalReactPower.flag, &gAirConWhMeterT);
        log_debug("aircon wh totalActPower.value %d", acm->totalActPower.value);
        log_debug("aircon wh totalReactPower.value %d", acm->totalReactPower.value);        

    }
    



}


/* wat-hour meter */
void GetOnGridWhMeterStatus(OnGridWhMeter *whm)
{
    int ret;


    ret = readModbusRTUData(&whm->mstatus, ONGRID_WH_METER_PORT, whm->address, &gOnGridWhMeterT);
    
    if (ret < 0) {
        log_error("read ongrid watt meter modbus data fail");
        return;
            
    } else {

        log_debug("$$$$$$get ongrid wh resp");
        
        whm->activePower.value = count(whm->activePower.opQ, whm->activePower.flag, &gOnGridWhMeterT);
            
        log_debug("ongrid activePwer %d", whm->activePower.value);

        
        whm->reactivePower.value = count(whm->reactivePower.opQ, whm->reactivePower.flag, &gOnGridWhMeterT);
        
        log_debug("ongrid reactPower %d", whm->reactivePower.value);
        

        whm->apparentPower.value = count(whm->apparentPower.opQ, whm->apparentPower.flag, &gOnGridWhMeterT);

        log_debug("ongrid appaPower %d", whm->apparentPower.value);
        

        whm->powerFactor.value = count(whm->powerFactor.opQ, whm->powerFactor.flag, &gOnGridWhMeterT);
        

        whm->abVolt.value = count(whm->abVolt.opQ, whm->abVolt.flag, &gOnGridWhMeterT);

        log_debug("ongrid abVolt %d", whm->abVolt.value);
        whm->bcVolt.value = count(whm->bcVolt.opQ, whm->bcVolt.flag, &gOnGridWhMeterT);
            
        whm->caVolt.value = count(whm->caVolt.opQ, whm->caVolt.flag, &gOnGridWhMeterT);

        whm->aPhaseVolt.value = count(whm->aPhaseVolt.opQ, whm->aPhaseVolt.flag, &gOnGridWhMeterT);

        whm->bPhaseVolt.value = count(whm->bPhaseVolt.opQ, whm->bPhaseVolt.flag, &gOnGridWhMeterT);
        
        whm->cPhaseVolt.value = count(whm->cPhaseVolt.opQ, whm->cPhaseVolt.flag, &gOnGridWhMeterT);

        whm->aPhaseCurr.value = count(whm->aPhaseCurr.opQ, whm->aPhaseCurr.flag, &gOnGridWhMeterT);
        
        whm->bPhaseCurr.value = count(whm->bPhaseCurr.opQ, whm->bPhaseCurr.flag, &gOnGridWhMeterT);
        
        whm->cPhaseCurr.value = count(whm->cPhaseCurr.opQ, whm->cPhaseCurr.flag, &gOnGridWhMeterT);
        

        whm->freq.value = count(whm->freq.opQ, whm->freq.flag, &gOnGridWhMeterT);


        whm->posActPower.value = count(whm->posActPower.opQ, whm->posActPower.flag, &gOnGridWhMeterT);

        log_debug("ongrid posActPower %d", whm->posActPower.value);

        whm->negActPower.value = count(whm->negActPower.opQ, whm->negActPower.flag, &gOnGridWhMeterT);
        

        whm->posReactPower.value = count(whm->posReactPower.opQ, whm->posReactPower.flag, &gOnGridWhMeterT);

        whm->negReactPower.value = count(whm->negReactPower.opQ, whm->negReactPower.flag, &gOnGridWhMeterT);




    }


}

void GetOffGridWhMeterStatus(OffGridWhMeter *whm)
{

    int ret;
    
    ret = readModbusRTUData(&whm->mstatus, OFFGRID_WH_METER_PORT, whm->address, &gOffGridWhMeterT);
    
    if (ret < 0) {
        log_error("read offgrid watt meter modbus data fail");
        return;
            
    } else {

        log_debug("##############get offgrid data");
        
        whm->activePower.value = count(whm->activePower.opQ, whm->activePower.flag, &gOffGridWhMeterT);
            
        log_debug("offgrid activePwer %d", whm->activePower.value);

        
        whm->reactivePower.value = count(whm->reactivePower.opQ, whm->reactivePower.flag, &gOffGridWhMeterT);
        
        log_debug("offgrid reactPower %d", whm->reactivePower.value);
        

        whm->apparentPower.value = count(whm->apparentPower.opQ, whm->apparentPower.flag, &gOffGridWhMeterT);

        log_debug("offgrid appaPower %d", whm->apparentPower.value);
        

        whm->powerFactor.value = count(whm->powerFactor.opQ, whm->powerFactor.flag, &gOffGridWhMeterT);
        

        whm->abVolt.value = count(whm->abVolt.opQ, whm->abVolt.flag, &gOffGridWhMeterT);

        log_debug("offgrid abVolt %d", whm->abVolt.value);
        whm->bcVolt.value = count(whm->bcVolt.opQ, whm->bcVolt.flag, &gOffGridWhMeterT);
            
        whm->caVolt.value = count(whm->caVolt.opQ, whm->caVolt.flag, &gOffGridWhMeterT);

        whm->aPhaseVolt.value = count(whm->aPhaseVolt.opQ, whm->aPhaseVolt.flag, &gOffGridWhMeterT);

        whm->bPhaseVolt.value = count(whm->bPhaseVolt.opQ, whm->bPhaseVolt.flag, &gOffGridWhMeterT);
        
        whm->cPhaseVolt.value = count(whm->cPhaseVolt.opQ, whm->cPhaseVolt.flag, &gOffGridWhMeterT);

        whm->aPhaseCurr.value = count(whm->aPhaseCurr.opQ, whm->aPhaseCurr.flag, &gOffGridWhMeterT);
        
        whm->bPhaseCurr.value = count(whm->bPhaseCurr.opQ, whm->bPhaseCurr.flag, &gOffGridWhMeterT);
        
        whm->cPhaseCurr.value = count(whm->cPhaseCurr.opQ, whm->cPhaseCurr.flag, &gOffGridWhMeterT);
        

        whm->freq.value = count(whm->freq.opQ, whm->freq.flag, &gOffGridWhMeterT);


        
        whm->consumedActPower.value = count(whm->consumedActPower.opQ, whm->consumedActPower.flag, &gOffGridWhMeterT);

        log_debug("consumedActPower.value %d", whm->consumedActPower.value);        

        whm->producedActPower.value = count(whm->producedActPower.opQ, whm->producedActPower.flag, &gOffGridWhMeterT);

        whm->capReactPower.value = count(whm->capReactPower.opQ, whm->capReactPower.flag, &gOffGridWhMeterT);
        log_debug("capReactPower.value %d", whm->capReactPower.value);        

        whm->inductReactPower.value = count(whm->inductReactPower.opQ, whm->inductReactPower.flag, &gOffGridWhMeterT);

        whm->totalActPower.value = count(whm->totalActPower.opQ, whm->totalActPower.flag, &gOffGridWhMeterT);

        whm->netActPower.value = count(whm->netActPower.opQ, whm->netActPower.flag, &gOffGridWhMeterT);

        whm->totalReactPower.value = count(whm->totalReactPower.opQ, whm->totalReactPower.flag, &gOffGridWhMeterT);

        whm->netReactPower.value = count(whm->netReactPower.opQ, whm->netReactPower.flag, &gOffGridWhMeterT);
    }
    


}
void GetTempSensorStatus(TempHumiSensor *ths)
{
    int ret;
    
    ret = readModbusRTUData(&ths->mstatus, TEMP_SENSOR_PORT, ths->address, &gSensorT);
    if (ret < 0) {
        log_error("read temp sensor modbus data fail");
        return;
            
    } else {

        log_debug("get temp sensor resp %d", ths->address);

        ths->temp.value = count(ths->temp.opQ, ths->temp.flag, &gSensorT);

        log_debug("temp %d", ths->temp.value);

        ths->humi.value = count(ths->humi.opQ, ths->humi.flag, &gSensorT);

        log_debug("humi %d", ths->humi.value);
        

    }
    



}

void GetTransThermoStatus(TransThermo *tth)
{

    int ret;

    ret = readModbusRTUData(&tth->mstatus, TEMP_SENSOR_PORT, tth->address, &gThermoT);
    if (ret < 0) {
        log_error("read thermo modbus data fail");
        return;
            
    } else {

        log_debug("@@@@get transthermo resp");
        
        tth->status.value = count(tth->status.opQ, tth->status.flag, &gThermoT);
        tth->aPhaseTemp.value = count(tth->aPhaseTemp.opQ, tth->aPhaseTemp.flag, &gThermoT);

        log_debug("aPhaseTemp %d", tth->aPhaseTemp.value);
        

        tth->bPhaseTemp.value = count(tth->bPhaseTemp.opQ, tth->bPhaseTemp.flag, &gThermoT);

        log_debug("bPhaseTemp %d", tth->bPhaseTemp);

        tth->cPhaseTemp.value = count(tth->cPhaseTemp.opQ, tth->cPhaseTemp.flag, &gThermoT);

        tth->fanPeriod.value = count(tth->fanPeriod.opQ, tth->fanPeriod.flag, &gThermoT);

    }
    



}
