#pragma once

#include <stdio.h>
#include <esp_err.h>

class io_expander
{
    public:
        io_expander();
        ~io_expander();
        esp_err_t setOutputPin(uint8_t io_num);
        esp_err_t setPinHigh(uint8_t io_num);
        esp_err_t setPinLow(uint8_t io_num);
        esp_err_t deinitI2C();
        
    private:
        esp_err_t initI2C();
        // esp_err_t deinitI2C();
        esp_err_t i2cRead(uint8_t reg_addr, uint8_t *data, size_t len);
        esp_err_t i2cWrite(uint8_t reg_addr, uint8_t data);
};