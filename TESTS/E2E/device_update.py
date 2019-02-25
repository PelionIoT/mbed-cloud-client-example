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

# pylint: disable=missing-docstring
# pylint: disable=line-too-long,method-hidden,relative-import

import time
from icetea_lib.bench import TestStepFail
from pelion_helper import PelionBase


class Testcase(PelionBase):
    def __init__(self):
        PelionBase.__init__(self,
                            name="device_update",
                            title="Example application can perform firmware update",
                            status="released",
                            type="acceptance",
                            component=["mbed_cloud_client_example"])
    def setup(self):
        super(Testcase, self).setup()
        super(Testcase, self).setup_update()

    def case(self):
        # Create filter for update campaign generation, with automatic startup
        update_filter = self.prepare_campaign_filter(state="scheduled")
        # Start the update campaign
        campaign_id = self.post_campaign(update_filter)
        # Wait for the campaign to reach autostopped state
        timeout = 600
        _endtime = time.time() + timeout

        while True:
            self.delay(10)
            result = self.check_campaign_state(campaign_id)
            # Update campaign has stopped
            if result[0] == "autostopped":
                break

            if time.time() > _endtime:
                self.logger.error("Campaign in state: %s, Device in state: %s", result[0], result[1])
                raise TestStepFail("Campaign did not finish in %d" % timeout)

        # Check for final state of the device
        final_result = self.check_campaign_state(campaign_id)
        if final_result[1] != "deployed":
            raise TestStepFail("Device in %s state, expected deployed" % result[1])

        self.logger.info("Firmware update finished successfully")

    def teardown(self):
        self.connect_api.stop_notifications()
