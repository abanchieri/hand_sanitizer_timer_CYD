#pragma once

#include <stdint.h>

void sensorsBegin();
void sensorsUpdate(uint32_t now);
bool sensorsBottleRemoved(uint8_t bottleIndex);