Bootloader
==========

Renesas secure bootloader.

Building
========

Build without bootloader & update support
-----------------------------------------

python pal-platform/pal-platform.py deploy --target Renesas_EK_RA6M3 generate

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/SDK/Renesas_EK_RA6M3/ARMGCC.cmake -DEXTERNAL_DEFINE_FILE=./../define_Renesas_EK_RA6M3.txt

make mbedCloudClientExample.elf

Flashing with J-Link.

Open in one terminal:
JLinkExe -device R7FA6M3AH -AutoConnect 1 -if SWD -speed auto

>loadfile mbedCloudClientExample.bin

Start in second terminal to access device logs:
JLinkRTTViewer -d R7FA6M3AH -ct sess -a

Build with bootloader & update support
--------------------------------------

python pal-platform/pal-platform.py deploy --target Renesas_EK_RA6M3 generate

cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/SDK/Renesas_EK_RA6M3/ARMGCC.cmake -DEXTERNAL_DEFINE_FILE=./../define_Renesas_EK_RA6M3_update.txt -DUPDATE_LINKING=1

make mbedCloudClientExample.elf

Signing and flashing the images with SecureBoot
-----------------------------------------------

You will need to get the secure boot example from Renesas.

`imgtool.py` and `flash_layout.h` can be found as part of [Secure boot example.](https://www.renesas.com/us/en/software/D6004344.html).
`imgtool.py` has been tested with cryptography==2.8.0.

python ../r01an5347eu0110-ra-arm-secure-boot-solution-ra6m3/SecureBoot_Package/scripts/image_creation/imgtool.py sign --layout ../flash_layout/flash_layout.h  --align 8 --header-size 0x400 __Renesas_EK_RA6M3/Release/mbedCloudClientExample.bin mbedCloudClientExample_signed.bin -k ../r01an5347eu0110-ra-arm-secure-boot-solution-ra6m3/SecureBoot_Package/scripts/image_creation/Signing_Key/signingkey.pem

- `__Renesas_EK_RA6M3/Release/mbedCloudClientExample.bin` is the output image from make.
- `mbedCloudClientExample_signed.bin` is the output created by `imgtool.py`.

It is recommended to use Segger J-Flash Lite application to perform erase and flashing.

Flashing with J-FLash LITE.
---------------------------

1. Programming with J-Flash Lite GUI application:

    In the Segger J-Flash Lite:
    1. Target device should be auto-detected to be `R7FA6M3AH`.
    1. Use the `Erase Chip` button to erase the flash.
    1. Program the `Secureboot_EK_RA6M3.bin` first.
    1. Program the `mbedCloudClientExample_signed.bin` at address `0x10000`.

Flashing with J-Link.
--------------------

First download JLink for your operating system from [Segger](https://www.segger.com/downloads/jlink).

SecureBoot_pdmc.bin is precompiled as part of the application repository. (./pal-platform/SDK/Renesas_EK_RA6M3/tools/SecureBoot_pdmc.bin)

Open in one terminal:
JLinkExe -device R7FA6M3AH -AutoConnect 1 -if SWD -speed auto

>erase
>loadfile `Secureboot_EK_RA6M3.bin`
>loadfile `mbedCloudClientExample_signed.bin 0x10000`

Note the address for the application image.

Start in second terminal to access device logs:
JLinkRTTViewer -d R7FA6M3AH -ct sess -a

