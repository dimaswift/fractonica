/*
 * solar_impl.c — Solar eclipse implementation translation unit.
 *
 * Compile with lunar_impl.c and test_saros_lib.c (or your own main).
 * Optionally define SAROS_USE_ALL to use the full Saros 1-180 dataset.
 * Optionally define ECLIPSE_USE_PROGMEM on AVR/ESP32.
 */

#define SAROS_IMPL_SOLAR
/* #define SAROS_USE_ALL */

#include "../../include/saros/solar/eclipse_times_modern.h"
#include "../../include/saros/solar/eclipse_info_modern.h"
#include "./../include/saros/solar/saros_modern.h"
#include "../../include/saros.h"
