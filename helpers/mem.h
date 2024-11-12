#include <stdint.h>

#define MODELS_COUNT 11

extern const char* const memory_model[MODELS_COUNT];

extern const uint32_t memory_capacity[MODELS_COUNT];

uint8_t memory_get_list_size();