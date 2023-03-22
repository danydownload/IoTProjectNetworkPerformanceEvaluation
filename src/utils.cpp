#include "utils.h"



long long compute_latency(String message, struct timeval tv)
{
    // Extracting timestamps from the message
    String timestamp_received_s = message.substring(32, 42); // 10 digits(for seconds). Position from 32 to 42 in the payload
    String timestamp_received_us = message.substring(43); // 6 digits(for microseconds). Position from 43 to the end of the payload

    // Parsing timestamps to integers
    unsigned long timestamp_s = strtoul(timestamp_received_s.c_str(), NULL, 10);
    unsigned long timestamp_us = strtoul(timestamp_received_us.c_str(), NULL, 10);

    // Computing latency
    unsigned long long current_time_ms = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
    unsigned long long received_time_ms = timestamp_s * 1000ULL + timestamp_us / 1000ULL;
    long long latency_ms = current_time_ms - received_time_ms;

    // Checking for overflow and incorrect timestamps
    if (timestamp_s > tv.tv_sec || (timestamp_s == tv.tv_sec && timestamp_us >= tv.tv_usec))
    {
        Serial.println("Received timestamp is greater than current time, resetting time...");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        return -1;
    }

    return latency_ms;
}
