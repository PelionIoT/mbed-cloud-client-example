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

import time
import re
from icetea_lib.bench import Bench
from icetea_lib.ResultList import ResultList
from icetea_lib.Result import Result
from icetea_lib.TestStepError import TestStepError, TestStepTimeout


class Testcase(Bench):
    """
    Testcase class implementing PAL Unity test runner
    """
    def __init__(self):
        Bench.__init__(self,
                       name="pal_test_runner",
                       type="smoke",
                       requirements={
                           "duts": {
                               '*': {
                                   "count": 1,
                                   "application": {
                                       "init_cli_cmds": [],
                                       "post_cli_cmds": []
                                   }
                               }
                           }
                       })
        self.workaround_dut_lineno = 0

    def parse_unity_result(self, dut):
        """
        Parses given DUTs stdout for Unity test results.
        :param dut: icetea DUT to inspect the results from
        """
        re_unity_test = re.compile(
            r".*\<\*\*\*UnityTest\*\*\*\>TEST\((?P<suite>.*?), (?P<test>.*?)\)\<\/\*\*\*UnityTest\*\*\*\>")  # noqa: E501 # pylint: disable=line-too-long
        re_unity_result = re.compile(
            r".*\<\*\*\*UnityResult\*\*\*\>(?P<result>.*?)\<\/\*\*\*UnityResult\*\*\*\>")  # noqa: E501 # pylint: disable=line-too-long

        test_output = ""
        test_active = False
        test_results = ResultList()
        for line in dut.traces:
            line = line.strip()

            # Activate test output logging
            match = re_unity_test.match(line)
            if match:
                test_active = True
                test_output = ""
                unity_name = match.group("test")
                unity_suite = match.group("suite")
                self.logger.info("parsing %s.%s", unity_suite, unity_name)

            # Log test output
            if test_active:
                test_output += line + "\n"

            # Check if test is over
            match = re_unity_result.match(line)
            if match and test_active:
                unity_result = match.group("result")

                # Create icetea Result()
                test_result = Result(
                    {
                        "testcase": unity_suite + "." + unity_name,
                        "stdout": test_output,
                        "reason": line if unity_result == "FAIL" else ""
                    })
                test_result.build_result_metadata({"toolchain": unity_name})
                # Would need to do runtime analysis to get duration
                test_result.set_verdict(unity_result, 0, 0.0)
                test_results.append(test_result)
                test_active = False
                self.logger.info("finished %s", unity_name)
        return test_results

    def print_dut_log_to_trace(self):
        """
        Workaround to print all DUT log without all icetea logs.
        """
        for line in self.duts[0].traces[self.workaround_dut_lineno:]:
            self.logger.info(line)
            self.workaround_dut_lineno += 1

    def check_execution_errors(self):
        """
        Checks that there are no execution errors during PAL Unity tests.
        """
        errors = ["MbedOS Fault Handler", "CMSIS-RTOS error"]
        for error in errors:
            if self.verify_trace(0, error, False):
                raise TestStepError(error)

    def wait_trace(self, stop_match_str, duration):
        """
        Waits a given trace for the given time.
        :param stop_match_str: Stop waiting when this string is found
        :param duration: How long to wait for a string
        """
        # Wait tests to pass (set timeout on suite level)
        wait_start = time.time()
        while (time.time()-wait_start) < duration:
            if self.verify_trace(0, stop_match_str, False):
                return
            self.logger.info("waiting \"%s\"...%.2fs",
                             stop_match_str, duration-(time.time()-wait_start))
            self.print_dut_log_to_trace()
            self.check_execution_errors()
            time.sleep(1)
        raise TestStepTimeout(
            "Didn't get \"{}\" in {}".format(stop_match_str, duration))

    def fail_testcase(self, error):
        """
        Workaround that overwrites icetea's result value in order to
        store the test binary name in the results. Without this workaround
        the testcase name would be "pal_test_runner" which is not descriptive.
        :param error: Python error that failed the test case
        """
        # Overwrite test case result
        self._results = ResultList()
        self._results.append(
            Result({
                "testcase": (self.config["requirements"]
                             ["duts"]["*"]["application"]["bin"]),
                "reason": repr(error),
                "verdict": "FAIL"
            })
        )

    def setup(self):  # pylint: disable=method-hidden
        """
        Pre-test activities.
        """
        pass

    def case(self):
        """
        Testcase activities.
        """
        try:
            # Wait test case
            self.wait_trace("PAL_TEST_START", 60)
            self.wait_trace("PAL_TEST_END", 600)
        except (TestStepError, TestStepTimeout) as error:
            self.fail_testcase(error)
            return

        # Overwrite previously written test case result.
        #   If no errors during execution, parse errors.
        #   Don't put this into teardown() as it is executed even if
        #   the case raises TestStepFail. This then causes test to "pass"
        #   even with stack overflow on DUT
        self._results = self.parse_unity_result(self.duts[0])

    def teardown(self):  # pylint: disable=method-hidden
        """
        Post-test activities
        """
        pass
