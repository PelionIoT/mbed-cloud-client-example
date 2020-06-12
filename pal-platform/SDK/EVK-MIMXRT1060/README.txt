Flash layout
============

FlexSPI NOR flash
--------------
1. Bootloader - 64 KiB from the beginning of flash
2. Active App Metadata Header - 1 KiB from the end of Bootloader
3. Active App - From end of header to the middle of the flash
4. Firmware Candidate Storage - Middle of flash to the beginning of TDBStore
5. TDBStore - Last 64 KiB of flash

+--------------------------+
|                          |
|         TDBStore         |
|                          |
+--------------------------+ <-+ storage_tdb_internal.base_address
|                          |
|                          |
|                          |
|Firmware Candidate Storage|
|                          |
|                          |
|                          |
+--------------------------+ <-+ update-client.storage-address
|                          |
|                          |
|                          |
|        Active App        |
|                          |
|                          |
|                          |
+--------------------------+ <-+ mbed-bootloader.application-start-address
|Active App Metadata Header|
+--------------------------+ <-+ update-client.application-details
|                          |
|        Bootloader        |
|                          |
+--------------------------+ <-+ 0


Bootloader
==========

Built from https://github.com/ARMmbed/mbed-bootloader/tree/v4.1.3


Building
========

CMAKE_BUILD_TYPE:
 * Release - optimized for size, no debug symbols.
 * Debug - Low optimization, debug symbols included.

Build without bootloader & update support
-----------------------------------------

> python pal-platform/pal-platform.py deploy --target MIMXRT1060_NXP generate
> cd __MIMXRT1060_NXP
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/SDK/EVK-MIMXRT1060/SDK_EVK-MIMXRT1060/tools/cmake_toolchain_files/armgcc.cmake -DEXTERNAL_DEFINE_FILE=./../define_NXP_MIMXRT1060.txt
> make mbedCloudClientExample.elf -j4


Build with bootloader & update support
-----------------------------------------

> python pal-platform/pal-platform.py deploy --target MIMXRT1060_NXP generate
> cd __MIMXRT1060_NXP
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/SDK/EVK-MIMXRT1060/SDK_EVK-MIMXRT1060/tools/cmake_toolchain_files/armgcc.cmake -DEXTERNAL_DEFINE_FILE=./../define_NXP_MIMXRT1060_update.txt -DUPDATE_LINKING=1
> make mbedCloudClientExample.elf -j4
> python ../pal-platform/SDK/EVK-MIMXRT1060/tools/combine_bootloader_with_app.py -b ../pal-platform/SDK/EVK-MIMXRT1060/tools/mbed_bootloader.bin -a Release/mbedCloudClientExample.bin -o Release/combined.bin -c 0x10400 -d 0x10000
