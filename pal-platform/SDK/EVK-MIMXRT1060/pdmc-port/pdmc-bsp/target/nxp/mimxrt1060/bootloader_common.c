/* mbed Microcontroller Library
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bootloader_common.h"

#include "fsl_debug_console.h"

/* buffer used in storage operations */
uint8_t buffer_array[BUFFER_SIZE];

/* lookup table for printing hexadecimal values */
const char hexTable[16] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
                          };

/**
 * @brief Function that directly outputs to serial port in blocking mode.
 *
 * @param string outputed to serial port.
 */
void boot_debug(const char *s)
{
    while(*s) {
        PUTCHAR(*s);
        s++;
    }
}

#if 1
/**
 * Redirect libc printf to UART.
 */
int _write(int fd, const void *buf, size_t count)
{
    const char* out = (const char*) buf;

    for (size_t index = 0; index < count; index++) {

        PUTCHAR(out[index]);
    }

    return count;
}
#endif
