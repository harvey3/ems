#ifndef MODBUS_HEADER
#define MODBUS_HEADER



void ModbusInit(ModbusMaster *);
int ModbusTCPInit(int *sock, ModbusMaster *mstatus, const char *ip);

void GetAirConStatus(AirCon *ac);
void AirConSetWorkMode(AirCon *ac, uint16_t mode);
void SetHeatTargetTemp(AirCon *ac, uint16_t temp);
void SetCoolTargetTemp(AirCon *ac, uint16_t temp);
void SetTargetHumi(AirCon *ac, uint16_t humi);
void SetDehumiOffset(AirCon *ac, uint16_t offset);
void SetCoolOffset(AirCon *ac, uint16_t offset);
void SetHeatOffset(AirCon *ac, uint16_t offset);
void SetHighAlarmTemp(AirCon *ac, uint16_t temp);
void SetLowAlarmTemp(AirCon *ac, uint16_t temp);
void SetMaxElecHeatTemp(AirCon *ac, uint16_t temp);
void SetMinElecHeatTemp(AirCon *ac, uint16_t temp);
void SetRemoteCtrlMode(AirCon *ac, uint16_t mode);

void GetAirConWhMeterStatus(AirConWhMeter *acm);

void GetOnGridWhMeterStatus(OnGridWhMeter *whm);
void GetOffGridWhMeterStatus(OffGridWhMeter *whm);
void GetTempSensorStatus(TempHumiSensor *ths);
void GetTransThermoStatus(TransThermo *tth);
void GetBmsStatus(Bms *bms);
void BmsPowerDown(Bms *bms);
void BmsPowerOn(Bms *bms);

void GetPcsStatus(Pcs *pcs);
void stopPcs(Pcs *pcs);
void startPcs(Pcs *pcs);

#define MODBUS_RESP_TIMEOUT_MSEC 1000


    






#endif
