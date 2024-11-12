#include "i2c.h"
#include "../boilerplate.h"

uint8_t i2c_buf[I2C_BUF_SIZE] = {0};
const uint8_t i2c_device_base_address = 0x50;

uint8_t i2c_read_byte(int address, bool two_byte_address, bool start_stop) {
    uint8_t device_address_read;
    uint8_t device_address_write;
    uint8_t memory_address_len;
    uint8_t memory_address_buffer[2] = {0, 0};
    uint8_t buffer_rx[1] = {0};
    uint32_t timeout = 100;

    if(two_byte_address == false) {
        device_address_write = (i2c_device_base_address | (uint8_t)(address >> 8)) << 1;
        device_address_read = device_address_write + 1;
        memory_address_buffer[0] = (uint8_t)(address & 0xFF);
        memory_address_len = 1;
    } else {
        device_address_write = i2c_device_base_address << 1;
        device_address_read = device_address_write + 1;
        memory_address_buffer[1] = (uint8_t)(address & 0xFF);
        memory_address_buffer[0] = (uint8_t)(address >> 8);
        memory_address_len = 2;
    }

    if(start_stop) {
        furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    }
    furi_hal_i2c_tx_ext(
        &furi_hal_i2c_handle_external,
        device_address_write,
        false,
        memory_address_buffer,
        memory_address_len,
        FuriHalI2cBeginStart,
        FuriHalI2cEndAwaitRestart,
        timeout);
    furi_hal_i2c_rx_ext(
        &furi_hal_i2c_handle_external,
        device_address_read,
        false,
        buffer_rx,
        memory_address_len,
        FuriHalI2cBeginRestart,
        FuriHalI2cEndStop,
        timeout);
    if(start_stop) {
        furi_hal_i2c_release(&furi_hal_i2c_handle_external);
    }
    return buffer_rx[0];
}

void i2c_read_buf(int start_address, bool two_byte_address) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    for(int i = 0; i < I2C_BUF_SIZE; i++) {
        i2c_buf[i] = i2c_read_byte(start_address + i, two_byte_address, false);
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}

void i2c_write_byte(int address, bool two_byte_address, uint8_t value, bool start_stop) {
    uint8_t device_address_write;
    uint8_t memory_buffer_len;
    uint8_t memory_buffer[3] = {0, 0, 0};
    uint32_t timeout = 100;

    if(two_byte_address == false) {
        device_address_write = (i2c_device_base_address | (uint8_t)(address >> 8)) << 1;
        memory_buffer[1] = value;
        memory_buffer[0] = (uint8_t)(address & 0xFF);
        memory_buffer_len = 2;
    } else {
        device_address_write = i2c_device_base_address << 1;
        memory_buffer[2] = value;
        memory_buffer[1] = (uint8_t)(address & 0xFF);
        memory_buffer[0] = (uint8_t)(address >> 8);
        memory_buffer_len = 3;
    }

    if(start_stop) {
        furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    }
    furi_hal_i2c_tx_ext(
        &furi_hal_i2c_handle_external,
        device_address_write,
        false,
        memory_buffer,
        memory_buffer_len,
        FuriHalI2cBeginStart,
        FuriHalI2cEndStop,
        timeout);
    if(start_stop) {
        furi_hal_i2c_release(&furi_hal_i2c_handle_external);
    }
    furi_delay_ms(2);
}

void i2c_write_buf(int start_address, bool two_byte_address) {
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    for(int i = 0; i < I2C_BUF_SIZE; i++) {
        i2c_write_byte(start_address + i, two_byte_address, i2c_buf[i], false);
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
}

bool i2c_verify_buf(int start_address, bool two_byte_address) {
    bool ec = true;
    furi_hal_i2c_acquire(&furi_hal_i2c_handle_external);
    for(int i = 0; i < I2C_BUF_SIZE; i++) {
        if (i2c_buf[i] != i2c_read_byte(start_address + i, two_byte_address, false)) {
            ec = false;
            break;
        }
    }
    furi_hal_i2c_release(&furi_hal_i2c_handle_external);
    return ec;
}
