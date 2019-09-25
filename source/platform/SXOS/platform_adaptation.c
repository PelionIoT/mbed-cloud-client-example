// ----------------------------------------------------------------------------
// Copyright 2018-2019 ARM Ltd.
//
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>


int sxos_uart_write(const char *data, int len);
extern int mbed_cloud_application_entrypoint();

static const char eol[2] = {'\r', '\n'};

// default external application entrypoint for SXOS sdk
int external_application_entrypoint()
{
    return mbed_cloud_application_entrypoint();
}

// XXX: we need to provide this as it misses from SDK and ns_cmdline
// has the default function which uses this and linker wants it.
int vprintf(const char *fmt, va_list ap)
{
    char buf[512];
    int total;
    int charsWritten = 0;

    total = vsnprintf(buf, (sizeof(buf) - 1), fmt, ap);

    const char* outputBuf = buf;
    const char* outputBufEnd = buf + total;

    while (outputBuf < outputBufEnd) {

        // search for end of line
        const char* nextEol = strchr(outputBuf, '\n');

        if (nextEol) {
            // found LF, count bytes before it
            int bytesToEol = nextEol - outputBuf;

            if (bytesToEol == 0) {
                // A single LF, so lets print the CR+LF and skip the source char.
                // This branch could be combined with next if also, but this looks simpler.
                charsWritten += sxos_uart_write(eol, 2);
                outputBuf += 1;
            } else if ((bytesToEol >= 1) && (outputBuf[bytesToEol - 1] != '\r')) {
                // Just LF on end of line, so we need to print chars before it and write extra CR+LF pair.
                charsWritten += sxos_uart_write(outputBuf, bytesToEol);
                charsWritten += sxos_uart_write(eol, 2);
                outputBuf += bytesToEol + 1;
            } else {
                // String had CR+LF, so no need to add extra, just print until LF.
                // Todo: this could just continue too to gang up more chars into write
                // if the source already has the CR+LF's in place.
                charsWritten += sxos_uart_write(outputBuf, bytesToEol + 1);
                outputBuf += bytesToEol + 1;
            }
        } else {
            // no need to convert anymore or at all, just write the rest of string
            int bytesLeft = strlen(outputBuf);
            charsWritten += sxos_uart_write(outputBuf, bytesLeft);
            break;
        }
    }
    return charsWritten;
}

int putchar(int ch)
{
    char buf = (char)ch;

    if (buf == 0x0a) {
        char cr = 0x0d;
        sxos_uart_write(&cr, 1);
    }
    sxos_uart_write(&buf, 1);
    return ch;
}

int puts(const char* str)
{
    size_t len = strlen(str);
    sxos_uart_write(str, len);

    // puts() needs to print string and trailing linefeed, which is CR+LF here
    sxos_uart_write(eol, 2);

    // return value is non-negative on success
    return len;
}

int fflush(void* stream)
{
    return 0;
}

int printf(const char *format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    int length = vprintf(format, arglist);
    va_end(arglist);
    return length;
}

