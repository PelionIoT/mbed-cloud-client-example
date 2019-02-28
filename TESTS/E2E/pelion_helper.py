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

# pylint: disable=missing-docstring,too-many-instance-attributes
# pylint: disable=line-too-long,method-hidden

import json
import uuid
import requests
from mbed_cloud import AccountManagementAPI
from mbed_cloud import ConnectAPI
from mbed_cloud import DeviceDirectoryAPI
from mbed_cloud import UpdateAPI
from icetea_lib.bench import Bench
from icetea_lib.bench import TestStepFail


class PelionBase(Bench):
    """
    Base class containing common implementation shared by tests
    """

    def __init__(self, **kwargs):
        Bench.__init__(self, **kwargs)
        self.test_config = None
        self.device_id = None
        self.manifest_id = None
        self.account_api = None
        self.connect_api = None
        self.device_api = None
        self.update_api = None
        self.rest_headers = None
        self.rest_address = None
        self.pattern_value1 = "1000:1000:1000:1000"
        self.pattern_value2 = "2000:1000:2000:1000"
        self.pattern_value3 = "3000:1000:3000:1000"
        self.notified_value = ""

    def setup(self):
        self.device_id = self.config.get("device_id")
        api_key = self.config.get("api_key")
        host = self.config.get("host")
        self.test_config = {"api_key": api_key, "host": host}
        self.account_api = AccountManagementAPI(self.test_config)
        self.connect_api = ConnectAPI(self.test_config)
        self.device_api = DeviceDirectoryAPI(self.test_config)

        # Additional parameters for handling REST requests without SDK
        self.rest_headers = {'Authorization': 'Bearer ' + self.config.get("api_key")}
        self.rest_address = self.config.get("host")

        # Init delay due to internal usage of PULL notification in SDK. Notications might be lost between
        # tests.
        # TODO: Remove workaround after limitation in SDK has been fixed.
        self.delay(5)

    def setup_update(self):
        self.manifest_id = self.config.get("manifest_id")
        self.update_api = UpdateAPI(self.test_config)

    def _callback_fn(self, device_id, path, value):
        string = value.decode("utf-8")
        self.logger.info("Notification for %s received: %r value: %r", device_id, path, string)
        self.notified_value = string

    def verify_registration(self, expected_state):
        self.logger.info("Verify device to in state %s", expected_state)
        device = self.device_api.get_device(self.device_id)
        if device is None:
            raise TestStepFail("device_id %s does not exist/is not listed in Device Directory" % self.device_id)
        else:
            if device.state == expected_state:
                self.logger.info("Endpoint %s is in state: %s", self.device_id, expected_state)
            else:
                raise TestStepFail("Endpoint %s is in state %s, expected %s" % (self.device_id, device.state, expected_state))

    def prepare_campaign_filter(self, state):
        # Create filter for the update campaign. Here we only update one endpoint.
        payload = {}
        payload["device_filter"] = "id=" + self.device_id
        payload["root_manifest_id"] = self.manifest_id
        payload["name"] = str(uuid.uuid4())
        payload["state"] = state
        return payload

    ## Function for starting a update campaign
    ## TODO: Replace with Python SDK implementation
    def post_campaign(self, payload):
        req_address = self.rest_address + '/v3/update-campaigns'
        res = requests.post(req_address, json=payload, headers=self.rest_headers)
        if res.status_code == 409:
            # Endpoint can be targeted by only one active campaign. The older campaign must be deleted/stopped.
            raise TestStepFail("Campaign already exists for device %s" % self.device_id)
        elif res.status_code != 201:
            raise TestStepFail("Campaign creation failed with %d" % res.status_code)

        data = res.json()
        campaign_id = data["id"]
        return campaign_id

    ## Function for checking firmware update status from Cloud
    ## TODO: Replace with Python SDK implementation
    def check_campaign_state(self, campaign_id):
        results = []
        base_url = self.rest_address + "/v3/update-campaigns"
        campaign_url = base_url + "/" + campaign_id
        metadata_url = campaign_url + "/" + "campaign-device-metadata"

        # Short wait until campaign has been initialized properly.
        self.delay(5)

        # Fetch basic campaign information
        response = requests.get(campaign_url, headers=self.rest_headers)
        if response.status_code != 200:
            raise TestStepFail("Get Campaign state returned %d" % response.status_code)

        resp = json.loads(response.content)
        results.append(resp["state"])

        # Fetch campaign metadata information
        response = requests.get(metadata_url, headers=self.rest_headers)
        if response.status_code != 200:
            raise TestStepFail("Get Campaign metadata returned %d" % response.status_code)

        resp = json.loads(response.content)
        if 'data' in resp and resp['data'][0]['deployment_state']:
            meta_data = resp['data']
            device_state = meta_data[0]['deployment_state']
            results.append(device_state)
        else:
            raise TestStepFail("No metadata for %d" % self.device_id)

        return results

    restTimeout = 60
