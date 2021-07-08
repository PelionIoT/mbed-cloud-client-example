Flash layout
============

Internal flash
--------------
1. Bootloader - 32kb from the beginning of flash
2. Active App Metadata Header - 1kb from the end of Bootloader
3. Active App - From end of header to the end second last sector of flash
4. TDBStore internal part - Two last sectors of flash (64kb)

+--------------------------+
|                          |
|  TDBStore internal part  |
|                          |
+--------------------------+ <-+ storage_tdb_external.internal_base_address
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


External flash
--------------

Uses on-board QSPI flash (128MB).
One Firmware Candidate storage slot configured.

1. TDBStore external part - 128kb from the beginning of flash
2. Firmware Candidate Storage - 512kb from the end of TDBStore external part

+--------------------------+
|                          |
|                          |
|                          |
|Firmware Candidate Storage|
|                          |
|                          |
|                          |
+--------------------------+ <-+ update-client.storage-address
|                          |
|  TDBStore external part  |
|                          |
+--------------------------+ <-+ 0



Bootloader
==========
Bootloader https://github.com/PelionIoT/mbed-bootloader/commit/95fa885b28b4ba5611f533f84ccb941a151200b0
Need to use ExternalBlockDevice

Config for LPC546XX:
    "macros": [
        "FOTA_HEADER_HAS_CANDIDATE_READY=1",        
        "MBED_CLOUD_CLIENT_FOTA_ENABLE=1",
        "MBED_CLOUD_CLIENT_FOTA_KEY_ENCRYPTION=FOTA_USE_ENCRYPTED_ONE_TIME_FW_KEY",
        "MBED_CONF_MBED_CLOUD_CLIENT_EXTERNAL_SST_SUPPORT=1",
        "MBED_CLOUD_CLIENT_FOTA_ENCRYPTION_SUPPORT=1",
        "MBED_CONF_MBED_BOOTLOADER_TRACE=USE_PRINTF",
        "MBEDTLS_AES_C",
        "MBEDTLS_CCM_C",
        "MBEDTLS_CIPHER_C"
    ],
    "target_overrides": {
            "target.default_lib"                         : "small",
            "platform.use-mpu"                           : false,
            "platform.default-serial-baud-rate"          : 115200,
            "target.restrict_size"                       : "0x8000",
            "target.components_add"                      : ["QSPIF"],
            "storage.storage_type"                       : "TDB_EXTERNAL",
            "storage_tdb_external.internal_base_address" : "(0x78000)",
            "storage_tdb_external.rbp_internal_size"     : "(0x8000)",
            "storage_tdb_external.blockdevice"           : "other",
            "storage_tdb_external.external_base_address" : "(0)",
            "storage_tdb_external.external_size"         : "(128*1024)",
            "mbed-bootloader.application-header-address" : "(0x8000)",
            "mbed-bootloader.application-start-address"  : "(0x8400)",
            "fota.trace-enable"                          : false,
            "mbed-bootloader.debug"                      : false,
            "mbed-bootloader.external-traces-enabled"    : false,
            "mbed-bootloader.storage-type"               : "KVSTORE",
            "fota.storage-size"                          : "0x70000",
            "fota.storage-start-address"                 : "(MBED_CONF_STORAGE_TDB_EXTERNAL_EXTERNAL_BASE_ADDRESS + MBED_CONF_STORAGE_TDB_EXTERNAL_EXTERNAL_SIZE)",
            "fota.block-device-type"                     : "FOTA_EXTERNAL_BD",
        }

QSPI differences between LPCXpresso 546XX boards.
=================================================

The earlier revisions of LPCXpresso 546XX have different QSPI chip than the some of the later revisions. Application needs to specify at compile-time which is supported.

# Selection for the QSPI chip support. (Only select one of the three options.)
# LPCXpresso546XX eval board revision C or D support FLASH_MT25Q.
# LPCXpresso546XX eval board revision E or later support FLASH_W25Q.

```
add_definitions(-DFLASH_W25Q)
#add_definitions(-DFLASH_MT25Q)
#add_definitions(-DFLASH_MX25R)
```
There is three provided bootloader files, one for each driver:

1. mbed-bootloader_rev_e_w25q.bin
1. mbed-bootloader_rev_c_d_mt25q.bin
1. mbed-bootloader_mx25r.bin

Building
========

CMAKE_BUILD_TYPE:
 * Release - optimized for size, no debug symbols.
 * Debug - Some optimization, debug symbols included.

Build without bootloader & update support
-----------------------------------------

> python pal-platform/pal-platform.py deploy --target LPC54628_NXP generate
> cd __LPC54628_NXP
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/SDK/LPCXpresso54628/SDK_LPCXpresso54628/tools/cmake_toolchain_files/armgcc.cmake -DEXTERNAL_DEFINE_FILE=./../define_NXP_LPC54628.txt
> make mbedCloudClientExample.elf -j4


Build with bootloader & update support with the default W25Q chip.
------------------------------------------------------------------

> python pal-platform/pal-platform.py deploy --target LPC54628_NXP generate
> cd __LPC54628_NXP
> cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/SDK/LPCXpresso54628/SDK_LPCXpresso54628/tools/cmake_toolchain_files/armgcc.cmake -DEXTERNAL_DEFINE_FILE=./../define_NXP_LPC54628_update.txt -DUPDATE_LINKING=1
> make mbedCloudClientExample.elf -j4
> python ../pal-platform/SDK/LPCXpresso54628/tools/combine_bootloader_with_app.py -b ../pal-platform/SDK/LPCXpresso54628/tools/mbed-bootloader_rev_e_w25q.bin -a Release/mbedCloudClientExample.bin -o Release/combined.bin -c 0x8400 -d 0x8000 --set-version 0

