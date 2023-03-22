#include "coap_handle_connection.h"
#include "utils.h"
#include "mqtt_handle_connection.h"

const char *coap_latency_topic = "coap/room/devices/esp32-server/latency";
const char *coap_packet_loss_topic = "coap/room/devices/esp32-server/packet-loss";
const char *coap_throughput_topic = "coap/room/devices/esp32-server/throughput";


WiFiUDP udp;
Coap coap(udp);

void coap_set_endpoints()
{
    coap.server(latency_resource, coap_latency_topic);
    coap.server(packet_loss_resource, coap_packet_loss_topic);
    coap.server(throughput_resource, coap_throughput_topic);
}

void latency_resource(CoapPacket &packet, IPAddress ip, int port)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';
    String message = String(p);

    long long latency_ms = compute_latency(message, tv);

    if (latency_ms == -1)
    {
        // Serial.println("Error computing latency");
        return;
    }

    // Printing results
    // Serial.print("Coap Latency (ms): ");
    Serial.println(latency_ms);
    mqttClient.publish(topic_nodered_coap_latency , String(latency_ms).c_str());
    coap.sendResponse(ip, port, packet.messageid, "1");
}

void packet_loss_resource(CoapPacket &packet, IPAddress ip, int port)
{
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';
    String message = String(p);

    if (message == "Ping")
    {
        coap.sendResponse(ip, port, packet.messageid, "Pong");
    }
    else
    {
        coap.sendResponse(ip, port, packet.messageid, "Error");
    }
}

void throughput_resource(CoapPacket &packet, IPAddress ip, int port)
{
    char p[packet.payloadlen + 1];
    memcpy(p, packet.payload, packet.payloadlen);
    p[packet.payloadlen] = '\0';
    String message = String(p);
    // print length of the payload
    // Serial.print("Coap Throughput message received len: ");
    // Serial.println(packet.payloadlen);
    coap.sendResponse(ip, port, packet.messageid, "10");
}

void coap_call_loop()
{
    coap.loop();
}