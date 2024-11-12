#include "mem.h"
#include "../boilerplate.h"

const char* const memory_model[MODELS_COUNT] = {
    "24C1024",
    "24C512",
    "24C256",
    "24C128",
    "24C64",
    "24C32",
    "24C16",
    "24C08",
    "24C04",
    "24C02",
    "24C01",
};

const uint32_t memory_capacity[MODELS_COUNT] = {
    13172,
    65536,
    32768,
    16384,
    8192,
    4096,
    2048,
    1024,
    512,
    256,
    128,
};

uint8_t memory_get_list_size() {
    return MODELS_COUNT;
};
