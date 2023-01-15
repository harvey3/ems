#ifndef IO_HEADER
#define IO_HEADER

#define IMX_GPIO_NR(bank, nr)		(((bank) - 1) * 32 + (nr))

#define IO_DELAY_MSEC 400

#define WATER_IN1_MASK 0x1
#define WATER_IN2_MASK 0x2
#define GATE_LOCK1_MASK 0x4
#define GATE_LOCK2_MASK 0x8
#define GATE_LOCK3_MASK 0x10
#define AC_SPD_ALARM_MASK 0x20
#define DC_SPD_ALARM_MASK 0x40
#define EMERG_STOP_MASK 0x80

#define MAIN_BREAKER_MASK 0x1
#define CONVERGE_SWITCH_MASK 0x2
#define FIRE_ALARM2_MASK 0x4
#define FIRE_ALARM1_MASK 0x8
#define FIRE_ERROR_MASK 0x10
#define ONGRID_SWITCH_ON_MASK 0x20
#define ONGRID_SWITCH_OFF_MASK 0x40

#define ONGRID_SWITCH_ON_OUTPUT_MASK 0x1
#define ONGRID_SWITCH_OFF_OUTPUT_MASK 0x2
#define TRANS_FAN_OUTPUT_MASK 0x4

extern char IoRecvBuffer[25];
void *IoThread(void *param);

typedef struct io_data
{
    uint8_t input1;
    uint8_t input2;

    uint8_t output1;
    uint8_t output2;
    struct rqueue *cmdRq;
    



} IoData;


    

extern IoData gIoData;





#endif
