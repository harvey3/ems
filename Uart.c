/*********************Copyright(c)******************************
**
**          Guangzhou ZHIYUAN electronics Co.,LTD
**
**              http://www.zlg.cn
**
** File:					serial/long-test.c
** Created Date:			2008-05-19
** Latest Modified Date:	2015-12-15
** Latest Version:			v1.0
** Description:             NONE
**
****************************************************************/
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>  
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <termios.h>
#include <errno.h>   
#include <limits.h> 
#include <asm/ioctls.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

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
#include "log.h"

#define DATA_LEN	0x19		/* test data's len              */
#define STOP		0			/* test stop                    */
#define RUN			1			/* test running                 */

static int GiSerialFds[8] = {-1, -1, -1, -1, -1, -1, -1, -1};		/* all serial device fd         */
/************************************************************
** Function name:		openSerial
** Descriptions:		open serial port at raw mode
** ARGUMENTS:
** input paramters:		iNum	serial port number
** output paramters:	NONE
** Return value:		iFD		file descriptor
*************************************************************/
int openSerial(int iPort, int baud, int stopLen, int parity)
{
    int iFd;

    struct termios opt; 
    char cSerialName[15];

    if (iPort > 8) {
        log_error("no such serial port:ttymxc%d . ", iPort-1);
        exit(1);
    }
    
    if (GiSerialFds[iPort-1] > 0) {
        return GiSerialFds[iPort-1];
    }

    sprintf(cSerialName, "/dev/ttymxc%d", iPort-1);
    log_debug("open serial name:%s ", cSerialName);
    iFd = open(cSerialName, O_RDWR | O_NOCTTY);                        
    if(iFd < 0) {
        perror(cSerialName);
        return -1;
    }

    tcgetattr(iFd, &opt);      

    switch (baud) {

    case 9600:
        cfsetispeed(&opt, B9600);
        cfsetospeed(&opt, B9600);
        break;
        
    case 19200:    
        cfsetispeed(&opt, B19200);
        cfsetospeed(&opt, B19200);
        break;
        
    case 38400:
        cfsetispeed(&opt, B38400);
        cfsetospeed(&opt, B38400);
        break;
        
    case 57600:
        cfsetispeed(&opt, B57600);
        cfsetospeed(&opt, B57600);
        break;
    case 115200:
        cfsetispeed(&opt, B115200);
        cfsetospeed(&opt, B115200);
        break;
    default:
        log_error("unsupported baud");

    }

    /*
     * raw mode
     */
    opt.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    opt.c_oflag &= ~(OPOST);
    opt.c_cflag &= ~(CSIZE | PARENB);
    opt.c_cflag |= CS8;

    if (stopLen == 2)
        opt.c_cflag |= CSTOPB;

    if (parity == 1)
        opt.c_cflag |= (PARENB | PARODD);
    else if (parity == 2)
        opt.c_cflag |= PARENB;

    
        
    /*
     * 'DATA_LEN' bytes can be read by serial
     */
    opt.c_cc[VMIN] = DATA_LEN;                                      
    opt.c_cc[VTIME] = 1;

    if (tcsetattr(iFd, TCSANOW, &opt)<0) {
        return   -1;
    }

    GiSerialFds[iPort - 1] = iFd; 

    return iFd;
}
void setSerialMinTime(int iPort, int vmin, int vtime)
{
    struct termios opt;
    int Fd;
    int ret;
    
    Fd = GiSerialFds[iPort - 1];

    if (Fd <= 0) {
        log_error("Fd not valid");
        return;
    }


    ret = tcgetattr(Fd,&opt);
    if (ret < 0)
        log_error("get serial attr fail");
    
    
    opt.c_cc[VMIN] = vmin;                                      
    opt.c_cc[VTIME] = vtime;
    ret = tcsetattr(Fd, TCSANOW, &opt);
    if (ret < 0)
        log_error("set serial vmin vtime fail");

    
}

int UartSendData(int iPort, char *buf, int len)
{
    
    int Fd;
    int ret;
    
    
    Fd = GiSerialFds[iPort - 1];

    if (Fd <= 0) {
        log_error("Fd not valid");
        return -1;
    }
    
    ret = write(Fd, buf, len);
    if (ret < 0)
    {
        log_error("write uart error");
        return ret;

    }
    return ret;
    
}

int UartRecvData(int iPort, char *buf, int len)
{
    struct timeval tv;
    fd_set rfds;
    int Fd;
    int ret;
    
    
    Fd = GiSerialFds[iPort - 1];

    if (Fd <= 0) {
        log_error("Fd not valid");
        
        return -1;
    }
#if 1    
    FD_ZERO(&rfds);
    FD_SET(Fd, &rfds);
    tv.tv_sec=MODBUS_RESP_TIMEOUT_MSEC/1000;
    tv.tv_usec=(MODBUS_RESP_TIMEOUT_MSEC%1000)*1000;
        
    ret = select(Fd + 1, &rfds, NULL, NULL, &tv);

    if (ret > 0) {

        if (FD_ISSET(Fd, &rfds)) {

            ret = read(Fd, buf, len);
            if (ret < 0)
            {
                log_error("read uart error");
                return ret;

            }
        }
    } else if (ret == 0)
        log_error("recv uart timeout");
    
#endif
#if 0
    log_error("read data\n");
    ret = read(Fd, buf, len);
    if (ret < 0)
    {
        log_error("read uart error\n");
        return ret;

    }
#endif
    
    return ret;
    
}

int UartSendRecvFrame(int iPort, ModbusMaster *Ms)
{
    struct timeval tv;
    fd_set rfds;
    int Fd;
    int ret;
    int len;
    
    Fd = GiSerialFds[iPort - 1];

    if (Fd <= 0)
        return -1;

    setSerialMinTime(iPort, Ms->predictedResponseLength, 1);
    
    ret = write(Fd, Ms->request.frame, Ms->request.length);
    if (ret < 0)
    {
        log_error("write uart port %d error", iPort);
        return ret;

    }
    
            
    FD_ZERO(&rfds);
    FD_SET(Fd, &rfds);
    tv.tv_sec=MODBUS_RESP_TIMEOUT_MSEC/1000;
    tv.tv_usec=(MODBUS_RESP_TIMEOUT_MSEC%1000)*1000;
        
    ret = select(Fd + 1, &rfds, NULL, NULL, &tv);

    if (ret > 0) {

        if (FD_ISSET(Fd, &rfds)) {

            len = read(Fd, Ms->response.frame, Ms->predictedResponseLength);
            if (len == Ms->predictedResponseLength) {
                
                Ms->response.length = len;
            } else {
                log_error("modbus resp data lost %d, expected %d", len, Ms->predictedResponseLength);
                return -1;

            }
            

        }
        


    } else if (ret == 0) {

        log_error("port %d modbus resp timeout", iPort);
        return -1;
        
    } else {

        log_error("select on uart port %d failed", iPort);
        return -1;
        
    }
    
    return 0;
    

}
