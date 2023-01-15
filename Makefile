EXEC = ./ems
OBJS = main.o AirCon.o WattHourMeter.o IO.o Uart.o modbus.o TempSensor.o mqtt.o modbusTCP.o Pcs.o Bms.o config.o Utils.o log.o
 
SRC  = main.c AirCon.c WattHourMeter.c IO.c Uart.c modbus.c TempSensor.c mqtt.c modbusTCP.c Pcs.c Bms.c config.c Utils.c log.c

CC = arm-linux-gnueabihf-gcc
#CC = arm-none-linux-gnueabi-gcc
CFLAGS += -O2 -Wall -I ../liblightmodbus-master/include -I ../paho.mqtt.embedded-c-v1.1.0/MQTTClient-C/src -I ../paho.mqtt.embedded-c-v1.1.0/MQTTPacket/src -I ../paho.mqtt.embedded-c-v1.1.0/MQTTClient-C/src/linux


#CFLAGS += -O2 -Wall
LDFLAGS += -L ../liblightmodbus-master/build -llightmodbus -lpthread -L ../paho.mqtt.embedded-c-v1.1.0/build.paho/MQTTPacket/src -lMQTTPacketClient -L ../paho.mqtt.embedded-c-v1.1.0/build.paho/MQTTPacket/src -lpaho-embed-mqtt3c -L ../paho.mqtt.embedded-c-v1.1.0/build.paho/MQTTClient-C/src -lpaho-embed-mqtt3cc

all: clean $(EXEC)

$(EXEC):$(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) 

%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -vf $(EXEC) *.o *~
