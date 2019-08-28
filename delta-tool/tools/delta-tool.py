#!/usr/bin/env python3
"""
Copyright (c) 2019 ARM Limited

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

LIBRARIES BUILD
"""
'''Creates a delta patch

This tool creates a delta patch by calling an underlying binary diff
implementation, then calculating any related information that is needed by the
manifest tool for manifest creation.
'''

__version__ = '0.0.1'

import sys
import os
from delta_vendor_info import DeltaInfo
from pyasn1.codec.der import encoder as der_encoder
from pyasn1.codec.native.decoder import decode as native_decode
import binascii
import argparse
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat import backends
import json
import subprocess
import traceback
import warnings
import filecmp

def digest_file(fd, digest_algo):
    fd.seek(0)
    md = hashes.Hash(digest_algo(), backends.default_backend())
    flen = 0
    while True:
        d = fd.read(8192)
        if not d:
            break
        flen += len(d)
        md.update(d)
    fd.seek(0)
    return md.finalize(), flen

# return percentage value, how much value2 is from value1
def diff_percentage(value1, value2):
    return 100 * float(value2)/float(value1)

def parse_arguments():
    parser = argparse.ArgumentParser(description = 'Create a delta patch')
    
    
    parser.add_argument("original", metavar='ORIGINAL_IMAGE-binary',
        help='Image currently running on the devices to be updated',
        type=argparse.FileType('rb'))
    
    parser.add_argument("new", metavar='NEW_IMAGE',
        help='New image to which update the devices to',
        type=argparse.FileType('rb'))
    
    parser.add_argument('--version', action='version', version=__version__,
        help='Display the version'
    )
    
    group = parser.add_mutually_exclusive_group()
    
    group.add_argument('-f', '--files',
        help='Do not run bsdiff, use files as input arguments will not invoke bsdiff directly. Delta file passed must already be created', action='store_true')
    
    group.add_argument('-b', '--bsdiff-binary', metavar='BSDIFF_BINARY-executable',
        default=os.path.join(os.path.dirname(os.path.realpath(__file__)),'..','bsdiff','bsdiff'),
        help='Path the the arm-bsdiff binary to invoke')

    parser.add_argument('-s', '--block-size', metavar='SIZE',
        default=512, help='Block size to pass to arm-bsdiff. This must match to memory available to update code (BS_PATCH_COMPILE_TIME_MEMORY_ALLOC or dynamic memory from malloc if compile time is not defined).\n'
                          'Higher number will generate slightly smaller patch files but require more memory from patching side')
    
    parser.add_argument('-d','--delta', metavar='delta-binary-file',
        help='Specify the delta output file', default='delta.patch')
    parser.add_argument('-i','--input-config', metavar='manifest-tools-input-json',
        help='Specify the config input file. stdin by default',
        type=argparse.FileType('r'), default=sys.stdin, required=False)
    parser.add_argument('-o','--output-config', metavar='manifest-tool-output-json',
        help='Specify the config output file. stdout by default',
        type=argparse.FileType('w'), default=sys.stdout, required=False)

    group.add_argument('-c', '--size-check',
        help='Do not run delta image size check. Otherwise will report error if delta is larger than normal update', action='store_true')
    
    return parser.parse_args()



def main():
    args = parse_arguments()
    config = {}
    if not args.input_config.isatty():
        content = args.input_config.read()
        if content and len(content) >= 2: #The minimum size of a JSON file is 2: '{}'
            config.update(json.loads(content))

    original_digest, original_size = digest_file(args.original, hashes.SHA256)
    new_digest, new_size = digest_file(args.new, hashes.SHA256)

    # Call Delta Function Here if -b is used
    if args.files is not True:
        #todo for piped version printing anything from progress is impossible
        #print ("Generating diff with with "+args.bsdiff_binary)
        proc = subprocess.Popen([args.bsdiff_binary,
                                 args.original.name,
                                 args.new.name,
                                 args.delta,
                                 str(args.block_size)],
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE)
        stdout_data, stderr_data = proc.communicate()
        
        rc = proc.wait()
        if rc:
           print('ERROR: {} returned error code {} with output:'.format(proc.args[0], rc))
           print(stdout_data)
           print(stderr_data)
           sys.exit(rc)
        # Delta function done!

    new_digest, new_size = digest_file(args.new, hashes.SHA256)
    delta_digest, delta_size = (None, None)
    with open(args.delta, 'rb') as fd:
        delta_digest, delta_size = digest_file(fd, hashes.SHA256)

    fd.close()
    diff = diff_percentage(new_size,delta_size)
    diff_str = '%.2f'%(diff)
    # Check delta size -c is not used
    if args.size_check is not True:
        if diff >= 100:
            print('Difference with delta image and update image is more than 100 percentage! Delta-file not generated. To override this behavior use -c argument')
            os.remove(args.delta)
            raise Exception('Difference with delta image and update image is more than 100 percentage! Percentage is: '+diff_str)
    
    # if delta is more than 60% from update image, show warning
    if diff >= 60:
        warnings.warn('Difference with delta image and update image is more than 60 percentage. Percentage is: ' + diff_str)

    if (filecmp.cmp(args.new.name, args.original.name) == True):
        warnings.warn('New and old file are binary same. This will generate delta that will not change the original image. This is probably an error')

    deltainfo_py = {
        'deltaVariant': 'arm-stream-diff-lz4',
        'precursorDigest': original_digest,
        'deltaDigest': delta_digest,
        'deltaSize': delta_size,
    }
    
    schema = DeltaInfo()
    deltainfo_asn1 = native_decode(deltainfo_py, schema)
    deltainfo_der = der_encoder.encode(deltainfo_asn1)

     # if delta is more than 60% from update image, show warning	
    if diff >= 60:	
        warnings.warn('Delta image size is larger than 60% of full new image size')
        
    ucc = {
        'payloadHash': binascii.b2a_hex(delta_digest).decode('utf-8'),
        'payloadSize': delta_size,
        'payloadFile' : args.delta,
        'payloadFormat': 'bsdiff-stream',
        'installedFile' : args.new.name,
        'deltaFile' : args.delta,
        'precursorFile' : args.original.name,
    }
    config.update(ucc)
    args.output_config.write(json.dumps(config))


if __name__ == '__main__':
    try:
        main()
    except Exception:
        traceback.print_exc()
        sys.exit(1)
