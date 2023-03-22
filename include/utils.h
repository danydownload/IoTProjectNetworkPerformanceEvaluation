#ifndef _UTILS_H
#define _UTILS_H

#include <Arduino.h>
#include "BluetoothSerial.h"

extern const int MESSAGE_SIZE;
extern BluetoothSerial SerialBT;
extern const char *ntpServer;
extern const long gmtOffset_sec;	  /*ECT OFFSET +1 HOURS(3600 SEC)*/
extern const int daylightOffset_sec; /*1 hour daylight offset*/

void build_large_message(char *message);

void compute_packet_loss_and_latency_and_throughput(const char *type, const char *uri, const char *topic, const char *method);



#endif // _UTILS_H