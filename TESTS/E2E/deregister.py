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
                            name="deregister",
                            title="Example application can deregister",
                            status="released",
                            type="acceptance",
                            component=["mbed_cloud_client_example"])

    def setup(self):
        super(Testcase, self).setup()

    def case(self):
        resource_path = '/5000/0/1'
        # Deregister device via POST (/5000/0/1)
        self.logger.info("Deregister via POST %s", resource_path)
        try:
            self.connect_api.execute_resource(device_id=self.device_id,
                                              resource_path=resource_path,
                                              timeout=self.restTimeout)
        except CloudApiException as error:
            raise TestStepFail("POST request failed with %d and msg %s" % (error.status, error.message))

        # Verify client is deregistered
        self.logger.info("POST done")

        self.verify_registration("deregistered")

    def teardown(self):
        self.connect_api.stop_notifications()
