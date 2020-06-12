# Changelog for Pelion Device Management Client example application

## Release 4.5.0 (12.06.2020)

* Added support for MIMXRT1060-EVK board for NXP FreeRTOS SDK.
* Increased the Renesas RA6M3 Ethernet buffers from 1+1 to 4+4 to increase stability.
* Updated to Pelion E2E test library v0.2.6.
* Added a network error counter that resets the device if too many errors have occurred
* Added sleeping device example that is enabled with `MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE` option. The sleepy device will `pause` the client when client goes to sleep,
whenever application will try to send notification, if the client is `paused` then application will first `resume` client and then send notification. 
* Added support for Device Sentry feature for Mbed OS and Linux.
  * Mbed OS - the feature is enabled for K66F board.
  * Linux - the feature is enabled by passing cmake ENABLE_DEVICE_SENTRY flag to cmake.
* **Breaking changes** (Due to update of SE ATECC608A driver , the application is not compatible with previous releases of SE ATECC608A driver).
    * Updated SE ATECC608A driver `COMPONENT_ATECC608A.lib`.
    * Updated mbed-cloud-client-platform-common `platform.lib` - includes adaptation for new SE ATECC608A driver.
* Updated `EK-RA6M3` to use Renesas `Arm® Secure Boot Solution for RA6M3 MCU Group` version 1.1.0.
* [Mbed OS] Added explicit Device Key generation. This is for future-compatibility with Mbed OS 6 and is only in use with Mbed OS 6 builds.

## Release 4.4.0 (17.04.2020)

* Added support for NXP FreeRTOS SDK.
  * The supported board is `LPCXpresso54628`.

      <span class="notes">**Note**: Select the correct QSPI chip in the board configuration file (`define_NXP_LPC54628.txt`). This depends on the board revision.</span>

* Added support for Renesas FreeRTOS SDK.
  * The supported board is `EK-RA6M3`.
* Replaced Icetea-based test framework and tests with the Pelion E2E Test Library v0.2.5.
* Updated to Mbed OS 5.15.1.
* Added a sleep callback function on application to indicate when client goes to sleep using `MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE` mode.
* Changed the default DTLS fragmentation to 1024 bytes for Wi-SUN mesh.
* Removed application dependency for int64 printf support which is not present in the nanolib-C library.
* Restricted the size of applications using internal storage on Mbed OS builds.
* pal-platform: Fixed an issue in `pal-platform.json` due to an incompatibility with `click` version later than 7.0.

## Release 4.3.0 (06.02.2020)

* Made the button counter resource writable to allow resetting it.
* Mbed OS platform setup now uses the new SocketAddress-based APIs for IP address printing.
* Removed `configs/eth_v4.json` and `configs/eth_v6.json`. Dropping configuration for `Ublox EVK Odin W2` Ethernet and explicit `IPv6` Ethernet support. The rest of the configurations are in the root-level `mbed_app.json`.
* Removed the duplicate configurations in `configs/wifi.json`. All configurations are already in `mbed_app.json`.
* Removed support for `6LoWPAN` and `Thread` Mesh configurations. The application will focus on providing `Wi-SUN` Mesh support.
* Added support for Atmel secure element (ATECC608A) with K64F target board (`configs-psa/eth_v4_with_se_atmel.json`). Atmel SE holds pre-provisioned bootstrap key and certificate, that Device Management Client uses for secure connection with Device Management.
* Wi-Fi driver ISM 43362 (`.lib`) updated to pull in the latest release from the master of the driver.
    * The SocketAddress-based `get_ip_address()` API will not work with older versions.

## Release 4.2.1 (20.12.2019)

No changes.

## Release 4.2.0 (18.12.2019)

* Added support for Nucleo F303RE + ESP8266 + SPI-flash. Firmware update is not yet supported on this target due to data corruption issues during firmware download.
* Added a timer that automatically increments the value of the button press resource. This allows testing the example application on devices with no hardware buttons.
* On Mbed OS 5.15, added support for all Wi-SUN channel configuration settings for testing purposes to the Mbed OS
  mesh configuration interface. You can use the settings for test configurations, for example for fixed channel mode. Do not use them on normal Wi-SUN operation. Removed unnecessary Wi-SUN channel configuration settings from the Wi-SUN configuration file.
* Updated to Mbed OS 5.15.0.
* Extended the mesh network configuration with a new macro `STARTUP_MIN_RANDOM_DELAY` to supplement the existing `STARTUP_MAX_RANDOM_DELAY`. The new macro defaults to `STARTUP_MAX_RANDOM_DELAY/4`. You can use these configurations to provide mesh networks more time to properly stabilize before the client application starts its registration flow. For Mbed OS, you can use the configuration options `client_app.startup_min_random_delay` and `client_app.startup_max_random_delay` to define `STARTUP_MIN_RANDOM_DELAY` and `STARTUP_MAX_RANDOM_DELAY` in the application configuration. See `configs/mesh_wisun.json` for an example.

## Release 4.1.0 (28.11.2019)

* [Mbed OS] Replace `X-Nucleo IDW01M1` with `ESP8266` Wi-Fi module in `mbed_app.json`.
* Added reset pin for ESP wifi module on Nucleo F411RE configuration.
* Added flow control and reset pins for ESP wifi module on LPC55S69 configuration.
* Updated bootloaders to v4.1.0. `LPC55S69_NS` bootloader is retained in v4.0.1.
* [Mbed OS] Optimize `Wi-SUN` configuration for 100-node networks.
* Increase application default lifetime to 24 hours (`mbed_cloud_client_user_config.h´).
* Updated to Mbed OS 5.14.2.

## Release 4.0.0 (25.09.2019)

* Updated to Mbed OS 5.14.0.
* [Linux] Updated Mbed TLS to 2.19.1.
* The example application initialization is now done in multiple steps. This ensures the succcessful initialization of the Device Management Client library. It needs to be properly initialized before the memory intensive components (for example nanostack mesh stack).
* Removed `CY8CKIT_062_WIFI_BT_PSA`, which is no longer supported starting from Mbed OS 5.13.1.
* [Linux] Fixed busy-loop when `stdin` is not connected.
* Added default behavior for optionally created `Factory Reset` Resource located under the `Device` Object `/3/0/5`.
* You can overwrite both `Reboot` and `Factory Reset` Resource default behavior by implementing corresponding handler functions.
* Added support for Discovery `L475VG IOT01A` target board (`configs/wifi.json`).
* Added PSA support for `NUCLEO_F411RE` target board (`configs-psa/wifi.json`).
* Added PSA support for Linux (`define_linux_psa.txt`).
* Changed `NUCLEO_F411RE` Wi-Fi configuration to use ESP8266 module instead of the deprecated X-Nucleo IDW01M1.
* Unified Wi-Fi configuration files. `configs/wifi.json` and `configs-psa/wifi.json` now include all supported Wi-Fi configurations except the minimum configuration example, which is still in `configs/wifi_esp8266_minimal.json`.
* New application feature to introduce random delay for Device Management Client registration after the network connection has been established. You can use this feature to stabilize large mesh-type networks with high latency and limited bandwidth. By default, it is enabled for mesh-type networks and disabled for other configurations. To enable the random delay, define `STARTUP_MAX_RANDOM_DELAY` in seconds.

## Release 3.4.0 (15.08.2019)

* Added PSA configuration for K66F (`configs-psa/eth_v4.json`).
* Updated usage of new Update Authorization API, which takes in priority as well, `set_update_authorize_priority_handler` instead of `set_update_authorize_handler`.
* Use `set_message_delivery_status_cb` as part of unregister resource triggering to make sure device does not close the network connection before client is able to send the final ACK to server. 
* [Linux] Updated Mbed TLS to 2.18.1.
* [Mbed OS] Removed the legacy ESFS-SOTP configurations from the applications. Only KVstore is supported for client storage.

## Release 3.3.0 (02.07.2019)

* Updated to Mbed OS 5.13.0.
* Added support for Wi-SUN mesh on Nucleo F429ZI (`configs/mesh_wisun.json`).
* Added PSA support for Linux PC.
  * PSA configuration (`define_linux_psa.txt`) for PSA-enabled Linux PC.

## Release 3.2.0 (07.06.2019)

* Updated to Mbed OS 5.12.4.
* Added delta tool to the example application.

## Release 3.1.1 (13.05.2019)

* Updated to Mbed OS 5.12.3.
* Fixed the application initialization issue in production flow. This fixes a regression caused by 3.1.0 release.
* Fixed a compilation issue of Nucleo F429ZI board with the root-level `mbed_app.json`.

## Release 3.1.0 (26.04.2019)

* Updated to Mbed OS 5.12.1.
* [Linux] Updated Mbed TLS to 2.17.0.
* General optimization on default stack-sizes and TLS configurations for all configurations.
  * Dropped the lwIP buffers from 32 KiB to 12 KiB.
  * Flash savings from 7 KiB to 20 KiB.
  * RAM savings from 7 KiB to 34 KiB.
* Introduced new `configs/wifi_esp8266_minimal.json` which demonstrates the bare minimum needed for full client functionality.
  * The configuration uses shared event-queue from main thread, instead of using a separate thread for events. This saves 10.5 KiB RAM.
  * New application flag `MCC_MINIMAL` disables several features only designed to enhance user experience, and removes other non-critical features.
  * Disabled Button and LED usage by default.
  * Disabled error descriptions.
  * Disabled usage of some helper functions like `fcc_verify()`.
  * Resources generated by the application are disabled on this profile.
  * Changed the KVStore and firmware candidate storage to internal flash configuration. This saves 4 KiB RAM and 34 KiB ROM as you don't need to pull in SDDriver or related components.
  * Altogether, these optimizations save 43 KiB flash and 31 KiB RAM in respect to the `configs/wifi_esp8266_v4.json` profile. (49 KiB flash and 39 KiB RAM vs. 3.0.0 release version).
  * Currently, the minimal profile supports only TCP.
* Minimal configuration is optimimal for boards with about 1024 KiB of flash. If you use it with boards with only 512 KiB flash, you need to enable the external storage for a firmware update candidate and KVstore. The following changes would be needed:
  * Change the bootloader to the `block_device` variant.
  * Change the target offsets to match the `block_device` bootloader.
  * Change the update storage to point to the external storage.
  * Change the KVstore configuration to match the external storage configuration.
  * See the [porting guide](https://www.pelion.com/docs/device-management/current/porting/index.html) for reference.
* K66F default configuration now provides the full Client feature. This target enables all the RAM/ROM intensive features by default.
  * Enabled certificate enrollment client features.
  * Increased the main stack size to reflect higher RAM requirements for features.
* [MbedOS] Stop using `CellularBase()` as it is deprecated in Mbed OS, use `CellularInterface()` instead.

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
