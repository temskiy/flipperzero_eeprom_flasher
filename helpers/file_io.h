#include <stdint.h>

void fio_write_buf_to_file(char* filename, uint8_t* buffer);
void fio_read_file_to_buf(const char* filename, uint8_t* buffer, int offset);
