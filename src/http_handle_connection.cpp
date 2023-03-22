#include "http_handle_connection.h"
#include "utils.h"
#include "mqtt_handle_connection.h"

int http_port = 80;

AsyncWebServer server(http_port);

void setup_http_server()
{
    server.on("/latency", HTTP_POST, latency_handler);

    server.on("/throughput", HTTP_POST, throughput_handler);
}

void latency_handler(AsyncWebServerRequest *request)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    String message = request->arg("message");
    request->send(200, "text/plain", "Message with timestamp received!");
    long long latency_ms = compute_latency(message, tv);
    // Serial.print("HTTP Latency (ms): ");
    // Serial.println(latency_ms);
    mqttClient.publish(topic_nodered_http_latency , String(latency_ms).c_str());
}

void throughput_handler(AsyncWebServerRequest *request)
{
    String message = request->arg("message");
    request->send(200, "text/plain", "Message received!");
}