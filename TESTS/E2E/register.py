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
# pylint: disable=method-hidden,relative-import

from pelion_helper import PelionBase


class Testcase(PelionBase):
    def __init__(self):
        PelionBase.__init__(self,
                            name="register",
                            title="Example application can perform basic CoAP operation (GET)",
                            status="released",
                            type="acceptance",
                            component=["mbed_cloud_client_example"])
    def setup(self):
        super(Testcase, self).setup()

    def case(self):
        self.verify_registration("registered")

    def teardown(self):
        self.connect_api.stop_notifications()
