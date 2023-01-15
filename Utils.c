#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

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
#include "Utils.h"
#include "log.h"


struct rqueue * rqueue_init(unsigned int size)
{
    struct rqueue *rq;
    int ret;
    
    rq = malloc(sizeof(struct rqueue));
    memset(rq, 0, sizeof(struct rqueue));

    rq->queue = malloc(sizeof(CMDT) * size);
    memset(rq->queue, 0, sizeof(CMDT) * size);

    rq->size = size;
    rq->read = 0;
    rq->write = 0;
    rq->tag = 0;
    ret = pthread_mutex_init(&rq->Mutex, NULL);
    if (ret < 0) {
        log_error("create thread mutex fail");
        return NULL;
    }



    
    return rq;
    

}
int rqueue_free(struct rqueue *rq)
{
    if (!rq)
        return -1;

    free(rq->queue);
    pthread_mutex_destroy(&rq->Mutex);
    free(rq);
    return 0;
    
}
inline int rqueue_empty(struct rqueue *rq)
{
    if (!rq)
        return -1;
    return (rq->read == rq->write) && (rq->tag == 0);
    
}

inline int rqueue_full(struct rqueue *rq)
{

    if (!rq)
        return -1;
    
    return (rq->read == rq->write) && (rq->tag == 1);
    
}
CMDT *rqueue_read(struct rqueue *rq)
{
    
    CMDT *p;
    
    if (!rq)
        return NULL;
    
    if (rqueue_empty(rq)) {

        return NULL;
    }
    
    p = rq->queue + rq->read;
    rq->read = (rq->read + 1)&(rq->size - 1);
    if (rq->write == rq->read)
        rq->tag = 0;
    
    return p;
    
}

int rqueue_write(struct rqueue *rq, CMDT *cmd)
{
    CMDT *t;
    int ret;

    if (!rq || !cmd)
        return -1;
    
    ret = pthread_mutex_lock(&rq->Mutex);

    if (ret < 0)
    {
        log_error("lock mutex error %d", ret);
    }
    else
    {

    if (rqueue_full(rq)) {

        ret = -1;
        goto out;
        
        
    }
    
    
    t = rq->queue + rq->write;
    memcpy(t, cmd, sizeof(CMDT));

    rq->write = (rq->write + 1)&(rq->size - 1);

    if (rq->write == rq->read)
        rq->tag = 1;

out:
    if (pthread_mutex_unlock(&rq->Mutex))
    {
        log_error("unlock mutex fail");
    }


    }

    return ret;
    
}

