<h1 id="zephyr">Command-line tutorial for the Device Management Client example with Zephyr OS</h1>

This is a Device Management Client example, written using Device Management Client 4.14.0 and [Zephyr OS 3.6.0](https://www.zephyrproject.org/).

## Supported boards

Supported boards have:

- At least 1 MiB total storage:
   - 512 KiB for application, bootloader and credential storage.
   - 512 KiB for firmware update.
- 256 KiB RAM.
- These Zephyr APIs implemented:
   - [POSIX Network API](https://docs.zephyrproject.org/3.6.0/reference/networking/sockets.html).
   - [Flash map](https://docs.zephyrproject.org/3.6.0/reference/storage/flash_map/flash_map.html).

You can use this tutorial with the example application [mbed-cloud-client-example](https://github.com/PelionIoT/mbed-cloud-client-example) and the following boards:

| Board               | Example application                                                                                                     | Connectivity              |
| ---                 | ---                                                                                                                     | ---                       |
| [frdm_k64f](https://os.mbed.com/platforms/FRDM-K64F/) | [mbed-cloud-client-example](https://github.com/PelionIoT/mbed-cloud-client-example)     | Ethernet                  |
| [nucleo_h753zi](https://www.st.com/en/evaluation-tools/nucleo-h753zi.html) | [mbed-cloud-client-example](https://github.com/PelionIoT/mbed-cloud-client-example)     | Ethernet                  |

For third party example applications and boards, use the following tutorials:

| Board               | Example application                                                                                                     | Connectivity              |
| ---                 | ---                                                                                                                     | ---                       |
| [nrf52840dk_nrf52840](https://www.nordicsemi.com/products/nrf52840) | [pelion_client](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/applications/pelion_client/README.html) | OpenThread mesh           |
| [nrf5340dk_nrf5340](https://www.nordicsemi.com/products/nrf5340) | [pelion_client](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/applications/pelion_client/README.html) | OpenThread mesh           |
| [nrf9160dk_nrf9160](https://www.nordicsemi.com/products/nrf9160) | [pelion_client](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/1.8.0/nrf/applications/pelion_client/README.html) | Cat-M1 or NB-IoT cellular |


## Prerequisites

To work with the Device Management Client example application, you need:

- A [Zephyr OS supported board](https://docs.zephyrproject.org/3.6.0/boards/index.html) with a network connection and [enough flash and RAM](https://developer.izumanetworks.com/docs/device-management/current/connecting/embedded-systems-examples.html).
- Serial or debugger connection to your device for [accessing the log](https://docs.zephyrproject.org/3.6.0/reference/logging/index.html).
- Zephyr's command-line tool, [West](https://docs.zephyrproject.org/3.6.0/guides/west/index.html).
- A [toolchain](https://docs.zephyrproject.org/3.6.0/getting_started/index.html#install-a-toolchain) matching your development board. 
- An Izuma Cloud account, if you do not have an account - [contact us](https://www.izumanetworks.com/#contact-us).
- Device Management's command-line [manifest-tool](https://github.com/PelionIoT/manifest-tool) version 2.4.1 or later for firmware updates.

   <span class="notes">**Note:**  If your host system Python version is 3.6, use `manifest-tool` version 2.4.1, use version 2.5.0 (or newer) for more up-to-date system. You can specify version via: `pip install manifest-tool==2.4.1`. </span>

<h2 id="qs-configuring">Configuring Device Management Client</h2>

1. Initialize a new Zephyr workspace using the manifest from `mbed-cloud-client-example`:

   ```
   west init -m https://github.com/PelionIoT/mbed-cloud-client-example zephyr-workspace
   cd zephyr-workspace
   cd izuma-dm-example; git checkout 4.14.0-dev; cd -
   west update
   ```
   
   This creates the following workspace on your computer:
   
   ```
   zephyr-workspace
   ├── bootloader
   ├── modules
   │   └── lib
   │       └── izuma-dm (middleware library)
   ├── izuma-dm-example (example application)
   │   ├── pal-platform
   │   │   └── SDK
   │   │       └── ZephyrOS (Zephyr project)
   │   │           └── boards (example overlays)
   │   └── west.yml (West manifest)
   ├── tools
   └── zephyr
   ```

1. [Download a developer certificate from Device Management Portal](../provisioning-process/provisioning-development-devices.html#creating-and-downloading-a-developer-certificate).

1. Copy the `mbed_cloud_dev_credentials.c` file to the `izuma-dm-example` directory in your workspace.

1. [Get an access key](../user-account/application-access-keys.html) for your Device Management account.

1. Create update-related configuration and credentials in a new, empty directory using the [`manifest-tool`](https://github.com/PelionIoT/manifest-tool):

   ```
   mkdir manifest
   cd manifest
   manifest-dev-tool init --access-key <Device Management access key>
   ```
   
   This directory now contains a private key suitable for authorizing firmware updates for devices provisioned with the public key and configuration stored in the file `update_default_resources.c`.
    
1. Copy the `update_default_resources.c` file from the `manifest` directory to the `pelion-dm-example` directory.
    
<span class="notes">**Note:** When you create a firmware update image for a deployed device, you must use the same update-related configuration and credentials (update private key, public key certificate, `update_default_resources.c` and configuration files) you used in the original device firmware image.</span>

**We highly recommend creating a Python virtual env for Zephyr SDK**. Do install also the Zephyr python requirements.
```
python3 -m venv venv
source venv/bin/activate
pip install -r zephyr/scripts/requirements.txt
```

<h2 id="qs-compiling">Compiling and flashing Device Management Client on Freedom-K64F</h2>

1. Build the MCUboot bootloader using the memory overlay file `frdm_k64f_mcuboot.overlay` by executing this command from the top-level `zephyr-workspace` directory:

   ```
   west build -p always -b frdm_k64f -d build/mcuboot -s bootloader/mcuboot/boot/zephyr -- \
   -DDTC_OVERLAY_FILE="$PWD/izuma-dm-example/pal-platform/SDK/ZephyrOS/boards/mcuboot/frdm_k64f_mcuboot.overlay" \
   -DCONFIG_MCUBOOT_CLEANUP_ARM_CORE=y
   ```

   <span class="notes">**Note:** See the [flash layout section](#flash-layout) for more about overlay files.</span>

1. Build and sign the example application:

   ```
   west build -p always -b frdm_k64f -d build/dmc -s izuma-dm-example -- \
   -DCONFIG_BOOTLOADER_MCUBOOT=y \
   -DDTC_OVERLAY_FILE="$PWD/izuma-dm-example/pal-platform/SDK/ZephyrOS/boards/frdm_k64f.overlay"
   
   mkdir keys
   imgtool keygen -k keys/dev_ecdsa.pem -t ecdsa-p256

   west sign -d build/dmc -t imgtool -- \
   --key keys/dev_ecdsa.pem \
   --version 1.0.0
   ```

## Flashing the binary to the Freedom-K64F device

To flash the bootloader and application to the device:

   ```
   # mcuboot
   west flash -d build/mcuboot --runner pyocd
   
   # dmc
   pyocd flash -t k64f build/dmc/zephyr/zephyr.signed.hex
   ```

<h2 id="qs-compiling">Compiling and flashing Device Management Client on Nucelo-H753ZI</h2>

1. Build the MCUboot bootloader using the memory overlay file `nucleo_h753zi_mcuboot.overlay` by executing this command from the top-level `zephyr-workspace` directory:

   ```
   west build -p always -b nucleo_h753zi -d build/mcuboot -s bootloader/mcuboot/boot/zephyr -- \
   -DDTC_OVERLAY_FILE="$PWD/izuma-dm-example/pal-platform/SDK/ZephyrOS/boards/mcuboot/nucleo_h753zi_mcuboot.overlay" \
   -DCONFIG_MCUBOOT_CLEANUP_ARM_CORE=y
   ```

   <span class="notes">**Note:** See the [flash layout section](#flash-layout) for more about overlay files.</span>

1. Build and sign the example application:

   ```
   west build -p always -b nucleo_h753zi -d build/dmc -s izuma-dm-example -- \
   -DOVERLAY_CONFIG="$PWD/izuma-dm-example/pal-platform/SDK/ZephyrOS/boards/nucleo_h753zi.conf" \
   -DDTC_OVERLAY_FILE="$PWD/izuma-dm-example/pal-platform/SDK/ZephyrOS/boards/nucleo_h753zi.overlay" \
   -DCONFIG_BOOTLOADER_MCUBOOT=y
   
   mkdir keys
   imgtool keygen -k keys/dev_ecdsa.pem -t ecdsa-p256

   west sign -d build/dmc -t imgtool -- \
   --key keys/dev_ecdsa.pem \
   --version 1.0.0
   ```

## Flashing the binary to the Nucelo-H753ZI device

To flash the bootloader and application to the device:

   ```
   # mcuboot
   west flash -d build/mcuboot -r openocd
   
   # dmc
   pyocd flash -t stm32h753zitx build/dmc/zephyr/zephyr.signed.hex
   ```

<h2 id="qs-connecting">Connecting and performing a firmware update on your device</h2>

### Checking the device connection and obtaining the device ID

Obtain the Device ID either from device console logs or from [Device Management Portal](https://portal.mbedcloud.com).

When the client has successfully connected, the terminal shows:

   ```
   Client registered
   Endpoint Name: <Endpoint name>
   Device ID: <Device ID>
   ```

To verify the connection with Device Management Portal:

1. Log in to Device Management Portal for your region:

   - [United States](https://portal.mbedcloud.com/).

1. Select **Device directory** from the menu on the left.

   When the **Devices** page lists the device as registered, the device is connected and available.

The device is now ready for firmware update. For development devices, the **endpoint name** and **device ID** are identical.

## Updating the firmware

To update the firmware on the device, copy the file `zephyr.signed.bin` from the build folder `build/dmc/zephyr` to the `manifest` directory created earlier, which contains your update credentials. To start a firmware update campaign, run:

  ```
  manifest-dev-tool update-v1 \
  --payload-path zephyr.signed.bin \
  --device-id <Device ID> \
  --wait-for-completion
  ```

This command:

   1. Uploads the new firmware to Device Management. 
   1. Creates and signs a manifest file describing the firmware.
   1. Creates and starts a firmware update campaign for the given device ID, using the manifest.
   1. Waits for the campaign to finish and reports the result.
    
During the update flow, the client tracing log shows:

```
Firmware download requested
Authorization granted
Downloading: 1 %
...
Downloading: 100 %
Download completed
Firmware install requested
Authorization granted
```

After this, the device reboots automatically and registers to Device Management.

<span class="notes">**Note:** Device Management Client for Zephyr OS currently only supports firmware update features from version 4.7. Please refer to the [version 4.7 documentation](https://developer.izumanetworks.com/docs/device-management/v4.7/updating-firmware/index.html) when using firmware update.</span>

<h2 id="user-application">User applications</h2>

To add Device Management Client to an existing project, add this repository and module to the [West manifest](https://docs.zephyrproject.org/3.6.0/guides/west/manifest.html):

  ```
  remotes:
    - name: PelionIoT
      url-base: https://github.com/PelionIoT
  projects:
    - name: izuma-dm
      repo-path: mbed-cloud-client
      remote: PelionIoT
      revision: master
      path: modules/lib/izuma-dm
  ```

<h2 id="flash-layout">Flash layout</h2>

Zephyr OS abstracts internal and external storage devices into isolated, labeled partitions. Each starts at address 0x0 regardless of the real, underlying storage medium.

The bootloader, MCUboot, is stored in the partition labeled `mcuboot`. The active application is executed from `image_0`, and the candidate firmware is stored in `image_1`. Izuma Device Management Client uses the partition `pelion_storage` to store configuration and credentials. If `pelion_storage` doesn't exist, Izuma Device Management Client uses the `storage` partition instead, if Zephyr isn't already using it (CONFIG_SETTINGS=n).

Depending on the architecture, you can put "image_0" and "image_1" on external storage. For better security, keep `pelion_storage` in internal storage.

For more information about Zephyr's storage partitions, see [the Zephyr Project documentation](https://docs.zephyrproject.org/3.6.0/reference/storage/flash_map/flash_map.html#relationship-with-devicetree).

As a reference, we've provided a partition overlay for the target `frdm_k64f` in the `pelion-dm-example/pal-platform/SDK/ZephyrOS/boards` directory, using this flash layout:

  ```
  +--------------------------+
  |                          |
  |         TDBStore         | <-+ "izuma_storage"
  |                          |
  +--------------------------+
  |                          |
  |      MCUboot scratch     | <-+ "image_scratch"
  |                          |
  +--------------------------+
  |                          |
  |                          |
  |Firmware candidate storage| <-+ "image_1"
  |                          |
  |                          |
  +--------------------------+
  |                          |
  |                          |
  |        Active app        | <-+ "image_0"
  |                          |
  |                          |
  +--------------------------+
  |                          |
  |      Zephyr storage      | <-+ "storage"
  |                          |
  +--------------------------+
  |                          |
  |        Bootloader        | <-+ "mcuboot"
  |                          |
  +--------------------------+
  ```

<h2 id="mcuboot">Bootloader</h2>

Zephyr OS uses [MCUboot](https://mcuboot.com/) as a bootloader.

The example application installs new firmware as "permanent" by default. To use MCUboot's rollback feature, use Kconfig to set `CONFIG_IZUMA_UPDATE_PERMANENT=n` and [Zephyr's DFU commands](https://docs.zephyrproject.org/3.6.0/guides/device_mgmt/dfu.html) for interacting with MCUboot.

<span class="notes">**Note:** The build instructions use the default developer keys in MCUboot. For production devices, use [secure credentials instead](https://docs.zephyrproject.org/3.6.0/guides/west/sign.html).</span>

<h2 id="pal">Platform Abstraction Layer (PAL)</h2>

The PAL implementation uses these Zephyr subsystems in the implementation:

- RTOS:

   - Threads are statically allocated using `K_THREAD_STACK_ARRAY_DEFINE` with the maximum number of threads and stack sizes set with the compile definitions `PAL_THREADS_MAX_COUNT` and `PAL_STACKS_MAX_SIZE`, respectively.
   - Timers use kernel work queues for scheduling timeouts and invoking callbacks.

- Network:

   - Sockets are implemented using Zephyr's POSIX API and are statically allocated with the compile definition `PAL_SOCKET_MAX` setting the maximum number of sockets.
   - PAL DNS API version 0 is implemented using POSIX getaddrinfo.
   - PAL DNS API version 3 is implemented using DNS Resolve.
   - Asynchronous callbacks are signaled through kernel work queues.

- Storage:

   - Device Management Client's `FlashIAP` API is mapped through Zephyr's `FlashMap` API.
   - Device Management Client instantiates multiple `FlashIAP` drivers to map between Izuma's absolute addresses and Zephyr's partions.

<h2 id="kconfig">Advanced Kconfig options</h2>

To activate the Kconfig, you need to be in the `izuma-dm-example` folder and run:

```
west build -t menuconfig --board=frdm_k64f
```

You can find the configuration options in the Kconfig menu under `modules` -> `izuma-dm`. Alternatively, you can pass configuration options on the command-line or set them permanently in the projects configuration file.

Once you have done your changes, remember to `S` save the configuration. Delete the `build` folder and rebuild.

| Configuration option | Type | Default setting | Description |
| --- | --- | --- | --- |
| `CONFIG_IZUMA_ENDPOINT_TYPE` | String | "default" | Endpoint type name:<br> Optional name for the type of endpoint being connected. Useful for creating filters and organizing devices. |
| `CONFIG_IZUMA_LIFETIME` | Integer | 86400 | Registration lifetime:<br> Interval in seconds between registration renewals. A device transitions from state "Registered" to "Deregistered" if the registration is not renewed within this lifetime interval. Recommended lifetime settings are in the range of multiple hours and days. |
| Choose one of:<br>- `CONFIG_IZUMA_TRANSPORT_MODE_TCP`<br>- `CONFIG_IZUMA_TRANSPORT_MODE_UDP`<br>- `CONFIG_IZUMA_TRANSPORT_MODE_UDP_QUEUE` | | UDP | Transport mode:<br> - TCP: Data is sent over TCP. Device actively listens for incoming messages. Recommended for applications that are always connected.<br>- UDP: Data is sent over UDP. Device actively listens for incoming messages. Recommended for low-bandwidth applications.<br>- UDP queue: Data is sent over UDP. Messages sent to the device queue in the cloud and are only delivered immediately after the device renews its registration. Recommended for low-power, low-bandwidth applications that powers down between transmissions. |
| `CONFIG_IZUMA_EVENT_LOOP_THREAD_STACK_SIZE` | Integer | 8192 | Event loop thread stack size:<br> Stack size for the event loop thread. |
| `CONFIG_IZUMA_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE` | Integer | 512 | Max CoAP blockwise payload size:<br> Maximum payload size used by CoAP. |
| `CONFIG_IZUMA_DEBUG_TRACE` | Bool | False | Build Izuma client with debug tracing:<br> Use debug tracing to observe Izuma Device Management Client's internal workings. |
| `CONFIG_IZUMA_UPDATE` | Bool | True | Enable firmware update:<br> Build Izuma Device Management Client with support for firmware update. |
| `CONFIG_IZUMA_UPDATE_PERMANENT` | Bool | False | Instruct MCUboot to perform a permanent update instead of a test:<br> Marking new firmware as permanent is useful during the development phase where applications evolve rapidly. |
| `CONFIG_IZUMA_UPDATE_BUFFER` | Integer | 1024 | Download buffer size for firmware updates:<br> Buffer used for downloading and processing firmware. Must be divisible by 2. |
| `CONFIG_IZUMA_UPDATE_PAGE_SIZE` | Integer | 8 | Page size for the candidate firmware image's storage medium:<br> Smallest write granularity supported by the underlying storage medium. This value is used at compile time to ensure buffers are correctly aligned. |
| `CONFIG_IZUMA_UPDATE_DEVELOPER` | Bool | False | Automatically populate developer update credentials in secure storage:<br> For development purposes, the manifest creation tool can generate credentials useful during development (not production). Select this option for automatic insertion into the credential manager and add `update_default_resources.c`, which the manifest tool generated, to the build. |


<h3 id="mbedtls">Mbed TLS options</h3>

| Configuration option | Type | Default setting | Description |
| --- | --- | --- | --- |
| `CONFIG_IZUMA_MBEDTLS_FILE` | Bool | False | Built-in Mbed TLS, configuration from file:<br> Use Mbed TLS library from Zephyr OS' manifest with a custom configuration file. |
| `CONFIG_MBEDTLS_CFG_FILE` | String | "config-tls-pelion.h" | Custom Mbed TLS configuration file name. |
| `CONFIG_IZUMA_MBEDTLS_LIB_NAME` | String | "mbedTLS" | Specify one or more Mbed TLS library files to be linked with Izuma. Separate multiple values with space " ". |

<h3 id="pal">Platform Abstraction Layer options</h3>

| Configuration option | Type | Default setting | Description |
| --- | --- | --- | --- |
| `CONFIG_IZUMA_PAL_SUPPORT_NAT64` | Bool | True | Automatic NAT64 address support on IPv6 networks:<br> Help IPv6 devices behind NAT64 use public DNS servers by converting IPv4 addresses to IPv6 using default NAT64 prefix. Only comes into effect when an IPv6-only device receives an IPv4 address from the DNS server. |
| `CONFIG_IZUMA_PAL_SUPPORT_SSL_CONNECTION_ID` | Bool | False | Use [Connection Identifiers for DTLS 1.2](https://tools.ietf.org/html/draft-ietf-tls-dtls-connection-id-11):<br> Connection ID is an extention to DTLS 1.2 that allows a DTLS session to persist even if the device's IP address and port changes, which otherwise would require a new DTLS handshake. This enables devices to power down their network interface for long periods of time, saving power and network bandwidth.<br>Requires Mbed TLS 2.18.0 or newer and transport mode to be UDP or UDP Queue. |
| `CONFIG_IZUMA_PAL_USE_APPLICATION_REBOOT` | Bool | False | Use reboot function provided by application:<br> Override default reboot function with one provided by application. Useful for shutting down the device gracefully, powering down external components and saving state.<br>Signature for C function:<br>```  void pal_plat_osApplicationReboot(void)``` |
| `CONFIG_IZUMA_PAL_USE_APPLICATION_NETWORK_CALLBACK` | Bool | False | Use network status callback provided by application:<br> Provide setter for registering network status callback function in application. Application uses callback function to notify Izuma Device Management Client about changes in network connectivity, enabling the client to refrain from sending data during network loss.<br>Signature for C function:<br>```#include "pal.h"```<br>```palStatus_t pal_plat_setConnectionStatusCallback(uint32_t interfaceIndex,```<br>```                                                 connectionStatusCallback callback,```<br>```                                                 void *client_arg)```												 
| Choose either:<br>- POSIX `CONFIG_IZUMA_PAL_USE_DNS_API_POSIX`<br>- DNS Resolve `CONFIG_IZUMA_PAL_USE_DNS_API_RESOLVE` | | DNS Resolve | DNS API:<br> - POSIX: Use POSIX's getaddrinfo. This call is synchronous and maps to Izuma's DNS version 0.<br>- DNS Resolve: Use DNS Resolve's dns_get_addr_info. This call is asynchronous and maps to Izuma's DNS version 3. |
| `CONFIG_NEWLIB_LIBC_FLOAT_PRINTF` | Bool | True | Build with newlib float printf:<br> Enable floating points in Newlib's snprintf. Must be enabled for correctly formatting floating points in LwM2M resources. |