#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <Arduino.h>

const int LATENCY_MESSAGE_SIZE = 64;
// const int MESSAGE_SIZE = 512;
const int MESSAGE_SIZE = 2048;
const int TIMES_TO_SEND = 10;

void test_latency(char *payload);
void build_large_message(char *message);
float compute_throughput(unsigned int bytes_sent, unsigned long start_time, unsigned long end_time, String message_type);


#endif