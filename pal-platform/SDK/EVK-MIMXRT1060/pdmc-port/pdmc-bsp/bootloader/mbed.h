
#ifndef MBED_H
#define MBED_H

#include "cmsis_compiler.h"

#if DEVICE_RESET_REASON
#include "reset_reason_api.h"
#endif

#include <cstdlib>

#define MBED_CONF_MBED_BOOTLOADER_APPLICATION_START_ADDRESS (MBED_CONF_UPDATE_CLIENT_APPLICATION_DETAILS + MBED_BOOTLOADER_ACTIVE_HEADER_REGION_SIZE)

#endif
