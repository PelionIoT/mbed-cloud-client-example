#!/usr/bin/env python3
#
# ----------------------------------------------------------------------------
# Copyright 2022 Pelion
#
# SPDX-License-Identifier: Apache-2.0
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
# ----------------------------------------------------------------------------

"""Convert a single file into C-compatible .h-file with an hex array."""

import argparse
import binascii
import pathlib
import sys

from helpers import _str_to_resolved_path

server_cert_file = "server_cert.der"
output_file = "migrate_server_cert.c"
array_name = "MIGRATE_BOOTSTRAP_CA_CERT"


def _parse_args():
    # Parse command line
    parser = argparse.ArgumentParser(
        description="Convert a file into C-compatible .c file "
        "with contents as array. For example\n"
        "./cnvfile2hex.py "
        "--server-cert server_ca_cert.der "
        "--output migrate_server_cert.c",
        add_help=False,
    )

    required = parser.add_argument_group("required arguments")
    optional = parser.add_argument_group("optional arguments")

    optional.add_argument(
        "--server-cert",
        help="Server CA cert, defaults to {}".format(server_cert_file),
        default=server_cert_file,
    )
    optional.add_argument(
        "--output",
        help="Output file (.c), defaults to {}".format(output_file),
        default=output_file,
    )
    optional.add_argument(
        "--name",
        help="Name for the array in the output file, defaults to {}.".format(
            array_name
        ),
        default=array_name,
    )
    optional.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Print verbose output",
        default=False,
        required=False,
    )
    optional.add_argument(
        "-h", "--help", action="help", help="Show this help message and exit."
    )
    args = parser.parse_args()

    return args


def _sanity_check_files(args):
    # Sanity check files exist or not.
    status = True

    # Expect the server_certificate to exist.
    if not pathlib.Path(server_cert_file).is_file():
        print("ERROR - file {} not found.".format(args.server_cert))
        status = False

    return status


def _process_data(infile, outfile, name, verbose):
    # Process data
    if verbose:
        print("\tAdding {} into the credentials file.".format(infile))
    outfile.write("const uint8_t {}[] =\n".format(name))
    outfile.write("{\n")
    with open(infile, "rb") as dataFile:
        while True:
            hexdata = dataFile.read(16).hex()
            if len(hexdata) == 0:
                break
            hexlist = map("".join, zip(hexdata[::2], hexdata[1::2]))
            outfile.write("    ")
            for item in hexlist:
                outfile.write("0x{}, ".format(item))
            outfile.write("\n")
    outfile.write("};\n")

    outfile.write("const uint32_t {0}_SIZE = sizeof({0});\n\n".format(name))


def _resolve_file_paths(args):

    # Convert all the file names to full paths.
    global server_cert_file
    global output_file

    output_file = _str_to_resolved_path(args.output)
    server_cert_file = _str_to_resolved_path(args.server_cert)


def main():
    """Perform the main execution."""
    args = _parse_args()

    if args.name:
        array_name = args.name

    _resolve_file_paths(args)

    if _sanity_check_files(args) is False:
        sys.exit(2)

    if args.verbose:
        print("\n\tGenerating {}.".format(output_file))
        print("\tInput file {}".format(server_cert_file))
        print("\tArray name {}".format(array_name))

    with open(output_file, "w") as output_data:
        output_data.write("#ifndef __{}_H\n".format(array_name))
        output_data.write("#define __{}_H\n".format(array_name))
        output_data.write("\n")
        output_data.write("#include <inttypes.h>\n\n")
        output_data.write("\n")

        _process_data(
            server_cert_file,
            output_data,
            array_name,
            args.verbose,
        )
        output_data.write("\n")
        output_data.write("#endif //__{}_H\n".format(array_name))
    print("File converted to {}".format(output_file))


if __name__ == "__main__":

    sys.exit(main())
