#ifndef UTILS_HEADER
#define UTILS_HEADER

#define RQUEUE_SIZE 16

enum CmdId {
    AIRCON_SET_WORKMODE_CMD = 1,
    AIRCON_SET_COOL_TEMP_CMD,
    AIRCON_SET_HEAT_TEMP_CMD,
    AIRCON_SET_HUMI_CMD,
    AIRCON_SET_DEHUMI_OFFSET_CMD,
    AIRCON_SET_COOL_OFFSET_CMD,
    AIRCON_SET_HEAT_OFFSET_CMD,
    AIRCON_SET_HIGH_ALARM_TEMP_CMD,
    AIRCON_SET_LOW_ALARM_TEMP_CMD,
    AIRCON_SET_MAX_ELECHEAT_TEMP_CMD,
    AIRCON_SET_MIN_ELECHEAT_TEMP_CMD,
    AIRCON_SET_REMOTE_CTRL_CMD,
    IO_SET_SWITCH_ON_CMD,
    IO_SET_SWITCH_OFF_CMD,
    IO_SET_FAN_CMD,
};

    
typedef struct command 
{
    enum CmdId cmd;
    
    int value;
    

} CMDT;

    
struct rqueue 
{
    int read;
    int write;
    int tag;
    int size;
    pthread_mutex_t Mutex;
    CMDT *queue;

};

extern struct rqueue * rqueue_init(unsigned int size);
extern int rqueue_free(struct rqueue *rq);

extern int rqueue_write(struct rqueue  *rq, CMDT *cmd);
extern CMDT *rqueue_read(struct rqueue *rq);
inline void MsecSleep(int msec);

void parseConfig(const char *path);




#endif
