#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

/* Probes address 0x76 then 0x77, checks the chip ID, resets and loads calibration.
 * Returns 0 on success, non-zero if no sensor answers. */
uint8_t bmp280_init(void);

/* Triggers one forced-mode measurement and returns compensated values.
 *   temp_c100 : temperature in 0.01 deg C   (e.g. 2543 = 25.43 C)
 *   press_pa  : pressure in Pa              (e.g. 101325 = 1013.25 hPa)
 * Returns 0 on success. */
uint8_t bmp280_read(int32_t *temp_c100, uint32_t *press_pa);

#endif
