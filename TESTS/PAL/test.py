#!/usr/bin/env python
"""
Copyright 2019 ARM Limited
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
"""

import os
import re
import json
import logging
import argparse


class SuiteBuilder(object):
    """
    SuiteBuilder class contains logic to search
    test binaries and create icetea test suites.
    """

    # pylint: disable=too-many-instance-attributes
    # All the path attributes have a logical meaning to them
    # and use case for error reporting.

    def __init__(self):
        self.build_data = None
        self.pal_test_binaries = None
        self.logger = logging.getLogger(__name__)
        self.root_path = os.path.dirname(__file__)
        self.suite_dir = os.path.join(self.root_path, "cache")
        self.suite_path = os.path.join(self.suite_dir, "dynamic.json")
        self.template_path = os.path.join(self.root_path, "templates")
        self.testcase_path = os.path.join(self.root_path, "testcases")
        self.suite_template_path = os.path.join(
            self.template_path, "suite.json")
        self.testcase_template_path = os.path.join(
            self.template_path, "testcase.json")

    def _find_mbed_build_data(self, path):
        """
        Finds latest mbed OS build_data.json.
        :param path: this is the path to search build_data.json
        """
        # Find every "build_data.json"
        build_data_files = []
        build_data_filename = "build_data.json"
        for root, _, files in os.walk(path):
            if build_data_filename in files:
                build_data_file = os.path.join(root, build_data_filename)
                self.logger.debug(
                    "found build_data.json from \"%s\"", build_data_file)
                build_data_files.append(build_data_file)

        # Choose latest "build_data.json"
        build_data_file = max(build_data_files, key=os.path.getmtime)

        # Read "build_data.json"
        with open(build_data_file) as fid:
            self.build_data = json.load(fid)

    def find_mbed_pal_tests(self, path):
        """
        Find mbed OS test case binaries.
        This is achieved by parsing build_data.json
        :param path: this is the path to search build_data.json
        """
        # Find build_data.json
        self._find_mbed_build_data(path)

        # PAL test ID checker
        re_pal_test = re.compile(
            r"MBED-CLOUD-CLIENT-MBED-CLIENT-PAL-TEST-TESTS-.*(?<!FULL_PAL)$")

        # Find PAL test binaries
        pal_test_binaries = []
        for build in self.build_data["builds"]:
            # Check if build ID matches PAL test
            match = re_pal_test.match(build["id"])
            if match:
                # Check if build ID is already in list
                # (builds are looped from newest to oldest)
                if build["id"] not in [x["id"] for x in pal_test_binaries]:
                    try:
                        # clean "path/./to/file" to "path/to/file"
                        value = build["bin_fullpath"]
                    except KeyError:
                        self.logger.error(
                            "Cannot find \"bin_fullpath\" from "
                            "build_data.json, did build complete succesfully?")
                        exit(-1)
                    else:
                        bin_fullpath = os.path.normpath(value)

                    self.logger.debug("found PAL test \"%s\"", bin_fullpath)
                    pal_test_binaries.append(
                        {
                            "type": "hardware",
                            "path": bin_fullpath,
                            "target": build["target_name"],
                            "id": build["id"]
                        })

        # Update pal test binaries list
        self.pal_test_binaries = pal_test_binaries

    def _cmake_walker(self, path, matcher, target_name, binary_type):
        """
        Find CMake based test binaries.
        This is achieved by looping through all
        files and matching against matcher.
        :param path: path to search test binaries recursively
        :param matcher: regular expression matcher to match files from path
        :param target_name: target name that this binary
                            belongs to {K64F, Linux, ...}
        :param binary_type: type of the binary, either "hardware" or "process"
        """
        # Find PAL test binaries
        pal_test_binaries = []
        for root, _, files in os.walk(path):
            for filename in files:
                # Check if filename matches a given PAL test pattern
                match = matcher.match(filename)
                if match:
                    self.logger.debug("found \"%s\"",
                                      os.path.join(root, filename))
                    pal_test_binaries.append(
                        {
                            "type": binary_type,
                            "path": os.path.join(root, filename),
                            "target": target_name,
                            "id": "NA"
                        })

        # Update pal test binaries list
        self.pal_test_binaries = pal_test_binaries

    def find_linux_pal_tests(self, path):
        """
        Find Linux test binaries from given path.
        :param path: path to search test binaries recursively
        """
        # ignore the combined palTests.elf test binary for test flexibility
        re_pal_test = re.compile(r".*(?<!^pal)Tests\.elf$")
        self._cmake_walker(path, re_pal_test,
                           "x86_x64_NativeLinux_mbedtls", "process")

    def find_freertos_pal_tests(self, path, target_name):
        """
        Find FreeRTOS test binaries from given path.
        :param path: path to search test binaries recursively
        """
        # ignore the combined palTests.bin test binary for test flexibility
        re_pal_test = re.compile(r".*(?<!^pal)Tests\.bin$")
        self._cmake_walker(path, re_pal_test, target_name, "hardware")

    def build_suite(self):
        """
        Build icetea test suite based on the instance attributes.
        """
        # Create cache dir if needed
        if not os.path.exists(self.suite_dir):
            os.makedirs(self.suite_dir)

        # Read suite template
        with open(self.suite_template_path) as fid:
            icetea_suite = json.load(fid)

        # Read testcase template
        with open(self.testcase_template_path) as fid:
            icetea_testcase_template = fid.read()

        # Fill suite
        for pal_test_binary in self.pal_test_binaries:
            # A bit CPU intensive to parse every round,
            # but there is no deepcopy() in json
            icetea_testcase = json.loads(icetea_testcase_template)

            # Fill properties
            (icetea_testcase["config"]["requirements"]["duts"]["*"]
             ["type"]) = pal_test_binary["type"]

            (icetea_testcase["config"]["requirements"]["duts"]["*"]
             ["platform_name"]) = pal_test_binary["target"]

            (icetea_testcase["config"]["requirements"]["duts"]["*"]
             ["application"]["bin"]) = pal_test_binary["path"]

            self.logger.info("  %s", pal_test_binary["path"])
            icetea_suite["testcases"].append(icetea_testcase)

        # Write suite
        with open(self.suite_path, "w") as fid:
            json.dump(icetea_suite, fid, indent=4)

        # Return number of found tests
        return len(self.pal_test_binaries)


def main():
    """
    The main code to be executed with this script.
    """
    logging.basicConfig(format="%(message)s", level=logging.INFO)
    logger = logging.getLogger(__name__)

    parser = argparse.ArgumentParser()
    parser.add_argument(
        "PLATFORM",
        choices=["mbed", "Linux", "FreeRTOS"],
        help="Platform to execute tests.")
    parser.add_argument(
        "BUILD_PATH",
        help="Path to search PAL test binaries")
    parser.add_argument(
        "-t", "--target",
        help="Target name for icetea allocator (check with `mbedls`).")
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Increase verbosity.")
    args = parser.parse_args()

    target = args.target
    platform = args.PLATFORM
    build_path = args.BUILD_PATH

    if args.verbose:
        logger.setLevel(logging.DEBUG)

    if platform == "FreeRTOS" and target is None:
        # mbed parses target from build_data.json.
        # Linux "process" type binaries don't need target as they are
        # launched on the host system.
        parser.error(
            "\"--target\" is required for \"{}\" platform.".format(platform))

    is_process = False
    builder = SuiteBuilder()
    if platform == "mbed":
        builder.find_mbed_pal_tests(build_path)
    elif platform == "Linux":
        is_process = True
        builder.find_linux_pal_tests(build_path)
    elif platform == "FreeRTOS":
        builder.find_freertos_pal_tests(build_path, target)

    logger.info("Creating dynamic icetea test suite:")
    if builder.build_suite() == 0:
        logger.error("No test binaries found from \"%s\"", build_path)
        exit(-1)

    logger.info("Run all tests with:")
    if is_process:
        logger.info("  icetea --suite \"%s\" --tcdir \"%s\"",
                    builder.suite_path, builder.testcase_path)
        logger.info("Run a single test by executing "
                    "one of the binary listed above.")
    else:
        # --reset makes sure no traces are lost from beginning
        logger.info("  icetea --reset --suite \"%s\" --tcdir \"%s\"",
                    builder.suite_path, builder.testcase_path)
        logger.info("Run a single test by flashing "
                    "one of the binary listed above.")
    exit(0)


if __name__ == "__main__":
    main()
