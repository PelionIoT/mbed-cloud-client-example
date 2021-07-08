#!/usr/bin/env python

## ----------------------------------------------------------------------------
## Copyright 2016-2017 ARM Ltd.
##
## SPDX-License-Identifier: Apache-2.0
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
## ----------------------------------------------------------------------------

from os import path
import json
import hashlib, zlib, struct
import time
import sys
from intelhex import IntelHex


'''
typedef struct {
    uint32_t magic;                                     /*< Magic value */
    uint32_t fw_size;                                   /*< FW size in bytes */
    uint64_t version;                                   /*< FW version - timestamp */
#if defined(MBED_CLOUD_CLIENT_FOTA_SIGNED_IMAGE_SUPPORT)
    uint8_t signature[FOTA_IMAGE_RAW_SIGNATURE_SIZE];   /*< RAW ECDSA signature */
#endif  // defined(MBED_CLOUD_CLIENT_FOTA_SIGNED_IMAGE_SUPPORT)
    uint8_t digest[FOTA_CRYPTO_HASH_SIZE];              /*< FW image SHA256 digest */
    uint8_t reserved[FOTA_INTERNAL_HEADER_RESERVED_FIELD_SIZE];    /*< Reserved */
    // From this point on, all fields are relevant to candidate only and
    // can be skipped by bootloader if it wishes not to save them internally
    // !The size of the internal header can't be changed, different size of the header 
    // will break older version of the bootloader.
    // reserved field should be used for additional internal header data.
    uint8_t internal_header_barrier;
    uint8_t flags;                                  /*< Flags */
    uint16_t external_header_size;                  /*< Size of external header size */
    uint16_t block_size;                            /*< Block size. Encryption block size if encrypted,
                                                        validated block size if unencrypted and block validation turned on */
    uint8_t precursor[FOTA_CRYPTO_HASH_SIZE];       /*< contains previously installed FW SHA256 digest */
    uint8_t vendor_data[FOTA_MANIFEST_VENDOR_DATA_SIZE];
    /*< Vendor custom data as received in Pelion FOTA manifest. */
    uint32_t footer;
    // !New fields of the external header must me added in the end of the current structure,
    // otherwise additional field will break older version of the bootloader.
} fota_header_info_t;
'''



# define defaults to go into the metadata header
SIZEOF_SHA256 = int(256/8)
FIRMWARE_HEADER_MAGIC =0x5c0253a3
FIRMWARE_HEADER_VERSION = 3
header_format = "<2IQ{}s".format(SIZEOF_SHA256)

if sys.version_info < (3,):
    def b(x):
        return bytearray(x)
else:
    def b(x):
        return x

def create_header(app_blob, firmwareVersion):
    # calculate the hash of the application
    firmwareHash = hashlib.sha256(app_blob).digest()

    # calculate the total size which is defined as the application size + metadata header
    firmwareSize = len(app_blob)


    # signature not supported, set size to 0
    signatureSize = 0

    print ('imageSize:    {}'.format(firmwareSize))
    print ('imageHash:    {}'.format(''.join(['{:0>2x}'.format(c) for c in b(firmwareHash)])))
    print ('imageversion: {}'.format(firmwareVersion))

    # Pack the data into a binary blob
    FirmwareHeader = struct.pack(header_format,
                                 FIRMWARE_HEADER_MAGIC,
                                 firmwareSize,
                                 firmwareVersion,
                                 firmwareHash)

    return FirmwareHeader


def combine(bootloader_fn, app_fn, app_addr, hdr_addr, bootloader_addr, output_fn, version, no_bootloader):
    # read bootloader
    bootloader_format = bootloader_fn.split('.')[-1]
    bootloader = IntelHex()
    if not no_bootloader:
        if bootloader_format == 'hex':
            bootloader.fromfile(bootloader_fn, bootloader_format)
        elif bootloader_format == 'bin':
            bootloader.loadbin(bootloader_fn, bootloader_addr)
        else:
            print('Bootloader format can only be .bin or .hex')
            exit(-1)

    # read application
    app_format=app_fn.split('.')[-1]
    app = IntelHex()
    if app_format == 'hex':
        app.fromfile(app_fn, app_format)
    elif app_format == 'bin':
        app.loadbin(app_fn, app_addr)
    else:
        print('Application format can only be .bin or .hex')
        exit(-1)

    # create firmware header
    header = IntelHex()
    fw_header = create_header(app.tobinstr(), version)
    header.puts(hdr_addr, fw_header)

    # combine
    output_format = output_fn.split('.')[-1]
    output = IntelHex()
    if not no_bootloader:
        print("Writing bootloader to address 0x%08x-0x%08x." % (bootloader.addresses()[0], bootloader.addresses()[-1]))
        output.merge(bootloader, overlap='error')
    print("Writing header to address 0x%08x-0x%08x." % (header.addresses()[0], header.addresses()[-1]))
    output.merge(header, overlap='error')
    print("Writing application to address 0x%08x-0x%08x." % (app.addresses()[0], app.addresses()[-1]))
    output.merge(app, overlap='error')

    # write output file
    output.tofile(output_fn, format=output_format)


if __name__ == '__main__':
    from glob import glob
    import argparse

    parser = argparse.ArgumentParser(
        description='Combine bootloader with application adding metadata header.')

    def addr_arg(s):
        if not isinstance(s, int):
            s = eval(s)

        return s

    bin_map = {
        'k64f': {
            'mem_start': '0x0'
        },
        'k66f': {
            'mem_start': '0x0'
        },
        'ublox_evk_odin_w2': {
            'mem_start': '0x08000000'
        },
        'nucleo_f429zi': {
            'mem_start': '0x08000000'
        },
        'nucleo_f411re': {
            'mem_start': '0x08000000'
        },
        'ublox_c030_u201': {
            'mem_start': '0x08000000'
        }
    }

    curdir = path.dirname(path.abspath(__file__))

    def parse_mbed_app_addr(mcu, key):
        mem_start = bin_map[mcu]["mem_start"]
        with open(path.join(curdir, "..", "mbed_app.json")) as fd:
            mbed_json = json.load(fd)
            addr = mbed_json["target_overrides"][mcu.upper()][key]
            return addr_arg(addr)

    # specify arguments
    parser.add_argument('-m', '--mcu', type=lambda s : s.lower().replace("-","_"), required=False,
                        help='mcu', choices=bin_map.keys())
    parser.add_argument('-b', '--bootloader',    type=argparse.FileType('rb'),     required=False,
                        help='path to the bootloader binary')
    parser.add_argument('-a', '--app',           type=argparse.FileType('rb'),     required=True,
                        help='path to application binary')
    parser.add_argument('-c', '--app-addr',      type=addr_arg,                    required=False,
                        help='address of the application')
    parser.add_argument('-d', '--header-addr',   type=addr_arg,                    required=False,
                        help='address of the firmware metadata header')
    parser.add_argument('-o', '--output',        type=argparse.FileType('wb'),     required=True,
                        help='output combined file path')
    parser.add_argument('-s', '--set-version',   type=int,                         required=False,
                        help='set version number', default=0)
    parser.add_argument('-nb', '--no-bootloader',action='store_true',              required=False,
                        help='Produce output without bootloader. The output only '+
                             'contains header + app. requires hex output format')

    # workaround for http://bugs.python.org/issue9694
    parser._optionals.title = "arguments"

    # get and validate arguments
    args = parser.parse_args()

    # validate the output format
    f = args.output.name.split('.')[-1]
    if f == 'hex':
        output_format = 'hex'
    elif f == 'bin':
        output_format = 'bin'
    else:
        print('Output format can only be .bin or .hex')
        exit(-1)

    # validate no-bootloader option
    if args.no_bootloader and output_format == 'bin':
        print('--no-bootloader option requires the output format to be .hex')
        exit(-1)

    # validate that we can find a bootloader or no_bootloader is specified
    bootloader = None
    if not args.no_bootloader:
        if args.mcu and not args.bootloader:
            bl_list = glob("tools/mbed-bootloader-{}-*".format(args.mcu))
            if len(bl_list) == 0:
                print("Specified MCU does not have a binary in this location " + \
                      "Please specify bootloader location with -b")
                exit(-1)
            elif len(bl_list) > 1:
                print("Specified MCU have more than one binary in this location " + \
                      "Please specify bootloader location with -b")
                print(bl_list)
                exit(-1)
            else:
                fname = bl_list[0]
                bootloader = open(fname, 'rb')
        elif args.bootloader:
            bootloader = args.bootloader
        elif not (args.mcu or args.bootloader):
            print("Please specify bootloader location -b or MCU -m")
            exit(-1)

    # get the path of bootloader, application and output
    if bootloader:
        bootloader_fn = path.abspath(bootloader.name)
        bootloader.close()
    else:
        bootloader_fn = ''

    app_fn = path.abspath(args.app.name)
    args.app.close()
    output_fn = path.abspath(args.output.name)
    args.output.close()

    # Use specified addresses or default if none are provided
    app_format = app_fn.split('.')[-1]
    if(not (args.mcu or args.app_addr or app_format == 'hex')):
        print("Please specify app address or MCU")
        exit(-1)
    if app_format != 'hex':
        app_addr = args.app_addr or parse_mbed_app_addr(args.mcu, "target.mbed_app_start")
    else:
        app_addr = None

    if args.mcu:
        mem_start = addr_arg(bin_map[args.mcu]["mem_start"])
    else:
        mem_start = 0

    if(not (args.mcu or args.header_addr)):
        print("Please specify header address or MCU")
        exit(-1)
    header_addr = args.header_addr or parse_mbed_app_addr(args.mcu, "update-client.application-details")

    # combine application and bootloader adding metadata info
    combine(bootloader_fn, app_fn, app_addr, header_addr, mem_start,
            output_fn, args.set_version, args.no_bootloader)

    # print the output file path
    print('Combined binary: ' + output_fn)

