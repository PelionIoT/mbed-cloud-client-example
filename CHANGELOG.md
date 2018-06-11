# Changelog for Mbed Cloud Client Reference Example

## Release 1.3.3 (11.06.2018)
* Updated to Mbed OS 5.8.5.
* The example application prints information about the validity of the stored Cloud credentials (including Connect and Update certificates).
* Added a start-up delay of two seconds as a workaround for the SD driver initialization [issue](https://github.com/ARMmbed/sd-driver/issues/93).
* Fixed the heap corruption that took place with `print_m2mobject_stats()` when building the code with the MBED_HEAP_STATS_ENABLED flag.
* Added the `-DMBED_STACK_STATS_ENABLED` flag. It enables printing information on the application thread stack usage.

## Release 1.3.2 (22.05.2018)
* Updated easy-connect to v1.2.9
* Updated to Mbed OS 5.8.4
* Added the `partition_mode` configuration. It is enabled by default and supposed to be used with a data storage, such as an SD card.
* Linux: Updated Mbedtls to 2.7.1 in pal-platform.
* Linux: Fixed CMake generation and performed generic cleanup in pal-platform scripts.

#### Platform Adaptation Layer (PAL)
* Linux: Converted all timers to use signal-based timer (`SIGEV_SIGNAL`) instead of (`SIGEV_THREAD`).
  * This fixes the Valgrind warnings for possible memory leaks caused by LIBC's internal timer helper thread.
    <span class="notes">**Note**: If the client application is creating a pthread before instantiating MbedCloudClient,
    it needs to block the PAL_TIMER_SIGNAL from it. Otherwise the thread may get an exception caused
    by the default signal handler with a message such as "Process terminating with default action
    of signal 34 (`SIGRT2`)". For a suggested way to handle this please see `mcc_platform_init()` in [here](https://github.com/ARMmbed/mbed-cloud-client-example/blob/master/source/platform/Linux/common_setup.c).</span>

## Release 1.3.1.1 (27.04.2018)
* No changes.

## Release 1.3.1 (19.04.2018)
* Converted LED blinking callback from a blocking loop to event based tasklet.
* Updated to Mbed OS 5.8.1.
* Rewrote platform-specific code to have common implementation which can be shared between other Cloud applications (source/platform/).
  * enabled multipartition support for application.
  * enabled LittleFS support.
  * enabled autoformat/autopartition for storage (controllable via compile-time flags).

## Release 1.3.0 (27.3.2018)

Initial public release.
