/*******************************************************************************
 * Copyright (c) 2012, 2016 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution. 
 *
 * The Eclipse Public License is available at 
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at 
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *    Ian Craggs - update MQTTClient function names
 *******************************************************************************/

/*
 
 stdout subscriber
 
 compulsory parameters:
 
  topic to subscribe to
 
 defaulted parameters:
 
	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber
	
	--userid none
	--password none

 for example:

    stdoutsub topic/of/interest --host iot.eclipse.org

*/
#include <stdio.h>
#include <memory.h>


#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#include <lightmodbus/lightmodbus.h>
#include <lightmodbus/master.h>
#include <lightmodbus/slave.h>

#include "MQTTLinux.h"
#include "MQTTClient.h"
#include "RegAddr.h"
#include "mqtt.h"
#include "WattHourMeter.h"
#include "TempSensor.h"
#include "IO.h"
#include "AirCon.h"
#include "Utils.h"
#include "log.h"

volatile int toStop = 0;

AirConMqttSetP gAirConMqttSet;
IoMqttSetP     gIoMqttSet;
static char airconPubBuf[AIRCON_PUB_BUF_LEN];
static char ioPubBuf[IO_PUB_BUF_LEN];
static char sensorsPubBuf[SENSORS_PUB_BUF_LEN];
static char thermoPubBuf[THERMO_PUB_BUF_LEN];
static char ongridWhMeterPubBuf[ONGRID_WH_METER_PUB_BUF_LEN];
static char offgridWhMeterPubBuf[OFFGRID_WH_METER_PUB_BUF_LEN];
static char airconWhMeterPubBuf[AIRCON_WH_METER_PUB_BUF_LEN];

static unsigned char mqtt_send_buf[1024];
static unsigned char mqtt_read_buf[1024];


static void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}


struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts =
{
	(char*)"stdout-subscriber", 0, (char*)"\n", QOS2, NULL, NULL, (char*)"localhost", 1883, 0
};



void airconMessageArrived(MessageData* md)
{
    AirConMqttSetP lastSet;
    CMDT cmd;
    
	MQTTMessage* message = md->message;
    
    log_debug("###########%s message arrived", md->topicName->lenstring.data);

    lastSet = gAirConMqttSet;
    memcpy(&gAirConMqttSet, message->payload, message->payloadlen);

    if (gAirConMqttSet.WorkMode != lastSet.WorkMode) {
        log_debug("########set aircon mode %x last %x", gAirConMqttSet.WorkMode, lastSet.WorkMode);
        
        cmd.cmd = AIRCON_SET_WORKMODE_CMD;
        cmd.value = gAirConMqttSet.WorkMode;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.CoolTargetTemp != lastSet.CoolTargetTemp) {
        log_debug("########set aircon coolTargetTemp %d last %d", gAirConMqttSet.CoolTargetTemp, lastSet.CoolTargetTemp);
        cmd.cmd = AIRCON_SET_COOL_TEMP_CMD;
        cmd.value = gAirConMqttSet.CoolTargetTemp;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.HeatTargetTemp != lastSet.HeatTargetTemp) {
        log_debug("########set aircon heatTargetTemp %d last %d", gAirConMqttSet.HeatTargetTemp, lastSet.HeatTargetTemp);
        cmd.cmd = AIRCON_SET_HEAT_TEMP_CMD;
        cmd.value = gAirConMqttSet.HeatTargetTemp;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    
    if (gAirConMqttSet.TargetHumi != lastSet.TargetHumi) {
        log_debug("########set aircon TargetHumi %d last %d", gAirConMqttSet.TargetHumi, lastSet.TargetHumi);
        cmd.cmd = AIRCON_SET_HUMI_CMD;
        cmd.value = gAirConMqttSet.TargetHumi;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }


    if (gAirConMqttSet.DehumiOffset != lastSet.DehumiOffset) {
        log_debug("########set aircon DeHumiOffset %d last %d", gAirConMqttSet.DehumiOffset, lastSet.DehumiOffset);
        cmd.cmd = AIRCON_SET_DEHUMI_OFFSET_CMD;
        cmd.value = gAirConMqttSet.DehumiOffset;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }
    
    if (gAirConMqttSet.CoolOffset != lastSet.CoolOffset) {
        log_debug("########set aircon cooloffset %d last %d", gAirConMqttSet.CoolOffset, lastSet.CoolOffset);
        cmd.cmd = AIRCON_SET_COOL_OFFSET_CMD;
        cmd.value = gAirConMqttSet.CoolOffset;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.HeatOffset != lastSet.HeatOffset) {
        log_debug("########set aircon heatoffset %d last %d", gAirConMqttSet.HeatOffset, lastSet.HeatOffset);
        cmd.cmd = AIRCON_SET_HEAT_OFFSET_CMD;
        cmd.value = gAirConMqttSet.HeatOffset;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.HighAlarmTemp != lastSet.HighAlarmTemp) {
        log_debug("########set aircon high alarm temp %d last %d", gAirConMqttSet.HighAlarmTemp, lastSet.HighAlarmTemp);
        cmd.cmd = AIRCON_SET_HIGH_ALARM_TEMP_CMD;
        cmd.value = gAirConMqttSet.HighAlarmTemp;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }
    if (gAirConMqttSet.LowAlarmTemp != lastSet.LowAlarmTemp) {
        log_debug("########set aircon low alarm temp %d last %d", gAirConMqttSet.LowAlarmTemp, lastSet.LowAlarmTemp);
        cmd.cmd = AIRCON_SET_LOW_ALARM_TEMP_CMD;
        cmd.value = gAirConMqttSet.LowAlarmTemp;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.maxElecHeatTemp != lastSet.maxElecHeatTemp) {
        log_debug("########set aircon maxElecHeatTemp %d last %d", gAirConMqttSet.maxElecHeatTemp, lastSet.maxElecHeatTemp);
        cmd.cmd = AIRCON_SET_MAX_ELECHEAT_TEMP_CMD;
        cmd.value = gAirConMqttSet.maxElecHeatTemp;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.minElecHeatTemp != lastSet.minElecHeatTemp) {
        log_debug("########set aircon minElecHeatTemp %d last %d", gAirConMqttSet.minElecHeatTemp, lastSet.minElecHeatTemp);
        cmd.cmd = AIRCON_SET_MIN_ELECHEAT_TEMP_CMD;
        cmd.value = gAirConMqttSet.minElecHeatTemp;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }

    if (gAirConMqttSet.remoteCtrl != lastSet.remoteCtrl) {
        log_debug("########set aircon remoteCtrl %d last %d", gAirConMqttSet.remoteCtrl, lastSet.remoteCtrl);
        cmd.cmd = AIRCON_SET_REMOTE_CTRL_CMD;
        cmd.value = gAirConMqttSet.remoteCtrl;
        rqueue_write(gAirCon.cmdRq, &cmd);
    }


}

void ioMessageArrived(MessageData* md)
{
    IoMqttSetP lastSet;
	MQTTMessage* message = md->message;
    CMDT cmd;
    
    log_debug("%s message arrived", md->topicName->lenstring.data);
    lastSet = gIoMqttSet;
    memcpy(&gIoMqttSet, message->payload, message->payloadlen);

    if ((gIoMqttSet.setFlag1 & 0x3) != (lastSet.setFlag1 & 0x3)) {

        if ((gIoMqttSet.setFlag1 & 0x3) == 1)
            cmd.cmd = IO_SET_SWITCH_ON_CMD;
        else if ((gIoMqttSet.setFlag1 & 0x3) == 2)
            cmd.cmd = IO_SET_SWITCH_OFF_CMD;
        else
            return;
        
        rqueue_write(gIoData.cmdRq, &cmd);
    }
    log_debug("@@@@@@@set fan %x", gIoMqttSet.setFlag1);
    
    if ((gIoMqttSet.setFlag1 & 0x4) != (lastSet.setFlag1 & 0x4)) {

        cmd.cmd = IO_SET_FAN_CMD;
        
        cmd.value = (gIoMqttSet.setFlag1 & 0x4) >> 2;
        log_debug("queue IO cmd %d value %d", cmd.cmd, cmd.value);
        
        rqueue_write(gIoData.cmdRq, &cmd);
    }
    
    
}

void pubMqttMessage(MQTTClient* c, int qos, char *topic, char *payload, int len)
{
    MQTTMessage pubmsg;
    int rc;
    
    memset(&pubmsg, '\0', sizeof(pubmsg));
	pubmsg.payload = payload;
	pubmsg.payloadlen = len;
	pubmsg.qos = qos;
	pubmsg.retained = 0;
    pubmsg.dup = 0;
    
    rc = MQTTPublish(c, topic, &pubmsg);
    if (rc < 0)
        log_error("pub %s topic msg failed %d", topic, rc);
    else
        log_debug("pub %s msg succeed", topic);
    
    
    

}

void pubAirconReport(MQTTClient *c, AirCon *ac)
{

    log_debug("###aircon temp %d %d %d", ac->indoorTemp.value, ac->outdoorTemp.value, ac->indoorHumi.value);
    airconPubBuf[0] = ac->indoorTemp.value;
    airconPubBuf[1] = ac->outdoorTemp.value;
    airconPubBuf[2] = ac->indoorHumi.value;
    log_debug("###aircon runStat %x", ac->runStat.value);
    airconPubBuf[3] = ac->runStat.value >> 8;
    airconPubBuf[4] = ac->runStat.value & 0xFF;
    airconPubBuf[5] = ac->workMode.value;
    airconPubBuf[6] = ac->heatTargetTemp.value;
    log_debug("###aircon heatTargetTemp %d", ac->heatTargetTemp.value);
    airconPubBuf[7] = ac->coolTargetTemp.value;
    airconPubBuf[8] = ac->targetHumi.value;
    airconPubBuf[9] = ac->errCode.value;
    airconPubBuf[10] = ac->inInterchangeTemp.value;
    airconPubBuf[11] = ac->outInterchangeTemp.value;
    airconPubBuf[12] = ac->airoutTemp.value;
    log_debug("###aircon directVolt %d", ac->directVolt.value);
    airconPubBuf[18] = ac->directVolt.value;
    airconPubBuf[19] = ac->maxElecHeatTemp.value;
    airconPubBuf[20] = ac->minElecHeatTemp.value;
    airconPubBuf[21] = ac->remoteCtrl.value;
    
    
    pubMqttMessage(c, QOS0, AIRCON_REPORT_TOPIC, airconPubBuf, AIRCON_PUB_BUF_LEN);

}
void pubIoReport(MQTTClient *c, char *io)
{
    
    ioPubBuf[0] = gIoData.input1;
    ioPubBuf[1] = gIoData.input2;
    log_debug("#######ioPubBuf %x %x", ioPubBuf[0], ioPubBuf[1]);

    
    pubMqttMessage(c, QOS0, IO_REPORT_TOPIC, ioPubBuf, IO_PUB_BUF_LEN);

}

void pubSensorsReport(MQTTClient *c, TempHumiSensor *ths)
{
    char *topic;
    
    sensorsPubBuf[0] = ths->temp.value;
    sensorsPubBuf[1] = ths->humi.value;
    if (ths->id == 1)
        topic = SENSORS1_REPORT_TOPIC;
    else if (ths->id == 2)
        topic = SENSORS2_REPORT_TOPIC;
    else
        topic = SENSORS3_REPORT_TOPIC;
    
    pubMqttMessage(c, QOS0, topic, sensorsPubBuf, SENSORS_PUB_BUF_LEN);

}

void pubThermoReport(MQTTClient *c, TransThermo *tth)
{
    
    thermoPubBuf[0] = tth->status.value;
    log_debug("#####thermo status %x", tth->status.value);
    thermoPubBuf[0] = 3;
    
    thermoPubBuf[1] = tth->aPhaseTemp.value;
    thermoPubBuf[2] = tth->bPhaseTemp.value;
    thermoPubBuf[3] = tth->cPhaseTemp.value;
    thermoPubBuf[4] = tth->fanPeriod.value;
    
    pubMqttMessage(c, QOS0, THERMO_REPORT_TOPIC, thermoPubBuf, THERMO_PUB_BUF_LEN);

}
void pubAirconWhMeterReport(MQTTClient *c, AirConWhMeter *whm)
{
    
    airconWhMeterPubBuf[0] = whm->activePower.value >> 8;
    airconWhMeterPubBuf[1] = whm->activePower.value & 0xFF;
    airconWhMeterPubBuf[2] = whm->reactivePower.value >> 8;
    airconWhMeterPubBuf[3] = whm->reactivePower.value & 0xFF;
    airconWhMeterPubBuf[4] = whm->apparentPower.value >> 8;
    airconWhMeterPubBuf[5] = whm->apparentPower.value & 0xFF;
    log_debug("###aircon whm actPower %d", whm->activePower.value);
    log_debug("###aircon whm reactPower %d", whm->reactivePower.value);
    log_debug("###aircon whm powerFactor %d", whm->powerFactor.value);
    airconWhMeterPubBuf[6] = whm->powerFactor.value >> 8;
    airconWhMeterPubBuf[7] = whm->powerFactor.value & 0xFF;
    log_debug("#######abVolt %d\n", whm->abVolt.value);
    airconWhMeterPubBuf[8] = whm->abVolt.value >> 8;
    airconWhMeterPubBuf[9] = whm->abVolt.value & 0xFF;
    airconWhMeterPubBuf[10] = whm->bcVolt.value >> 8;
    airconWhMeterPubBuf[11] = whm->bcVolt.value & 0xFF;
    airconWhMeterPubBuf[12] = whm->caVolt.value >> 8;
    airconWhMeterPubBuf[13] = whm->caVolt.value & 0xFF;
    log_debug("#######aPhaseVolt %d\n", whm->aPhaseVolt.value);
    airconWhMeterPubBuf[14] = whm->aPhaseVolt.value >> 8;
    airconWhMeterPubBuf[15] = whm->aPhaseVolt.value & 0xFF;
    airconWhMeterPubBuf[16] = whm->bPhaseVolt.value >> 8;
    airconWhMeterPubBuf[17] = whm->bPhaseVolt.value & 0xFF;
    airconWhMeterPubBuf[18] = whm->cPhaseVolt.value >> 8;
    airconWhMeterPubBuf[19] = whm->cPhaseVolt.value & 0xFF;
    log_debug("#######aPhaseCurr %d\n", whm->aPhaseCurr.value);
    
    airconWhMeterPubBuf[20] = whm->aPhaseCurr.value >> 8;
    airconWhMeterPubBuf[21] = whm->aPhaseCurr.value & 0xFF;
    airconWhMeterPubBuf[22] = whm->bPhaseCurr.value >> 8;
    airconWhMeterPubBuf[23] = whm->bPhaseCurr.value & 0xFF;
    airconWhMeterPubBuf[24] = whm->cPhaseCurr.value >> 8;
    airconWhMeterPubBuf[25] = whm->cPhaseCurr.value & 0xFF;

    airconWhMeterPubBuf[26] = whm->freq.value >> 8;
    airconWhMeterPubBuf[27] = whm->freq.value & 0xFF;

    
    pubMqttMessage(c, QOS0, AIRCON_WH_METER_REPORT_TOPIC, airconWhMeterPubBuf, AIRCON_WH_METER_PUB_BUF_LEN);

}

void pubOngridWhMeterReport(MQTTClient *c, OnGridWhMeter *whm)
{
    
    ongridWhMeterPubBuf[0] = whm->activePower.value >> 8;
    ongridWhMeterPubBuf[1] = whm->activePower.value & 0xFF;
    log_debug("#######activePower %d\n", whm->activePower.value);
    ongridWhMeterPubBuf[2] = whm->reactivePower.value >> 8;
    ongridWhMeterPubBuf[3] = whm->reactivePower.value & 0xFF;
    log_debug("#######reactivePower %d\n", whm->reactivePower.value);
    ongridWhMeterPubBuf[4] = whm->apparentPower.value >> 8;
    ongridWhMeterPubBuf[5] = whm->apparentPower.value & 0xFF;
    log_debug("#######appaPower %d\n", whm->apparentPower.value);
    ongridWhMeterPubBuf[6] = whm->powerFactor.value >> 8;
    ongridWhMeterPubBuf[7] = whm->powerFactor.value & 0xFF;
    log_debug("#######powerFactor %d\n", whm->powerFactor.value);
    ongridWhMeterPubBuf[8] = whm->abVolt.value >> 8;
    ongridWhMeterPubBuf[9] = whm->abVolt.value & 0xFF;
    log_debug("#######abVolt %d\n", whm->abVolt.value);
    ongridWhMeterPubBuf[10] = whm->bcVolt.value >> 8;
    ongridWhMeterPubBuf[11] = whm->bcVolt.value & 0xFF;
    ongridWhMeterPubBuf[12] = whm->caVolt.value >> 8;
    ongridWhMeterPubBuf[13] = whm->caVolt.value & 0xFF;
    log_debug("#######caVolt %d\n", whm->caVolt.value);
    log_debug("#######aPhaseVolt %d\n", whm->aPhaseVolt.value);
    ongridWhMeterPubBuf[14] = whm->aPhaseVolt.value >> 8;
    ongridWhMeterPubBuf[15] = whm->aPhaseVolt.value & 0xFF;
    ongridWhMeterPubBuf[16] = whm->bPhaseVolt.value >> 8;
    ongridWhMeterPubBuf[17] = whm->bPhaseVolt.value & 0xFF;
    ongridWhMeterPubBuf[18] = whm->cPhaseVolt.value >> 8;
    ongridWhMeterPubBuf[19] = whm->cPhaseVolt.value & 0xFF;
    log_debug("#######aPhaseCurr %d\n", whm->aPhaseCurr.value);
    ongridWhMeterPubBuf[20] = whm->aPhaseCurr.value >> 8;
    ongridWhMeterPubBuf[21] = whm->aPhaseCurr.value & 0xFF;
    ongridWhMeterPubBuf[22] = whm->bPhaseCurr.value >> 8;
    ongridWhMeterPubBuf[23] = whm->bPhaseCurr.value & 0xFF;
    ongridWhMeterPubBuf[24] = whm->cPhaseCurr.value >> 8;
    ongridWhMeterPubBuf[25] = whm->cPhaseCurr.value & 0xFF;
    
    log_debug("#######freq %d", whm->freq.value);
    ongridWhMeterPubBuf[26] = whm->freq.value >> 8;
    ongridWhMeterPubBuf[27] = whm->freq.value & 0xFF;
    
    log_debug("#######posActPower %d", whm->posActPower.value);
    ongridWhMeterPubBuf[28] = whm->posActPower.value >> 24;
    ongridWhMeterPubBuf[29] = (whm->posActPower.value & 0xFF0000) >> 16;
    ongridWhMeterPubBuf[30] = (whm->posActPower.value & 0xFF00) >> 8;
    ongridWhMeterPubBuf[31] = (whm->posActPower.value & 0xFF);

    ongridWhMeterPubBuf[32] = whm->negActPower.value >> 24;
    ongridWhMeterPubBuf[33] = (whm->negActPower.value & 0xFF0000) >> 16;
    ongridWhMeterPubBuf[34] = (whm->negActPower.value & 0xFF00) >> 8;
    ongridWhMeterPubBuf[35] = (whm->negActPower.value & 0xFF);
    log_debug("#######negActPower %d", whm->negActPower.value);
    ongridWhMeterPubBuf[36] = whm->posReactPower.value >> 24;
    ongridWhMeterPubBuf[37] = (whm->posReactPower.value & 0xFF0000) >> 16;
    ongridWhMeterPubBuf[38] = (whm->posReactPower.value & 0xFF00) >> 8;
    ongridWhMeterPubBuf[39] = (whm->posReactPower.value & 0xFF);
    log_debug("#######posReactPower %d", whm->posReactPower.value);
    ongridWhMeterPubBuf[40] = whm->negReactPower.value >> 24;
    ongridWhMeterPubBuf[41] = (whm->negReactPower.value & 0xFF0000) >> 16;
    ongridWhMeterPubBuf[42] = (whm->negReactPower.value & 0xFF00) >> 8;
    ongridWhMeterPubBuf[43] = (whm->negReactPower.value & 0xFF);
    log_debug("#######negReactPower %d", whm->negReactPower.value);

    
    pubMqttMessage(c, QOS0, ONGRID_WH_METER_REPORT_TOPIC, ongridWhMeterPubBuf, ONGRID_WH_METER_PUB_BUF_LEN);

}

void pubOffgridWhMeterReport(MQTTClient *c, OffGridWhMeter *whm)
{
    
    offgridWhMeterPubBuf[0] = whm->activePower.value >> 8;
    offgridWhMeterPubBuf[1] = whm->activePower.value & 0xFF;
    offgridWhMeterPubBuf[2] = whm->reactivePower.value >> 8;
    offgridWhMeterPubBuf[3] = whm->reactivePower.value & 0xFF;
    offgridWhMeterPubBuf[4] = whm->apparentPower.value >> 8;
    offgridWhMeterPubBuf[5] = whm->apparentPower.value & 0xFF;
    offgridWhMeterPubBuf[6] = whm->powerFactor.value >> 8;
    offgridWhMeterPubBuf[7] = whm->powerFactor.value & 0xFF;

    offgridWhMeterPubBuf[8] = whm->abVolt.value >> 8;
    offgridWhMeterPubBuf[9] = whm->abVolt.value & 0xFF;
    offgridWhMeterPubBuf[10] = whm->bcVolt.value >> 8;
    offgridWhMeterPubBuf[11] = whm->bcVolt.value & 0xFF;
    offgridWhMeterPubBuf[12] = whm->caVolt.value >> 8;
    offgridWhMeterPubBuf[13] = whm->caVolt.value & 0xFF;

    offgridWhMeterPubBuf[14] = whm->aPhaseVolt.value >> 8;
    offgridWhMeterPubBuf[15] = whm->aPhaseVolt.value & 0xFF;
    offgridWhMeterPubBuf[16] = whm->bPhaseVolt.value >> 8;
    offgridWhMeterPubBuf[17] = whm->bPhaseVolt.value & 0xFF;
    offgridWhMeterPubBuf[18] = whm->cPhaseVolt.value >> 8;
    offgridWhMeterPubBuf[19] = whm->cPhaseVolt.value & 0xFF;
    log_debug("#######offgrid wh aPhaseVolt %d", whm->aPhaseVolt.value);
    offgridWhMeterPubBuf[20] = whm->aPhaseCurr.value >> 8;
    offgridWhMeterPubBuf[21] = whm->aPhaseCurr.value & 0xFF;
    offgridWhMeterPubBuf[22] = whm->bPhaseCurr.value >> 8;
    offgridWhMeterPubBuf[23] = whm->bPhaseCurr.value & 0xFF;
    offgridWhMeterPubBuf[24] = whm->cPhaseCurr.value >> 8;
    offgridWhMeterPubBuf[25] = whm->cPhaseCurr.value & 0xFF;
    log_debug("#######offgrid wh aPhaseCurr %d", whm->aPhaseCurr.value);
    offgridWhMeterPubBuf[26] = whm->freq.value >> 8;
    offgridWhMeterPubBuf[27] = whm->freq.value & 0xFF;

    offgridWhMeterPubBuf[28] = whm->consumedActPower.value >> 24;
    offgridWhMeterPubBuf[29] = (whm->consumedActPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[30] = (whm->consumedActPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[31] = (whm->consumedActPower.value & 0xFF);
    log_debug("#######offgrid wh consumedActPower %d", whm->consumedActPower.value);
    offgridWhMeterPubBuf[32] = whm->producedActPower.value >> 24;
    offgridWhMeterPubBuf[33] = (whm->producedActPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[34] = (whm->producedActPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[35] = (whm->producedActPower.value & 0xFF);
    log_debug("#######offgrid wh producedActPower %d", whm->producedActPower.value);
    offgridWhMeterPubBuf[36] = whm->capReactPower.value >> 24;
    offgridWhMeterPubBuf[37] = (whm->capReactPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[38] = (whm->capReactPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[39] = (whm->capReactPower.value & 0xFF);
    log_debug("#######offgrid wh capReactPower %d", whm->capReactPower.value);
    offgridWhMeterPubBuf[40] = whm->inductReactPower.value >> 24;
    offgridWhMeterPubBuf[41] = (whm->inductReactPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[42] = (whm->inductReactPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[43] = (whm->inductReactPower.value & 0xFF);

    offgridWhMeterPubBuf[44] = whm->totalActPower.value >> 24;
    offgridWhMeterPubBuf[45] = (whm->totalActPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[46] = (whm->totalActPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[47] = (whm->totalActPower.value & 0xFF);
    log_debug("#######offgrid wh totalActPower %d", whm->totalActPower.value);
    offgridWhMeterPubBuf[48] = whm->netActPower.value >> 24;
    offgridWhMeterPubBuf[49] = (whm->netActPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[50] = (whm->netActPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[51] = (whm->netActPower.value & 0xFF);

    offgridWhMeterPubBuf[52] = whm->totalReactPower.value >> 24;
    offgridWhMeterPubBuf[53] = (whm->totalReactPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[54] = (whm->totalReactPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[55] = (whm->totalReactPower.value & 0xFF);
    log_debug("#######offgrid wh totalReactPower %d", whm->totalReactPower.value);
    offgridWhMeterPubBuf[56] = whm->netReactPower.value >> 24;
    offgridWhMeterPubBuf[57] = (whm->netReactPower.value & 0xFF0000) >> 16;
    offgridWhMeterPubBuf[58] = (whm->netReactPower.value & 0xFF00) >> 8;
    offgridWhMeterPubBuf[59] = (whm->netReactPower.value & 0xFF);
    log_debug("#######offgrid wh netReactPower %d", whm->netReactPower.value);
    pubMqttMessage(c, QOS0, OFFGRID_WH_METER_REPORT_TOPIC, offgridWhMeterPubBuf, OFFGRID_WH_METER_PUB_BUF_LEN);

}

static void sighandler(int signo)
{
    if(signo == SIGPIPE){
        log_error("SIGPIPE received");
    }
}



void* MqttThread(void *param)
{
	int rc = 0;
	Network n;
	MQTTClient c;
    struct sigaction action;
    
    action.sa_handler = sighandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    
	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);
    
    sigaction(SIGPIPE, &action, NULL);

	NetworkInit(&n);
	NetworkConnect(&n, MQTT_HOST, MQTT_PORT);
	MQTTClientInit(&c, &n, 1000, mqtt_send_buf, 1024, mqtt_read_buf, 1024);
 
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = "lcu";
	data.username.cstring = "admin";
	data.password.cstring = "public";

	data.keepAliveInterval = 10;
	data.cleansession = 1;
	log_info("Connecting to %s %d\n", MQTT_HOST, MQTT_PORT);
#if 1
	rc = MQTTConnect(&c, &data);
	log_info("Connected %d\n", rc);
    
    log_info("Subscribing to %s\n", AIRCON_SET_TOPIC);
	rc = MQTTSubscribe(&c, AIRCON_SET_TOPIC, QOS0, airconMessageArrived);
	log_info("Subscribed %d\n", rc);

    log_info("Subscribing to %s\n", IO_SET_TOPIC);
	rc = MQTTSubscribe(&c, IO_SET_TOPIC, QOS0, ioMessageArrived);
	log_info("Subscribed %d\n", rc);
#endif
	while (!toStop)
	{
#if 1
        pubAirconReport(&c, &gAirCon);
        pubAirconWhMeterReport(&c, &gAirConWhMeter);
        pubIoReport(&c, IoRecvBuffer);
        
        pubSensorsReport(&c, &gTempHumiSensor1);
        pubSensorsReport(&c, &gTempHumiSensor2);
        pubSensorsReport(&c, &gTempHumiSensor3);

        pubThermoReport(&c, &gTransThermo);

        pubOffgridWhMeterReport(&c, &gOffGridWhMeter);
        pubOngridWhMeterReport(&c, &gOnGridWhMeter);
        
		MQTTYield(&c, 1000);
        
#endif

        
        
	}
	
	log_info("Stopping\n");

	MQTTDisconnect(&c);
	NetworkDisconnect(&n);

	return 0;
}


