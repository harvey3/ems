#ifndef UART_HEADER
#define UART_HEADER

int openSerial(int iPort, int baud, int stopLen, int parity);
void setSerialMinTime(int iPort, int vmin, int vtime);
int UartSendRecvFrame(int iPort, ModbusMaster *Ms);
int UartRecvData(int iPort, char *buf, int len);
int UartSendData(int iPort, char *buf, int len);

#define AIRCON_PORT 3
#define ONGRID_WH_METER_PORT 7
#define OFFGRID_WH_METER_PORT 5
#define TEMP_SENSOR_PORT 2
#define TRANS_THERMO_PORT 2
#define IO_PORT 8



#endif
