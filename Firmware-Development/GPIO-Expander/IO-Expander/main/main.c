#include "PCA9536.h"

void app_main(void)
{
    pca9536_i2c_master_init();
    pca9536_set_pin_mode_output(PCA9536_IO1);
    pca9536_set_pin_high(PCA9536_IO1);
    pca9536_i2c_deinit();
}
