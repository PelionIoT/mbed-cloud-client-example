#!/usr/bin/env python3
# ----------------------------------------------------------------------------
# Copyright 2020 ARM Limited or its affiliates
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
# ------------------------------------------------------------------------
import argparse
import getpass
import logging
import subprocess
import sys
from pathlib import Path

import yaml

from get_mbed_cloud_credentials import get_mbed_cloud_credentials

LOG = logging.getLogger('dev-init')


def manifest_tool_init(
        manifest_dev_dir: Path,
        api_key: str,
        api_url: str,
        is_verbose: bool
):
    cmd = ['manifest-dev-tool', 'init']
    if is_verbose:
        cmd = ['manifest-dev-tool', '--debug', 'init']
    if api_key:
        cmd.extend(['--api-key', api_key, '--api-url', api_url])

    cmd.append('--force')

    try:
        subprocess.check_call(cmd, cwd=manifest_dev_dir.as_posix())
    except subprocess.CalledProcessError:
        LOG.error('Failed to initialize manifest-dev environment.\n'
                  'Command: %s\n'
                  'Directory: %s',
                  ' '.join(cmd),
                  manifest_dev_dir.as_posix())
        raise
    except FileNotFoundError:
        LOG.error('"manifest-dev-tool" executable not found! '
                  'Make sure correct version of manifest-tool Python package '
                  'is installed and is available on PATH')
        raise


def main():
    pelion_keychain = Path.home() / '.pelion-dev-presets.yaml'
    pelion_presets = None
    if pelion_keychain.is_file():
        with pelion_keychain.open('rb') as fh:
            pelion_presets = yaml.safe_load(fh)
    parser = argparse.ArgumentParser(
        description='Mbed Cloud Dev Credentials helper tool. The tool creates '
                    'a developer certificate with a given name, if one does '
                    'not exists, and fetches mbed_cloud_dev_credentials.c '
                    'source file'
    )
    default_parser = argparse.ArgumentParser(add_help=False)

    default_name = '{} [Pelion Dev]'.format(getpass.getuser())
    default_parser.add_argument(
        '-n', '--name',
        help='Developer certificate name. Default: "{}"'.format(default_name),
        default=default_name
    )
    example_root = Path(__file__).resolve().parent.parent
    default_out_file = example_root / 'mbed_cloud_dev_credentials.c'
    default_parser.add_argument(
        '-o', '--output',
        default=default_out_file,
        type=Path,
        help='Output developer credentials C source file.'
             '[Default: {}]'.format(default_out_file.as_posix())
    )
    default_parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Run in verbose mode - useful for troubleshooting.'
    )

    subparsers = parser.add_subparsers(dest='action')

    if pelion_presets:
        preset_parser = subparsers.add_parser(
            'with-preset', parents=[default_parser],
            description='Get developer certificate source file using '
                        'credentials from {}'.format(
                pelion_keychain.as_posix())
        )
        preset_parser.add_argument(
            'GW_PRESET',
            help='API GW preset name specify GW URL and API key',
            choices=pelion_presets.keys()
        )

    cred_parser = subparsers.add_parser(
        'with-credentials', parents=[default_parser],
        description='Get developer certificate source file using '
                    'credentials API key and URL'
    )
    cred_parser.add_argument(
        '-a', '--api-key',
        help='API key for for accessing Pelion Device Management service.',
        required=True
    )
    cred_parser.add_argument(
        '-u', '--api-url',
        help='Pelion Device Management API gateway URL. ',
        required=True
    )

    args = parser.parse_args()
    if not args.action:
        parser.error('Action is required')
    logging.basicConfig(
        stream=sys.stdout,
        format='%(asctime)s %(levelname)s %(message)s',
        level=logging.DEBUG if args.verbose else logging.INFO
    )
    if args.action == 'with-preset':
        try:
            api_key = pelion_presets[args.GW_PRESET]['api_key']
            api_url = pelion_presets[args.GW_PRESET]['host']
        except KeyError as ex:
            raise AssertionError(
                'Invalid preset {} key not found'.format(ex)
            ) from ex
    else:
        api_key = args.api_key
        api_url = args.api_url

    get_mbed_cloud_credentials(api_key, api_url, args.name, args.output)
    manifest_tool_init(example_root, api_key, api_url, args.verbose)


if __name__ == '__main__':
    main()
