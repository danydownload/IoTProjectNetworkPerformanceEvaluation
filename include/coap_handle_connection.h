#ifndef COAP_HANDLE_CONNECTION_H
#define COAP_HANDLE_CONNECTION_H

#include <Arduino.h>
#include <coap-simple.h>
#include <WiFiUdp.h>


extern const char* coap_latency_topic;
extern const char* coap_packet_loss_topic;
extern const char* coap_throughput_topic;

extern Coap coap;
extern WiFiUDP udp;

void coap_set_endpoints();
void latency_resource(CoapPacket &packet, IPAddress ip, int port);
void packet_loss_resource(CoapPacket &packet, IPAddress ip, int port);
void throughput_resource(CoapPacket &packet, IPAddress ip, int port);
void coap_call_loop();

#endif