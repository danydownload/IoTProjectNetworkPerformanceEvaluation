#include <common.h>


void test_latency(char *payload)
{

    // Fill the payload with 'A's
    for (int i = 0; i < LATENCY_MESSAGE_SIZE / 2; i++)
    {
        payload[i] = 'A';
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
	snprintf(payload + 32, LATENCY_MESSAGE_SIZE - 32, "%ld.%06ld", tv.tv_sec, tv.tv_usec);
    // Add a null terminator to the payload
    payload[63] = '\0';
}

void build_large_message(char *message)
{
    // send a large message
		for (int i = 0; i < MESSAGE_SIZE; i++)
		{
			message[i] = 'a' + i % 26;
		}
		message[MESSAGE_SIZE - 1] = '\0';
}

float compute_throughput(unsigned int bytes_sent, unsigned long start_time, unsigned long end_time, String message_type)
{
    // Calculate the throughput in B/s
    float throughput = (float)bytes_sent / (float)(end_time - start_time) * 1000;
    Serial.printf("[%s] throughput: %.2f B/s\n", message_type, throughput);
    // Calculate the throughput in Mb/s
    float throughput_Mbs = (throughput / 1024 / 1024 * 8);
    Serial.printf("[%s] throughput in Mb/s: %.2f Mb/s\n", message_type, throughput_Mbs);
    return throughput;
}