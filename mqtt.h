#ifndef MQTT_HEADER
#define MQTT_HEADER

#define MQTT_PORT 1883
#define MQTT_HOST "47.104.176.81"
#define MQTT_DELAY_MSEC 400

#define AIRCON_PUB_BUF_LEN 22
#define IO_PUB_BUF_LEN 3
#define SENSORS_PUB_BUF_LEN 4
#define THERMO_PUB_BUF_LEN 5
#define ONGRID_WH_METER_PUB_BUF_LEN 44
#define OFFGRID_WH_METER_PUB_BUF_LEN 60
#define AIRCON_WH_METER_PUB_BUF_LEN 28
    
/* topic */
#define AIRCON_SET_TOPIC "longertek/ems/rcu/aircon/set"
#define IO_SET_TOPIC "longertek/ems/rcu/io/set"

#define AIRCON_REPORT_TOPIC "longertek/ems/lcu/aircon/report"
#define IO_REPORT_TOPIC "longertek/ems/lcu/io/report"
#define SENSORS1_REPORT_TOPIC "longertek/ems/lcu/sensors/1/report"
#define SENSORS2_REPORT_TOPIC "longertek/ems/lcu/sensors/2/report"
#define SENSORS3_REPORT_TOPIC "longertek/ems/lcu/sensors/3/report"
#define THERMO_REPORT_TOPIC "longertek/ems/lcu/thermo/1/report"
#define ONGRID_WH_METER_REPORT_TOPIC "longertek/ems/lcu/wattmeter/ongrid/report"
#define OFFGRID_WH_METER_REPORT_TOPIC "longertek/ems/lcu/wattmeter/offgrid/report"
#define AIRCON_WH_METER_REPORT_TOPIC "longertek/ems/lcu/wattmeter/aircon/report"

typedef struct aircon_mqtt_set_packet 
{
    uint8_t WorkMode;
    uint8_t CoolTargetTemp;
    uint8_t HeatTargetTemp;
    uint8_t TargetHumi;
    uint8_t DehumiOffset;
    uint8_t CoolOffset;
    uint8_t HeatOffset;
    uint8_t HighAlarmTemp;
    uint8_t LowAlarmTemp;
    uint8_t maxElecHeatTemp;
    uint8_t minElecHeatTemp;
    uint8_t remoteCtrl;


} __attribute__((packed)) AirConMqttSetP;

typedef struct io_mqtt_set_packet 
{
    uint8_t setFlag1;
    uint8_t setFlag2;
    uint8_t setFlag3;

} __attribute__((packed)) IoMqttSetP;


void* MqttThread(void *param);





    






#endif
