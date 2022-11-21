#!/usr/bin/env python3

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

"""Script to convert DER files into C arrays."""

import argparse
import binascii
import pathlib
import sys

from helpers import ExecuteHelper
from helpers import _str_to_resolved_path

bs_public_cert_file = "bs_cert.der"
bs_private_key_file = "bs_key.der"
csr_file = "csr.pem"
keys_file = "keys.pem"
lwm2m_private_key_file = None
lwm2m_public_cert_file = None
output_file = "mbed_cloud_dev_credentials.c"
private_key_file = "cprik.der"
public_cert_file = "cert.der"
root_cert_pem_file = "CA/root_cert.pem"
root_cert_der_file = "CA/root_cert.der"
root_keys_file = "CA/root_keys.pem"
server_cert_file = "server_ca_cert.der"


def _parse_args():
    # Parse command line
    parser = argparse.ArgumentParser(
        description="Convert DER into c file", add_help=False
    )

    required = parser.add_argument_group("required arguments")
    optional = parser.add_argument_group("optional arguments")

    required.add_argument(
        "--endpoint",
        help="endpoint name",
        required=True,
    )
    required.add_argument(
        "--uri",
        help="The URI of the bootstrap or device managment service",
        required=True,
    )
    optional.add_argument(
        "--server-cert",
        help="Server CA cert",
        default=server_cert_file,
    )
    optional.add_argument(
        "--use-bs",
        action="store_true",
        help="Use BS certificate",
        default=False,
        required=False,
    )
    optional.add_argument(
        "--use-ca",
        action="store_true",
        help="Use CA certificate",
        default=False,
        required=False,
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


def _generate_ca_keys(endpoint, use_bs, verbose):

    helper = ExecuteHelper(verbose)

    status = True
    if (
        pathlib.Path(root_cert_pem_file).exists()
        and pathlib.Path(root_keys_file).exists()
    ):
        print("Using the existing CA instead of generating a new one.")
    else:
        status = status and helper.execute_command(
            [
                "openssl",
                "ecparam",
                "-out",
                root_keys_file,
                "-name",
                "prime256v1",
                "-genkey",
            ]
        )[0]
        status = status and helper.execute_command(
            [
                "openssl",
                "req",
                "-x509",
                "-new",
                "-key",
                root_keys_file,
                "-sha256",
                "-days",
                "36500",
                "-subj",
                "/CN=ROOT_CA",
                "-outform",
                "PEM",
                "-out",
                root_cert_pem_file,
            ]
        )[0]
        status = status and helper.execute_command(
            [
                "openssl",
                "x509",
                "-inform",
                "PEM",
                "-in",
                root_cert_pem_file,
                "-outform",
                "DER",
                "-out",
                root_cert_der_file,
            ]
        )[0]

    # Now the CA is in place, create the certificates using it.
    status = status and helper.execute_command(
        [
            "openssl",
            "ecparam",
            "-out",
            keys_file,
            "-name",
            "prime256v1",
            "-genkey",
        ]
    )[0]
    status = status and helper.execute_command(
        [
            "openssl",
            "pkcs8",
            "-topk8",
            "-inform",
            "PEM",
            "-outform",
            "DER",
            "-in",
            keys_file,
            "-out",
            private_key_file,
            "-nocrypt",
        ]
    )[0]
    status = status and helper.execute_command(
        [
            "openssl",
            "req",
            "-new",
            "-key",
            keys_file,
            "-out",
            csr_file,
            "-subj",
            "/CN={}".format(endpoint),
        ]
    )[0]
    status = status and helper.execute_command(
        [
            "openssl",
            "x509",
            "-req",
            "-in",
            csr_file,
            "-CA",
            root_cert_pem_file,
            "-CAkey",
            root_keys_file,
            "-CAcreateserial",
            "-days",
            "36500",
            "-outform",
            "DER",
            "-out",
            public_cert_file,
        ]
    )[0]

    # If the use_bs flag is set the generate a set of LwM2M server files.
    if use_bs:
        status = status and helper.execute_command(
            [
                "openssl",
                "ecparam",
                "-out",
                keys_file,
                "-name",
                "prime256v1",
                "-genkey",
            ]
        )[0]
        status = status and helper.execute_command(
            [
                "openssl",
                "pkcs8",
                "-topk8",
                "-inform",
                "PEM",
                "-outform",
                "DER",
                "-in",
                keys_file,
                "-out",
                lwm2m_private_key_file,
                "-nocrypt",
            ]
        )[0]
        status = status and helper.execute_command(
            [
                "openssl",
                "req",
                "-new",
                "-key",
                keys_file,
                "-out",
                csr_file,
                "-subj",
                "/CN={}".format(endpoint),
            ]
        )[0]
        status = status and helper.execute_command(
            [
                "openssl",
                "x509",
                "-req",
                "-in",
                csr_file,
                "-CA",
                root_cert_pem_file,
                "-CAkey",
                root_keys_file,
                "-CAcreateserial",
                "-days",
                "36500",
                "-outform",
                "DER",
                "-out",
                lwm2m_public_cert_file,
            ]
        )[0]
    return status


def _generate_non_ca_keys(endpoint, verbose):

    helper = ExecuteHelper(verbose)

    status = True

    status = status and helper.execute_command(
        [
            "openssl",
            "ecparam",
            "-out",
            keys_file,
            "-name",
            "prime256v1",
            "-genkey",
        ]
    )[0]

    status = status and helper.execute_command(
        [
            "openssl",
            "pkcs8",
            "-topk8",
            "-inform",
            "PEM",
            "-outform",
            "DER",
            "-in",
            keys_file,
            "-out",
            private_key_file,
            "-nocrypt",
        ]
    )[0]
    status = status and helper.execute_command(
        [
            "openssl",
            "req",
            "-x509",
            "-new",
            "-key",
            keys_file,
            "-sha256",
            "-days",
            "36500",
            "-subj",
            "/CN={}".format(endpoint),
            "-outform",
            "DER",
            "-out",
            public_cert_file,
        ]
    )[0]

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


def _create_output_folders(endpoint):
    status = True

    # Create the CA folder if we don't already have one.
    try:
        pathlib.Path("CA").mkdir(parents=True, exist_ok=True)
    except FileExistsError:
        print("ERROR - could not create folder {}.".format("CA"))
        status = False

    try:
        pathlib.Path(endpoint).mkdir(parents=True, exist_ok=True)
    except FileExistsError:
        print("ERROR - could not create folder {}.".format(endpoint))
        status = False
    return status


def _resolve_file_paths(args):

    # Convert all the file names to full paths.
    global csr_file
    global keys_file
    global output_file
    global private_key_file
    global public_cert_file
    global root_cert_der_file
    global root_cert_pem_file
    global root_keys_file
    global server_cert_file

    global lwm2m_private_key_file
    global lwm2m_public_cert_file

    csr_file = _str_to_resolved_path(
        pathlib.Path(args.endpoint).joinpath(csr_file)
    )
    keys_file = _str_to_resolved_path(
        pathlib.Path(args.endpoint).joinpath(keys_file)
    )
    output_file = _str_to_resolved_path(
        pathlib.Path(args.endpoint).joinpath(output_file)
    )

    root_cert_der_file = _str_to_resolved_path(root_cert_der_file)
    root_cert_pem_file = _str_to_resolved_path(root_cert_pem_file)
    root_keys_file = _str_to_resolved_path(root_keys_file)
    server_cert_file = _str_to_resolved_path(args.server_cert)

    # If the use_bs flag is set then we need to generate 2 sets of files.
    # Set the filenames to reflect this.

    if args.use_ca and args.use_bs:
        lwm2m_private_key_file = _str_to_resolved_path(
            pathlib.Path(args.endpoint).joinpath(private_key_file)
        )
        lwm2m_public_cert_file = _str_to_resolved_path(
            pathlib.Path(args.endpoint).joinpath(public_cert_file)
        )
        private_key_file = _str_to_resolved_path(
            pathlib.Path(args.endpoint).joinpath(bs_private_key_file)
        )
        public_cert_file = _str_to_resolved_path(
            pathlib.Path(args.endpoint).joinpath(bs_public_cert_file)
        )
    else:
        private_key_file = _str_to_resolved_path(
            pathlib.Path(args.endpoint).joinpath(private_key_file)
        )
        public_cert_file = _str_to_resolved_path(
            pathlib.Path(args.endpoint).joinpath(public_cert_file)
        )


def main():
    """Perform the main execution."""
    args = _parse_args()

    if not _create_output_folders(args.endpoint):
        sys.exit(2)

    _resolve_file_paths(args)

    if _sanity_check_files(args) is False:
        sys.exit(2)

    if args.use_ca:
        status = _generate_ca_keys(args.endpoint, args.use_bs, args.verbose)
    else:
        status = _generate_non_ca_keys(args.endpoint, args.verbose)

    if status is False:
        print(
            "ERROR - certificate generation failed. "
            "Run with --verbose for more details."
        )
        sys.exit(1)

    # All the variables etc in the output file have the same prefix. Doing it
    # like this keeps the line lengths below 80 characters.
    prefix = "MBED_CLOUD_DEV_"

    if args.verbose:
        print("\nGenerating {}.".format(output_file))

    with open(output_file, "w") as output_data:
        output_data.write("#ifndef __{}CREDENTIALS_H__\n".format(prefix))
        output_data.write("#define __{}CREDENTIALS_H__\n\n".format(prefix))
        output_data.write("#include <inttypes.h>\n\n")

        output_data.write(
            'const char {}BOOTSTRAP_ENDPOINT_NAME[] = "{}";\n'.format(
                prefix, args.endpoint
            )
        )
        output_data.write(
            'const char {}BOOTSTRAP_SERVER_URI[] = "{}";\n\n'.format(
                prefix, args.uri
            )
        )
        _process_data(
            private_key_file,
            output_data,
            "{}BOOTSTRAP_DEVICE_PRIVATE_KEY".format(prefix),
            args.verbose,
        )
        _process_data(
            public_cert_file,
            output_data,
            "{}BOOTSTRAP_DEVICE_CERTIFICATE".format(prefix),
            args.verbose,
        )
        _process_data(
            server_cert_file,
            output_data,
            "{}BOOTSTRAP_SERVER_ROOT_CA_CERTIFICATE".format(prefix),
            args.verbose,
        )

        output_data.write(
            'const char {}MANUFACTURER[] = "dev_manufacturer";\n'.format(
                prefix
            )
        )
        output_data.write(
            'const char {}MODEL_NUMBER[] = "dev_model_num";\n'.format(prefix)
        )
        output_data.write(
            'const char {}SERIAL_NUMBER[] = "0";\n'.format(prefix)
        )
        output_data.write(
            'const char {}DEVICE_TYPE[] = "dev_device_type";\n'.format(prefix)
        )
        output_data.write(
            'const char {}HARDWARE_VERSION[] = "'
            'dev_hardware_version";\n'.format(prefix)
        )
        output_data.write(
            "const uint32_t {}MEMORY_TOTAL_KB = 0;\n\n".format(prefix)
        )

        output_data.write("#endif //__{}CREDENTIALS_H__\n".format(prefix))


if __name__ == "__main__":
    sys.exit(main())
