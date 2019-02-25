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

# pylint: disable=missing-docstring,useless-super-delegation
# pylint: disable=line-too-long,method-hidden,relative-import

from icetea_lib.bench import TestStepFail
from mbed_cloud.exceptions import CloudApiException
from pelion_helper import PelionBase


class Testcase(PelionBase):
    def __init__(self):
        PelionBase.__init__(self,
                            name="put",
                            title="Example application can perform basic CoAP operation (PUT)",
                            status="released",
                            type="acceptance",
                            component=["mbed_cloud_client_example"])

    def setup(self):
        super(Testcase, self).setup()

    def case(self):
        resource_path = '/3201/0/5853'
        # Write new value to custom resource (/3201/0/5853)
        self.logger.info("Testing PUT %s with value %s", self.pattern_value1, resource_path)
        try:
            self.connect_api.set_resource_value(device_id=self.device_id,
                                                resource_path=resource_path,
                                                resource_value=self.pattern_value1,
                                                timeout=self.restTimeout)
        except CloudApiException as error:
            raise TestStepFail("PUT request failed with %d and msg %s" % (error.status, error.message))

        # Read and verify that new value is in effect
        try:
            pattern_value_new = self.connect_api.get_resource_value(device_id=self.device_id,
                                                                    resource_path=resource_path,
                                                                    timeout=self.restTimeout)
        except CloudApiException as error:
            raise TestStepFail("GET request failed with %d and msg %s" % (error.status, error.message))

        self.logger.info("Read back value %s for %s", pattern_value_new, resource_path)

        if self.pattern_value1 != pattern_value_new:
            raise TestStepFail("Pattern %s written does not match the pattern %s read back" % (self.pattern_value1,
                                                                                               pattern_value_new))

    def teardown(self):
        self.connect_api.stop_notifications()
