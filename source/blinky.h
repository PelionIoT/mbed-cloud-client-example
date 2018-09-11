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

#ifndef __BLINKY_H__
#define __BLINKY_H__

#include "nanostack-event-loop/eventOS_event.h"
#include "m2mresource.h"
#include <stdint.h>

class Blinky
{
    typedef enum {
        STATE_IDLE,
        STATE_STARTED,
    } BlinkyState;

    typedef void(*blinky_completed_cb) (void);

public:
    Blinky();

    ~Blinky();

    bool start(const char* pattern, size_t length, bool pattern_restart, blinky_completed_cb cb);

    void stop();

public:
    void event_handler(arm_event_s &event);


private:
    int get_next_int();
    bool run_step();

private:

    char *_pattern;
    const char *_curr_pattern;

    BlinkyState _state;

    bool _restart;

    static int8_t _tasklet;

    blinky_completed_cb _callback;

};
#endif /* __BLINKY_H__ */
