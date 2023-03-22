#ifndef HTTP_HANDLE_CONNECTION_H
#define HTTP_HANDLE_CONNECTION_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <WiFiClient.h>
#include <ESP32Ping.h>
#include <constants.h>


// extern WiFiClient wifiClient;
// extern HTTPClient httpClient;


bool connectToHttpServer(HTTPClient& httpClient, const IPAddress& ip, int port, const String& path);
void closeConnectionToHttpServer(HTTPClient& httpClient);
int sendHttpPost(HTTPClient& httpClient, const String& payload);
// void sendHTTPPost_2(const IPAddress& ip, int port, const char* message, time_t timestamp_s, long timestamp_us);
void send5KBMessage(const IPAddress &ip, int port);
// void sendHTTPMessage(const IPAddress &ip, int port, const char *message);
// void sendAndCalculatePacketLoss(const IPAddress& ip, int port, const char* message, int num_packets);


#endif // HTTP_HANDLE_CONNECTION_H