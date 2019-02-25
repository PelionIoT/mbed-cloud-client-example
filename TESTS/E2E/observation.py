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

import time
from icetea_lib.bench import TestStepFail
from mbed_cloud.exceptions import CloudApiException
from pelion_helper import PelionBase


class Testcase(PelionBase):
    def __init__(self):
        PelionBase.__init__(self,
                            name="observation",
                            title="Example application can respond to resource subscription)",
                            status="released",
                            type="acceptance",
                            component=["mbed_cloud_client_example"])

    def setup(self):
        super(Testcase, self).setup()

    def case(self):
        # Test resource subscription
        resource_path = '/3201/0/5853'

        # 1) Add initial resource value
        self.logger.info("PUT %s with value %s", resource_path, self.pattern_value1)
        try:
            self.connect_api.set_resource_value(device_id=self.device_id,
                                                resource_path=resource_path,
                                                resource_value=self.pattern_value1,
                                                timeout=self.restTimeout)
        except CloudApiException as error:
            raise TestStepFail("PUT request failed with %d and msg %s" % (error.status, error.message))

        # Create resource subscription for custom resource (/3201/0/5853)
        self.logger.info("Testing subscription for %s", resource_path)
        self.connect_api.start_notifications()

        try:
            self.connect_api.add_resource_subscription_async(device_id=self.device_id,
                                                             resource_path=resource_path,
                                                             callback_fn=self._callback_fn)
        except CloudApiException as error:
            raise TestStepFail("Subscription request failed with %d and msg %s" % (error.status, error.message))

        # 2) Update resource value
        self.logger.info("PUT %s with value %s", resource_path, self.pattern_value2)
        try:
            self.connect_api.set_resource_value(device_id=self.device_id,
                                                resource_path=resource_path,
                                                resource_value=self.pattern_value2,
                                                timeout=self.restTimeout)
        except CloudApiException as error:
            raise TestStepFail("PUT request failed with %d and msg %s" % (error.status, error.message))

        self.logger.info("Waiting for resource notification")
        notif_timeout = time.time() + self.restTimeout

        # 3) Wait until we receive some value or timeout
        while time.time() < notif_timeout:
            if self.notified_value != "":
                break

        if self.notified_value != self.pattern_value2:
            self.logger.error("Web-application received %s as notification, expected %s", self.notified_value, self.pattern_value2)
            raise TestStepFail("Incorrect/No notification received")
        else:
            self.logger.info("Web-application received %s as notification.", self.notified_value)

        # 4) Remove subscription
        try:
            self.connect_api.delete_resource_subscription(device_id=self.device_id)
        except CloudApiException as error:
            raise TestStepFail("Subscription removal failed with %d and msg %s" % (error.status, error.message))

        # 5) Update resource value to trigger reset from server
        # This is needed to clean the client subscription status for any follow-up tests
        self.logger.info("PUT %s with value %s", resource_path, self.pattern_value3)
        try:
            self.connect_api.set_resource_value(device_id=self.device_id,
                                                resource_path=resource_path,
                                                resource_value=self.pattern_value3,
                                                timeout=self.restTimeout)
        except CloudApiException as error:
            raise TestStepFail("PUT request failed with %d and msg %s" % (error.status, error.message))

    def teardown(self):
        # Remove subscription
        try:
            self.connect_api.delete_resource_subscription(device_id=self.device_id)
        except CloudApiException as error:
            raise TestStepFail("Subscription removal failed with %d and msg %s" % (error.status, error.message))

        self.connect_api.stop_notifications()
