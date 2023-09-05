#ifndef __ES8388_VOLUME_H__
#define __ES8388_VOLUME_H__

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The speaker playback route gain (Audio Process Gain + Codec DAC Volume + PA Gain) needs to ensure that the
 * speaker PA output is not saturated and exceeds the speaker rated power. We define the maximum route gain
 * as MAX_GAIN. To ensure the speaker PA output is not saturated, MAX_GAIN can be calculated simply by the formula.
 *    MAX_GAIN = 20 * log(Vpa/Vdac)
 *    Vpa: PA power supply
 *    Vdac: Codec DAC power supply
 * e.g., Vpa = 5V, Vdac = 3.3V, then MAX_GAIN = 20 * log(5/3.3) = 3.6 dB.
 * If the speaker rated power is lower than the speaker PA MAX power, MAX_GAIN should be defined according to
 * the speaker rated power.
 *
 */
#define VPA        (5.0)
#define VDAC       (3.3)
#define MAX_GAIN   (20.0 * log10(VPA / VDAC))

/*
 * User can customize the volume setting by modifying the mapping table and adjust the volume step according to
 * the speaker playback system, and the other volume levels shift the value accordingly.
 * Integers are used instead of floating-point variables to reduce storage space. -80 means -40 dB, 0 means 0 dB.
 */
static const int8_t dac_volume_offset[] = {
    -99, -98, -97, -96, -95, -94, -93, -92, -91, -90, -89, -88, -87, -86, -85, -84, -83, -82, -81, -80,
    -79, -78, -77, -76, -75, -74, -73, -72, -71, -70, -69, -68, -67, -66, -65, -64, -63, -62, -61, -60,
    -59, -58, -57, -56, -55, -54, -53, -52, -51, -50, -49, -48, -47, -46, -45, -44, -43, -42, -41, -40,
    -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24, -23, -22, -21, -20,
    -19, -18, -17, -16, -15, -14, -13, -12, -11, -10, -9,  -8,  -7,  -6,  -5,  -4,  -3,  -2,  -1,   0
};

/* User Defined Values */
#define PA_GAIN         30
#define MAX_DAC_VOLUME  100
#define MIN_DAC_VOLUME  -96
#define DAC_VOL_SYMBOL  -1
#define VOL_ACCURACY    0.5
#define ZERO_VOL_REG    0

#ifdef __cplusplus
}
#endif

#endif //__ES8388_VOLUME_H__
