// ----------------------------------------------------------------------------
// Copyright 2020 ARM Ltd.
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

#if defined (MBED_CONF_APP_ENABLE_DS_CUSTOM_METRICS_EXAMPLE)

#include "ds_custom_metrics.h"
#include "ds_custom_metrics_app.h"

/**
 * @brief Example implementation of the custom metric callback. 
 * This function will be called each time when Device Sentry is required to know the value 
 * and the type of the particular metric (i.g. before sending metric report to the Pelion).
 * @param[in] metric_id custom metric id.
 * @param[in] user_context the opaque pointer that was passed during he callback registration.
 * @param[out] metric_value_out_addr address of the pointer that should point to the buffer that contains value of the metric.
 * @param[out] metric_value_type_out output type of the metric with an appropriate metric id.
 * @param[out] metric_value_size_out metric_value_out size in bytes.
 * @returns
 *      ::DS_STATUS_SUCCESS if all output values were successfully filled.
 *      One of the ::ds_status_e errors otherwise.
 */
ds_status_e mcce_ds_custom_metric_value_getter(
                                        ds_custom_metric_id_t metric_id,                        // input param
                                        void *user_context,                                     // input param
                                        uint8_t **metric_value_out_addr,                        // output param
                                        ds_custom_metrics_value_type_t *metric_value_type_out,  // output param
                                        size_t *metric_value_size_out                           // output param
                                    )
{
    static uint64_t value = 115; // 115 is the arbitrary value.   
    value++; // arbitrary change of the metric value.
    *metric_value_out_addr = (uint8_t*)&value;
    *metric_value_type_out = DS_INT64;
    *metric_value_size_out = DS_SIZE_OF_INT64;
    return DS_STATUS_SUCCESS;
}

void mcce_ds_custom_metric_callback_set()
{
    static int user_context = 0;
    ds_custom_metric_callback_set(mcce_ds_custom_metric_value_getter, &user_context);
}

#endif
