#ifndef HTTP_HANDLE_CONNECTION_H
#define HTTP_HANDLE_CONNECTION_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;
extern int http_port;

void handleRequest(AsyncWebServerRequest *request);

void setup_http_server();
void latency_handler(AsyncWebServerRequest *request);
void throughput_handler(AsyncWebServerRequest *request);
void packet_loss_handler(AsyncWebServerRequest *request);

#endif