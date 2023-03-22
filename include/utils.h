#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <time.h>
#include "constants.h"

long long compute_latency(String message, struct timeval tv);


#endif