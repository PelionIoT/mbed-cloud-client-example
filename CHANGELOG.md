# Changelog for Mbed Cloud Client Reference Example

## Release 1.3.2 (22.05.2018)
* Update easy-connect to v1.2.9
* Update to Mbed OS 5.8.4
* Configuration partition_mode is added, which is enabled by default. This is supposed to be used with storage for data e.g. SD card.
* Linux: Update Mbedtls to 2.7.1 in pal-platform.
* Linux: Fix for CMake generation and generic cleanup in pal-platform.

#### Platform Adaptation Layer (PAL)
* Linux: Converted all timers to use signal-based timer (SIGEV_SIGNAL) instead of (SIGEV_THREAD).
  * This fixes the Valgrind warnings for possible memory leaks caused by LIBC's internal timer helper thread.
    <span class="notes">**Note**: If the client application is creating a pthread before instantiating MbedCloudClient,
    it needs to block the PAL_TIMER_SIGNAL from it. Otherwise the thread may get an exception caused
    by the default signal handler with a message such as "Process terminating with default action
    of signal 34 (SIGRT2)". For a suggested way to handle this please see `mcc_platform_init()` in
    https://github.com/ARMmbed/mbed-cloud-client-example/blob/master/source/platform/Linux/common_setup.c.</span>

## Release 1.3.1.1 (27.04.2018)
* No changes.

## Release 1.3.1 (19.04.2018)
* Convert LED blinking callback from a blocking loop to event based tasklet.
* Update to Mbed OS 5.8.1.
* Rewrite platform-specific code to have common implementation which can be shared between other Cloud applications (source/platform/).
  * enable multipartition support for application.
  * enable LittleFS support.
  * enable autoformat/autopartition for storage (controllable via compile-time flags).

## Release 1.3.0 (27.3.2018)

Initial public release.

