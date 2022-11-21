#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
# Copyright 2019-2022 Pelion Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Generate the binary image to be used by FOTA."""

import struct
from time import time
import hashlib


def make_firmware_package(binary: bytes,
                          version: int):
    """Given version and binary, Generate the FOTA binary image."""
    # Big endian. Fields: Magic, header size, version, FW size, FW hash
    st_map = '>IIQI32s'
    magic = 0x464F5441  # "FOTA"
    header_size = struct.calcsize(st_map)
    fw_size = len(binary)
    hash = hashlib.sha256()
    hash.update(binary)
    meta = struct.pack(
        st_map,
        magic,
        header_size,
        version,
        fw_size,
        hash.digest())
    return meta + binary


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(
        description='Create firmware package from binary application image')
    parser.add_argument('-i', '--in-file',
                        help='Raw binary application file name',
                        required=True)
    parser.add_argument('-o', '--out-file',
                        help='Generated FOTA binary file name',
                        required=True)
    parser.add_argument(
        '-v', '--version',
        type=int,
        help='Firmware version (64 bit integer). Default: current epoch',
        default=int(time()))

    args = parser.parse_args()

    with open(args.in_file, 'rb') as in_file, \
            open(args.out_file, 'wb') as out_file:
        out_file.write(make_firmware_package(in_file.read(),
                                             version=args.version))
