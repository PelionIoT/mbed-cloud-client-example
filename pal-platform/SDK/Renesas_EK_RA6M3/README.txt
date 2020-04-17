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

`imgtool.py` and `flash_layout.h` can be found as part of [Secure boot example.](https://www.renesas.com/us/en/software/D6004181.html).
`imgtool.py` has been tested with cryptography==2.8.0.

python imgtool.py sign --layout ../../../../platform/ext/target/ek-ra6m3/flash_layout.h --align 8 --header-size 0x400 /work/signing/mbedCloudClientExample_unsigned.bin /work/signing/mbedCloudClientExample_signed.bin -k /work/mbed-cloud-client-example/pal-platform/SDK/Renesas_EK_RA6M3/tools/root-rsa-2048.pem

`/work/signing/mbedCloudClientExample_unsigned.bin` is the output image from make.
`/work/signing/mbedCloudClientExample_signed.bin` is the output created by `imgtool.py`.
`/work/mbed-cloud-client-example/pal-platform/SDK/Renesas_EK_RA6M3/tools/root-rsa-2048.pem` is delivered as part of the mbed-cloud-client-example repository.



Flashing with J-Link.
--------------------

First download JLink for your operating system from [Segger](https://www.segger.com/downloads/jlink).

SecureBoot_pdmc.bin is precompiled as part of the application repository. (./pal-platform/SDK/Renesas_EK_RA6M3/tools/SecureBoot_pdmc.bin)

Open in one terminal:
JLinkExe -device R7FA6M3AH -AutoConnect 1 -if SWD -speed auto

>loadfile SecureBoot_pdmc.bin
>loadfile mbedCloudClientExample_signed.bin 0x10000

Note the address for the application image.

Start in second terminal to access device logs:
JLinkRTTViewer -d R7FA6M3AH -ct sess -a

