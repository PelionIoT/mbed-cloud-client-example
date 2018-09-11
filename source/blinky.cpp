// ----------------------------------------------------------------------------
// Copyright 2018 ARM Ltd.
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

#include "blinky.h"

#include "nanostack-event-loop/eventOS_event.h"
#include "nanostack-event-loop/eventOS_event_timer.h"

#include "mbed-trace/mbed_trace.h"
#include "mcc_common_button_and_led.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define TRACE_GROUP "blky"

#define BLINKY_TASKLET_INIT_EVENT 0
#define BLINKY_TASKLET_TIMER 10

int8_t Blinky::_tasklet = -1;

extern "C" {

static void blinky_event_handler_wrapper(arm_event_s *event)
{
    assert(event);

    if (event->event_type != BLINKY_TASKLET_INIT_EVENT) {
        Blinky *instance = (Blinky *)event->data_ptr;
        instance->event_handler(*event);
    }
}

}

Blinky::Blinky() : _pattern(NULL), _curr_pattern(NULL), _state(STATE_IDLE)
{
}

Blinky::~Blinky()
{
    // stop will free the pattern buffer if any is allocated
    stop();
}

bool Blinky::start(const char* pattern, size_t length, bool pattern_restart, blinky_completed_cb cb)
{
    assert(pattern);
    _callback = cb;
    _restart = pattern_restart;

    if (_tasklet < 0) {
        _tasklet = eventOS_event_handler_create(blinky_event_handler_wrapper, BLINKY_TASKLET_INIT_EVENT);

        if (_tasklet < 0) {
            return false;
        }
    }

    // allow one to start multiple times before previous sequence has completed
    stop();

    _pattern = (char*)malloc(length+1);
    if (_pattern == NULL) {
        return false;
    }

    memcpy(_pattern, pattern, length);
    _pattern[length] = '\0';

    _curr_pattern = _pattern;

    return run_step();
}

void Blinky::stop()
{
    free(_pattern);
    _pattern = NULL;
    _curr_pattern = NULL;
    _state = STATE_IDLE;
}

int Blinky::get_next_int()
{
    int result = -1;

    char *endptr;

    int conv_result  = strtol(_curr_pattern, &endptr, 10);

    if (*_curr_pattern != '\0') {
        // ints are separated with ':', which we will skip on next time
        if (*endptr == ':') {
            endptr += 1;
            result = conv_result;
        } else if (*endptr == '\0') { // end of
            result = conv_result;
        } else {
            tr_debug("invalid char %c", *endptr);
        }
    }

    _curr_pattern = endptr;

    return result;
}

bool Blinky::run_step()
{
    int delay = get_next_int();

    // tr_debug("patt: %s, curr: %s, delay: %d", _pattern, _curr_pattern, delay);

    if (delay < 0) {
        _callback();
        _state = STATE_IDLE;
        return false;
    }

    arm_event_t event;

    memset(&event, 0, sizeof(event));

    event.event_type = BLINKY_TASKLET_TIMER;
    event.receiver = _tasklet;
    event.sender =  _tasklet;
    event.data_ptr = this;
    event.priority = ARM_LIB_MED_PRIORITY_EVENT;

    if (eventOS_event_send_after(&event, eventOS_event_timer_ms_to_ticks(delay)) == NULL) {
        _state = STATE_IDLE;
        assert(false);
        return false;
    }

    mcc_platform_toggle_led();

    _state = STATE_STARTED;

    return true;
}

void Blinky::event_handler(arm_event_s &event)
{
    assert(event.event_type == BLINKY_TASKLET_TIMER);

    bool success = run_step();

    if ((!success) && (_restart)) {
        // tr_debug("Blinky restart pattern");
        _curr_pattern = _pattern;
        run_step();
    }
}
