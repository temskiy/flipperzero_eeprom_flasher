#include <stdint.h>
#include <stdbool.h>

#define I2C_BUF_SIZE 128

extern uint8_t i2c_buf[I2C_BUF_SIZE];
extern const uint8_t i2c_device_base_address;

uint8_t i2c_read_byte(int address, bool two_byte_address, bool start_stop);
void i2c_read_buf(int start_address, bool two_byte_address);
void i2c_write_byte(int address, bool two_byte_address, uint8_t value, bool start_stop);
void i2c_write_buf(int start_address, bool two_byte_address);
bool i2c_verify_buf(int start_address, bool two_byte_address);
