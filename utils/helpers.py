#!/usr/bin/env python3
# Copyright (c) 2022 Pelion Limited and Contributors. All rights reserved.
#
#
# SPDX-License-Identifier: Apache-2.0
#

"""
Helpers module.

This module contains helper functions and classes
"""

import pathlib
import subprocess


def _str_to_resolved_path(path_str):
    """
    Convert a string to a resolved Path object.

    Args:
    * path_str (str): string to convert to a Path object.

    """
    return pathlib.Path(path_str).resolve(strict=False)


class ExecuteHelper:
    """Class to provide a wrapper for executing commands as a subprocess."""

    verbose = False

    def __init__(self, verbose=False):
        """Initialise the class."""
        ExecuteHelper.verbose = verbose

    @staticmethod
    def _print(data):
        if ExecuteHelper.verbose:
            print(data)

    @staticmethod
    def _print_command(data):
        if ExecuteHelper.verbose:
            for item in data:
                print("{} ".format(item), end="")
            print("")

    @staticmethod
    def execute_command(command, timeout=None):
        """Execute the provided command list.

        Executes the command and returns the error code, stdout and stderr.
        """
        ExecuteHelper._print_command(command)
        p = subprocess.Popen(
            command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            bufsize=-1,
            universal_newlines=True,
        )
        try:
            output, error = p.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            ExecuteHelper._print("Timed out after {}s".format(timeout))
            p.kill()
            output, error = p.communicate()

        if p.returncode:
            ExecuteHelper._print("error:")
            ExecuteHelper._print(error)
            ExecuteHelper._print("returnCode:")
            ExecuteHelper._print(p.returncode)

        return (p.returncode == 0), output, error
