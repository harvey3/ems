#include <stdint.h>
#include <stdio.h>
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

char IoSendBuffer[25];
char IoRecvBuffer[25];

IoData gIoData;

void IoChecksumCalculate(void)
{
	unsigned char i,sum;
	sum=0;
	for (i=4;i<24;i++)
	{
		sum = sum + IoSendBuffer[i];
	}
	IoSendBuffer[24] = sum;
}

void SendToEXIODataFrame(void)    
{

    
	IoSendBuffer[0] = 0xAA;
	IoSendBuffer[1] = 0x55;	   			
	IoSendBuffer[2] = 0xE7;	    			
	IoSendBuffer[3] = 0x18;	
	IoSendBuffer[4] = 0xFF;					
	IoSendBuffer[5] = 0x80;	 
	IoSendBuffer[6] = 0x00;	   
		

	IoSendBuffer[9] = gIoData.output1;

    
}
void IoProcessCmd(CMDT *cmd)
{
    log_debug("process IO cmd %d value %d", cmd->cmd, cmd->value);
    
    switch (cmd->cmd) {

    case IO_SET_SWITCH_ON_CMD:
        gIoData.output1 = 1;
        break;
    case IO_SET_SWITCH_OFF_CMD:
        gIoData.output1 = 2;
        break;
    case IO_SET_FAN_CMD:
        if (cmd->value)
            gIoData.output1 |= 1 << 2;
        else
            gIoData.output1 &= ~(1 << 2);
        break;
    default:
        log_debug("IO cmd not support %d", cmd->cmd);
        
    }



}

void* IoThread(void *param)
{

    struct timeval tv;
    CMDT *cmd;
    int err;

    gIoData.cmdRq = rqueue_init(RQUEUE_SIZE);
    
    err = openSerial(IO_PORT, 9600, 1, 1);
    if (err < 0)
        pthread_exit(NULL);

    memset(IoSendBuffer, 0, 25);
    
    while (1) {


        tv.tv_sec=IO_DELAY_MSEC/1000;
        tv.tv_usec=(IO_DELAY_MSEC%1000)*1000;
    
        do{
            err=select(0,NULL,NULL,NULL,&tv);
        }while(err<0 && errno==EINTR);

        while ((cmd = rqueue_read(gIoData.cmdRq))) {

            IoProcessCmd(cmd);

        }

        SendToEXIODataFrame();
        IoChecksumCalculate();
        
        log_debug("send io data...");
        err = UartSendData(IO_PORT, IoSendBuffer, 25);
        
        err = UartRecvData(IO_PORT, IoRecvBuffer, 25);
        
        if (err <= 0)
            log_error("not IO data %d", err);
        else {
            
            log_debug("received %d bytes IO data", err);
            gIoData.input1 = IoRecvBuffer[9];
            gIoData.input2 = IoRecvBuffer[10];
            
            
            
        }

        

    }
    rqueue_free(gIoData.cmdRq);
    pthread_exit(NULL);
    

}
