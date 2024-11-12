#include "file_io.h"
#include "i2c.h"
#include "../boilerplate.h"

void fio_write_buf_to_file(char* filename, uint8_t* buffer) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    storage_file_open(file, filename, FSAM_WRITE, FSOM_OPEN_APPEND);
    storage_file_write(file, buffer, I2C_BUF_SIZE);
    storage_file_close(file);
    storage_file_free(file);
}

void fio_read_file_to_buf(const char* filename, uint8_t* buffer, int offset) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    storage_file_open(file, filename, FSAM_READ, FSOM_OPEN_EXISTING);
    storage_file_seek(file, offset, true);
    storage_file_read(file, buffer, I2C_BUF_SIZE);
    storage_file_close(file);
    storage_file_free(file);
}