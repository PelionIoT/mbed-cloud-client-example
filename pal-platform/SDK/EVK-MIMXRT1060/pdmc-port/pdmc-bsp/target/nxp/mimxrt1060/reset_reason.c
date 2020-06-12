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

#include "reset_reason_api.h"

#include "fsl_src.h"

#include <stdio.h>

#if 0
typedef enum {
    RESET_REASON_POWER_ON,       /**< Set when power is initially applied to the board. The power-on-reset circuit causes a POWER_ON reset when this occurs */
    RESET_REASON_PIN_RESET,      /**< Set when a reset is triggered by the hardware pin on the board */
    RESET_REASON_BROWN_OUT,      /**< Triggered when the voltage drops below the low voltage detect (LVD) threshold; the system is held in a reset until the voltage rises above the threshold */
    RESET_REASON_SOFTWARE,       /**< Set during software reset, typically triggered by writing the SYSRESETREQ bit in the Application Interrupt and Reset Control register */
    RESET_REASON_WATCHDOG,       /**< Set when a running watchdog timer fails to be refreshed */
    RESET_REASON_LOCKUP,         /**< Set when the core is locked because of an unrecoverable exception */
    RESET_REASON_WAKE_LOW_POWER, /**< Set when waking from deep sleep mode */
    RESET_REASON_ACCESS_ERROR,   /**< Umbrella value that encompasses any access related reset */
    RESET_REASON_BOOT_ERROR,     /**< Umbrella value that encompasses any boot related reset */
    RESET_REASON_MULTIPLE,       /**< Set if multiple reset reasons are set within the board. Occurs when the reset reason registers aren't cleared between resets */
    RESET_REASON_PLATFORM,       /**< Platform specific reset reason not captured in this enum */
    RESET_REASON_UNKNOWN         /**< Unknown or unreadable reset reason **/
} reset_reason_t;
#endif

#if 0
enum _src_reset_status_flags
{
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_RESET_OUT) && FSL_FEATURE_SRC_HAS_SRSR_RESET_OUT)
    kSRC_ResetOutputEnableFlag = SRC_SRSR_RESET_OUT_MASK, /*!< This bit indicates if RESET status is
                                                               driven out on PTE0 pin. */
#endif                                                    /* FSL_FEATURE_SRC_HAS_SRSR_RESET_OUT */
#if !(defined(FSL_FEATURE_SRC_HAS_NO_SRSR_WBI) && FSL_FEATURE_SRC_HAS_NO_SRSR_WBI)
    kSRC_WarmBootIndicationFlag = SRC_SRSR_WBI_MASK,     /*!< WARM boot indication shows that WARM boot
                                                              was initiated by software. */
#endif                                                   /* FSL_FEATURE_SRC_HAS_NO_SRSR_WBI */
    kSRC_TemperatureSensorResetFlag = SRC_SRSR_TSR_MASK, /*!< Indicates whether the reset was the
                                                              result of software reset from on-chip
                                                              Temperature Sensor. Temperature Sensor
                                                              Interrupt needs to be served before this
                                                              bit can be cleaned.*/
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_WDOG3_RST_B) && FSL_FEATURE_SRC_HAS_SRSR_WDOG3_RST_B)
    kSRC_Wdog3ResetFlag = SRC_SRSR_WDOG3_RST_B_MASK, /*!< IC Watchdog3 Time-out reset. Indicates
                                                          whether the reset was the result of the
                                                          watchdog3 time-out event. */
#endif                                               /* FSL_FEATURE_SRC_HAS_SRSR_WDOG3_RST_B */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_SW) && FSL_FEATURE_SRC_HAS_SRSR_SW)
    kSRC_SoftwareResetFlag = SRC_SRSR_SW_MASK, /*!< Indicates a reset has been caused by software
                                                    setting of SYSRESETREQ bit in Application
                                                    Interrupt and Reset Control Register in the
                                                    ARM core. */
#endif                                         /* FSL_FEATURE_SRC_HAS_SRSR_SW */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_JTAG_SW_RST) && FSL_FEATURE_SRC_HAS_SRSR_JTAG_SW_RST)
    kSRC_JTAGSystemResetFlag =
        SRC_SRSR_JTAG_SW_RST_MASK, /*!< Indicates whether the reset was the result of software reset form JTAG */
#endif                             /* FSL_FEATURE_SRC_HAS_SRSR_JTAG_SW_RST */
    kSRC_JTAGSoftwareResetFlag = SRC_SRSR_SJC_MASK,   /*!< Indicates whether the reset was the result of
                                                      setting SJC_GPCCR bit 31. */
    kSRC_JTAGGeneratedResetFlag = SRC_SRSR_JTAG_MASK, /*!< Indicates a reset has been caused by JTAG
                                                           selection of certain IR codes: EXTEST or
                                                           HIGHZ. */
    kSRC_WatchdogResetFlag = SRC_SRSR_WDOG_MASK,      /*!< Indicates a reset has been caused by the
                                                           watchdog timer timing out. This reset source
                                                           can be blocked by disabling the watchdog. */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_IPP_USER_RESET_B) && FSL_FEATURE_SRC_HAS_SRSR_IPP_USER_RESET_B)
    kSRC_IppUserResetFlag = SRC_SRSR_IPP_USER_RESET_B_MASK, /*!< Indicates whether the reset was the
                                                                 result of the ipp_user_reset_b
                                                                 qualified reset. */
#endif                                                      /* FSL_FEATURE_SRC_HAS_SRSR_IPP_USER_RESET_B */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_SNVS) && FSL_FEATURE_SRC_HAS_SRSR_SNVS)
    kSRC_SNVSFailResetFlag = SRC_SRSR_SNVS_MASK, /*!< SNVS hardware failure will always cause a cold
                                                      reset. This flag indicates whether the reset
                                                      is a result of SNVS hardware failure. */
#endif                                           /* FSL_FEATURE_SRC_HAS_SRSR_SNVS */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_CSU_RESET_B) && FSL_FEATURE_SRC_HAS_SRSR_CSU_RESET_B)
    kSRC_CsuResetFlag = SRC_SRSR_CSU_RESET_B_MASK, /*!< Indicates whether the reset was the result
                                                        of the csu_reset_b input. */
#endif                                             /* FSL_FEATURE_SRC_HAS_SRSR_CSU_RESET_B */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_LOCKUP) && FSL_FEATURE_SRC_HAS_SRSR_LOCKUP)
    kSRC_CoreLockupResetFlag = SRC_SRSR_LOCKUP_MASK, /*!< Indicates a reset has been caused by the
                                                          ARM core indication of a LOCKUP event. */
#endif                                               /* FSL_FEATURE_SRC_HAS_SRSR_LOCKUP */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_POR) && FSL_FEATURE_SRC_HAS_SRSR_POR)
    kSRC_PowerOnResetFlag = SRC_SRSR_POR_MASK, /*!< Indicates a reset has been caused by the
                                                    power-on detection logic. */
#endif                                         /* FSL_FEATURE_SRC_HAS_SRSR_POR */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_LOCKUP_SYSRESETREQ) && FSL_FEATURE_SRC_HAS_SRSR_LOCKUP_SYSRESETREQ)
    kSRC_LockupSysResetFlag =
        SRC_SRSR_LOCKUP_SYSRESETREQ_MASK, /*!< Indicates a reset has been caused by CPU lockup or software
                                               setting of SYSRESETREQ bit in Application Interrupt and
                                               Reset Control Register of the ARM core. */
#endif                                    /* FSL_FEATURE_SRC_HAS_SRSR_LOCKUP_SYSRESETREQ */
#if (defined(FSL_FEATURE_SRC_HAS_SRSR_IPP_RESET_B) && FSL_FEATURE_SRC_HAS_SRSR_IPP_RESET_B)
    kSRC_IppResetPinFlag = SRC_SRSR_IPP_RESET_B_MASK, /*!< Indicates whether reset was the result of
                                                           ipp_reset_b pin (Power-up sequence). */
#endif                                                /* FSL_FEATURE_SRC_HAS_SRSR_IPP_RESET_B */
#endif

/** Fetch the reset reason for the last system reset.
 *
 * This function must return the contents of the system reset reason registers
 * cast to an appropriate platform independent reset reason. If multiple reset
 * reasons are set, this function should return ::RESET_REASON_MULTIPLE. If the
 * reset reason does not match any existing platform independent value, this
 * function should return ::RESET_REASON_PLATFORM. If no reset reason can be
 * determined, this function should return ::RESET_REASON_UNKNOWN.
 *
 * This function is not idempotent; there is no guarantee the system
 * reset reason will not be cleared between calls to this function altering the
 * return value between calls.
 *
 * Note: Some platforms contain reset reason registers that persist through
 * system resets. If the registers haven't been cleared before calling this
 * function, multiple reasons may be set within the registers. If multiple reset
 * reasons are detected, this function returns ::RESET_REASON_MULTIPLE.
 *
 * @return enum containing the last reset reason for the board.
 */
reset_reason_t hal_reset_reason_get(void)
{
    reset_reason_t reason = RESET_REASON_UNKNOWN;

    uint32_t flags = SRC_GetResetStatusFlags(SRC);

    printf("reset flag: %lu\r\n", flags);

    if (flags & kSRC_IppResetPinFlag) {

        printf("reason: power on\r\n");

        reason = RESET_REASON_POWER_ON;

    } else {

        if ((flags & kSRC_Wdog3ResetFlag) || (flags & kSRC_WatchdogResetFlag)) {

            printf("reason: watchdog\r\n");

            reason = RESET_REASON_WATCHDOG;
        } else if (flags & kSRC_LockupSysResetFlag) {

            printf("reason: lockup\r\n");

            reason = RESET_REASON_LOCKUP;
        } else {

            printf("reason: unknown\r\n");

            reason = RESET_REASON_UNKNOWN;
        }
    }

    SRC_ClearResetStatusFlags(SRC, flags);

    return reason;
}
