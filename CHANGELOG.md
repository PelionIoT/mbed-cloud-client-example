# Changelog for Pelion Device Management Client example application

## Release 3.0.0 (27.03.2019)

* Updated to Mbed OS 5.12.0.
* [Mbed OS] Use asyncronous DNS by default for all targets.
* [Mbed OS] Preview support for Platform Security Arhitecture (PSA) enabled boards. 
   * PSA configuration for PSA-enabled Cypress PSoC6 and NXP LPC55S69 boards. Configuration is in the `configs-psa/` folder.
   * Both PSA-enabled boards use ESP8266 Wi-Fi.
   * PSA configuration for K64F board.

## Release 2.2.1 (28.02.2019)

* Updated to Mbed OS 5.11.5.
* Added KVStore support for Nucleo F411RE.
* Changed the board configuration for Nucleo F429ZI to use internal flash profile.
* Added bootloaders for the `legacy_storage` configurations as part of the application repository. Explicitly defined the target offsets for each bootloader to provide more clarify on the board configuration.
* By default, `MBED_CLOUD_CLIENT_STL_API` is set to `0` in `mbed_lib-json`. This disables the deprecated client APIs completely in `SimpleM2MResourceString` and `SimpleM2MResourceInt` classes. `MBED_CLOUD_CLIENT_STD_NAMESPACE_POLLUTION` is `0` also by default. It disables the namespace pollution of code that includes `MbedCloudClient.h` with using `namespace std;`. With these changes you can save extra ROM overhead of ~15 KB on ROM budget, depending on the compiler toolchain used.
* Added Python3 support for Device Management end-to-end tests.
  * Updated the minimum required version of Mbed Python SDK to 2.0.5.
  * Updated the minimum Icetea version to 1.2.1.

## Release 2.2.0 (25.02.2019)

* Updated to Mbed OS 5.11.4.
* [Mbed OS] Client 2.2.0 has fixed the internal initialization of `ns_hal_init()`, which fixes the issue of double initialization of memory when using Mesh network stacks.
  * Removed the application configurations of `mbed-client.event-loop-size` for most configurations. The application uses the default configuration defined in `mbed-client`, or from Mbed OS for Mesh network stacks.
  * For Wi-Fi stacks, `mbed-client.event-loop-size` needs to be 2048.
* [Linux] Updated CMake minimum version to 3.5.
* [pal-platform] Deprecated the fullBuild option in build tools.
* [Mbed OS] By default, application configuration updated to use secure storage implementation through KVstore.
  * K66F now uses internal flash for both storing client credentials and the update image.
  * All other boards have been changed to use LittleFS instead of FAT file system by default. FAT file system is not power-loss resilient and should not be used in embedded devices.
  * Old SOTP-ESFS based storage configuration files are still available in the `configs/legacy_storage` folder.
  * `NUCLEO F411RE` board still uses SOTP-ESFS implementation, as non-TRNG boards are not yet supported in new KVStore-based storage.
  * Legacy Wi-Fi configuration `wifi_esp8266_v4_legacy.json` introduced in the 2.1.1 release has been removed.
* Moved the network `init` call to `main()` and added network disconnect call to end of `main()`.
* Removed extern block evice reference from `platform-common`.
* Added `TESTS/PAL/test.py` script to help run PAL porting tests.
* Added basic Device Management acceptance end-to-end tests that use the [Icetea](https://github.com/ARMmbed/icetea) test framework.

## Release 2.1.1 (19.12.2018)

* Updated to Mbed OS 5.11.0.
* Modified the Wi-Fi configuration `configs/wifi_esp8266_v4.json` to support the [new ESP8266 Wi-Fi chip](https://www.esp8266.com/wiki/doku.php?id=esp8266-module-family) with 2 MB or more flash and [ESP8266 AT firmware](https://www.espressif.com/en/support/download/at?keys=) 1.7.0, with serial flow control (`UART0_CTS`, `UART0_RTS`) pins and reset pin (`EXT_RSTB`) connected. For production devices, we recommend that you upgrade your ESP8266 to this version and connect the control pins to corresponding pins on the target hardware.
* Previous Wi-Fi ESP866 is supported in the new `configs/wifi_esp8266_v4_legacy.json` file.

    <span class="notes">**Note**: With legacy ESP8266 Wi-Fi with 1 MB flash, the minimum supported firmware version is now 1.6.2. **Without flow control and reset pin connected, the device will eventually disconnect and fail to re-connect.** ESP8266 firmware has to be version 1.7.0 (or later) to enable flow control and only boards with 2 MB flash or more can take that firmware in.</span>
* [Linux] Updated Mbed TLS to 2.15.1.

## Release 2.1.0 (11.12.2018)

* Updated to Mbed OS 5.10.3.

## Release 2.0.1 (12.10.2018)

* The application uses `wait_ms(int)` instead of `wait(float)`. This saves a bit on the application size.
* [Mbed OS] The application prints NetworkInterface status over the console.
* Updated to Mbed OS 5.10.1.

## Release 2.0.0 (26.09.2018)

* Updated to Mbed OS 5.10.0.
* **Breaking changes** (Due to integration of storage and networking drivers to Mbed OS 5.10 and the introduction of new APIs, the application is not compatible with previous releases of Mbed OS).
    * Changed the application to use Mbed OS 5.10 network interfaces directly and removed the dependency to `easy-connect.lib`.
    * Changed the application to use Mbed OS 5.10 storage interfaces directly and removed the dependency to `storage-selector.lib`.
    * The example uses Mbed OS bootloader binaries and the new Mbed OS feature `FEATURE_BOOTLOADER`. This feature makes the `combine_bootloader_with_app.py` script obsolete. We have removed the obsolete script and old bootloader binaries from the application repository.
* The bootloader is now automatically combined with the application binary at compile time:
    * `mbed-cloud-client-example.bin` is the binary you need to flash on the development board.
    * You can use `mbed-cloud-client-example_update.bin` for the firmware update as long as the prequisites for firmware update are fullfilled. (See the application tutorial).
* Removed the legacy configuration file `configs/eth_v4_legacy.json` that was used for Mbed Cloud Client 1.2.6.
* Removed the `.autostart` configurations used by the online compiler.
* Enabled serial buffer in all `.json` files.
    * This improves the reliability of the connection with tracing enabled, for example Nucleo F429ZI had connectivity problems with UDP and traces enabled.
* Increased LWIP buffers for all STM-based targets to improve the stability of DTLS connection.
* Disabled hardware acceleration for Ublox EVK Odin W2 and Nucleo F411RE. See Mbed OS [issue](https://github.com/ARMmbed/mbed-os/issues/6545).
* [LINUX] Updated Mbed TLS to 2.13.1.

## Release 1.5.0 (11.9.2018)

* Added a hardcoded RoT injection when application is configured to use developer mode. This preserves the Device Management Client credentials even when SOTP is erased (for example due to reflashing of the application binary).
* Updated to Mbed OS 5.9.6.
* Updated easy-connect to v1.2.16.
* Updated the storage selector with compiler warning fixes in the internal libraries.
* Replaced the notification delivery status functionality with a more generic message delivery status callback.
* Added an example on using the delayed response for execute operations.
* Added configurations for the K66F target board.


## Release 1.4.0 (13.07.2018)

* Increased application main stack-size to 5120 to fix Stack overflow with ARMCC compiled binaries when tracing is enabled.
* Linux: Updated Mbed TLS to 2.10.0 in `pal-platform`.
* Updated to Mbed OS 5.9.2.
* Updated easy-connect to v1.2.12.
* Updated storage-selector with support for SPI flash frequency, and Nucleo F411RE board.
* Moved the initialization of `mbed_trace()` as the first step in the application initialization. This allows the printing of any early debug information from the PAL layer when debug tracing is enabled.
* Added support for Nucleo F411RE board with Wifi-X-Nucleo shield.
* Increased the event loop thread stack size to 8192 bytes from the default value of 6144. This fixes stack overflows in some cases where crypto operations result in deep callstacks.

## Release 1.3.3 (08.06.2018)

* Updated to Mbed OS 5.8.5.
* The example application prints information about the validity of the stored Cloud credentials (including Connect and Update certificates).
* Added a start-up delay of two seconds as a workaround for the SD driver initialization [issue](https://github.com/ARMmbed/sd-driver/issues/93).
* Fixed the heap corruption that took place with `print_m2mobject_stats()` when building the code with the `MBED_HEAP_STATS_ENABLED` flag.
* Added the `-DMBED_STACK_STATS_ENABLED` flag. It enables printing information on the application thread stack usage.

## Release 1.3.2 (22.05.2018)

* Updated easy-connect to v1.2.9.
* Updated to Mbed OS 5.8.4.
* Added the `partition_mode` configuration. It is enabled by default and supposed to be used with a data storage, such as an SD card.
* Linux: Updated Mbed TLS to 2.7.1 in pal-platform.
* Linux: Fixed CMake generation and performed generic cleanup in pal-platform scripts.

#### Platform Adaptation Layer (PAL)

* Linux: Converted all timers to use signal-based timer (`SIGEV_SIGNAL`) instead of (`SIGEV_THREAD`).
  * This fixes the Valgrind warnings for possible memory leaks caused by LIBC's internal timer helper thread.
  
      <span class="notes">**Note**: If the client application is creating a pthread before instantiating MbedCloudClient,
    it needs to block the `PAL_TIMER_SIGNAL` from it. Otherwise, the thread may get an exception caused
    by the default signal handler with a message such as "Process terminating with default action
    of signal 34 (`SIGRT2`)". For a suggested way to handle this, see `mcc_platform_init()` in [the code](https://github.com/ARMmbed/mbed-cloud-client-example/blob/master/source/platform/Linux/common_setup.c).</span>

## Release 1.3.1.1 (27.04.2018)

* No changes.

## Release 1.3.1 (19.04.2018)

* Converted LED blinking callback from a blocking loop to event based tasklet.
* Updated to Mbed OS 5.8.1.
* Rewrote platform-specific code to have common implementation that can be shared between other Cloud applications (source/platform/).
  * Enabled multipartition support for application.
  * Enabled LittleFS support.
  * Enabled autoformat/autopartition for storage (controllable via compile-time flags).

## Release 1.3.0 (27.3.2018)

Initial public release.
