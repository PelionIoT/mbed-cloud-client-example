Flash layout
============

Zephyr OS abstracts internal and external storage devices into isolated,
labeled partitions, each starting at address 0x0 regardless of the real,
underlying storage medium.

The bootloader, MCUboot, is stored in the partition labeled "mcuboot". The
active application is executed from "image_0", and the candidate firmware is
stored in "image_1". Pelion Device Management Client uses the partition "pelion_storage"
for storing configuration and credentials. If "pelion_storage" doesn't exist,
Pelion Device Management Client uses "storage" partition unless Zephyr is already using it
(CONFIG_SETTINGS=n).

Depending on the architecture, you can put "image_0" and "image_1" on external
storage, but for better security, keep "pelion_storage" in internal
storage.

For more information about Zephyr's storage partitions, see:
https://docs.zephyrproject.org/2.5.0/reference/storage/flash_map/flash_map.html#relationship-with-devicetree

As a reference, we've provided a partition overlay for the target "frdm_k64f"
in the "boards" folder, using the following flash layout:


+--------------------------+
|                          |
|         TDBStore         | <-+ "pelion_storage"
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


Bootloader
==========

Zephyr OS uses MCUboot (https://mcuboot.com/) as a bootloader.

The example application installs new firmware as "permanent" by default.
To use MCUboot's rollback feature, use Kconfig to set PELION_UPDATE_PERMANENT=n
and Zephyr's DFU commands for interacting with MCUboot:
https://docs.zephyrproject.org/2.5.0/guides/device_mgmt/dfu.html

Note that the build instructions below use the default developer keys
in MCUboot. For production devices, use secure credentials, instead:
https://docs.zephyrproject.org/2.5.0/guides/west/sign.html


Building
========

Ensure a working build environment has been set up:
https://docs.zephyrproject.org/2.5.0/getting_started/index.html

The example below creates a new Zephyr workspace with the structure:

  pelion-workspace
    ├── bootloader
    ├── modules
    │   └── lib
    │       └── pelion-dm (middleware library)
    ├── pelion-dm-example (example application)
    │   ├── pal-platform
    │   │   └── SDK
    │   │       └── ZephyrOS (Zephyr project)
    │   │           └── boards (example overlays)
    │   └── west.yml (West manifest)
    ├── tools
    └── zephyr

and uses the "frdm_k64f" target as an example.


Create workspace
----------------

> west init -m https://github.com/PelionIoT/mbed-cloud-client-example pelion-workspace
> cd pelion-workspace
> west update

Add developer credentials for connecting (https://developer.pelion.com/docs/device-management/current/provisioning-process/provisioning-development-devices.html)
and updating devices (https://github.com/PelionIoT/manifest-tool#manifest-dev-tool-init).
Note that V1 manifests are used for Zephyr builds.


Build and flash without bootloader and update support
---------------------------------------------------

> west build -b frdm_k64f -d build/pdmc -s pelion-dm-example -- -DCONFIG_PELION_UPDATE=n
> west flash -d build/pdmc


Build and flash with bootloader and update support
---------------------------------------------------

> west build -b frdm_k64f -d build/mcuboot -s bootloader/mcuboot/boot/zephyr -- \
  -DDTC_OVERLAY_FILE=`pwd`/pelion-dm-example/pal-platform/SDK/ZephyrOS/boards/frdm_k64f.overlay \
  -DCONFIG_MCUBOOT_CLEANUP_ARM_CORE=y
> west flash -d build/mcuboot
> west build -b frdm_k64f -d build/pdmc -s pelion-dm-example -- \
  -DCONFIG_MCUBOOT_SIGNATURE_KEY_FILE=\"bootloader/mcuboot/root-rsa-2048.pem\"
> west flash -d build/pdmc


Advanced options
================

To add Pelion Device Management to an existing project, add the following repository and module
to the West manifest:

  remotes:
    - name: PelionIoT
      url-base: https://github.com/PelionIoT
  projects:
    - name: pelion-dm
      repo-path: mbed-cloud-client
      remote: PelionIoT
      revision: master
      path: modules/lib/pelion-dm

You can find further configuration options using the Kconfig menu under "modules -> pelion-dm".

