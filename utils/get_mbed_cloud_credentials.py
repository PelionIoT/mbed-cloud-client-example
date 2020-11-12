#!/usr/bin/env python3
# ----------------------------------------------------------------------------
# Copyright 2020 ARM Limited or its affiliates
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ----------------------------------------------------------------------------
import logging
import urllib.parse
import urllib.parse
from pathlib import Path

import requests

LOG = logging.getLogger('pelion-dev-cert')


class BaseApi:
    def __init__(self, host: str, api_key: str):
        """
        Create REST API provider for Update Service APIs
        :param host: Pelion service URL
        :param api_key: account API key
        """
        self._host = host
        self._default_headers = {
            'Authorization': 'Bearer {}'.format(api_key)
        }

    def _url(self, api, **kwargs) -> str:
        """
        Helper function for constructing full REST API URL
        Concatenates Pelion host URL with the desired REST API url and expands
        any patterns present in the URL (e.g. v3/manifests/{id})
        :param api: REST API URL part
        :param kwargs: dictionary for expanding the the URL patterns
        :return: full URL
        """
        if kwargs:
            return urllib.parse.urljoin(self._host, api.format(**kwargs))
        return urllib.parse.urljoin(self._host, api)

    def _headers(self, extra_headers: dict = None) -> dict:
        """
        Helper function for creating request header
        Initializes a dictionary with common headers and allows extending it
        with REST API specific data
        :param extra_headers: dictionary containing extra headers to be sent as
        part of REST API request
        :return: headers dictionary
        """
        copy = self._default_headers.copy()
        if extra_headers:
            copy.update(extra_headers)
        return copy


class DeveloperCertificate(BaseApi):
    TRUSTED_CERTIFICATES = '/v3/trusted-certificates'
    NEW_DEVELOPER_CERTIFICATE = 'v3/developer-certificates'
    EXISTING_DEVELOPER_CERTIFICATE = 'v3/developer-certificates/{id}'

    def __init__(self, host: str, api_key: str, name: str):
        """
        Construct DeveloperCertificate instance.

        Certificate with a given name will be created if does not exists in
        the account.
        :param host: Pelion service URL
        :param api_key: account API key
        :param name: certificate name
        """
        super().__init__(host, api_key)
        LOG.debug('Checking certificate for name: %s', name)
        query_string = '?name__eq={}'.format(urllib.parse.quote(name))
        response = requests.get(
            self._url(self.TRUSTED_CERTIFICATES + query_string),
            headers=self._headers()
        )
        response.raise_for_status()
        data = response.json()
        if data['total_count'] == 0:
            LOG.info('No certificate found for name: %s', name)
            LOG.debug('Creating dev-certificate for name: %s', name)
            response = requests.post(
                self._url(self.NEW_DEVELOPER_CERTIFICATE),
                headers=self._headers(),
                json={
                    'description': 'dev-certificate',
                    'name': name
                }
            )
            response.raise_for_status()
            self.certificate_id = response.json()['id']
            LOG.info(
                'Created dev-certificate for name: %s id: %s',
                name, self.certificate_id)
        else:
            self.certificate_id = data['data'][0]['id']
            LOG.debug(
                'Found certificate for name: %s id: %s',
                name, self.certificate_id)

    def get_c_source_code(self) -> str:
        """
        Get developer certificate C code content
        :return: developer certificate C code content
        """

        LOG.debug('Getting dev-certificate id: %s', self.certificate_id)
        response = requests.get(
            self._url(
                self.EXISTING_DEVELOPER_CERTIFICATE,
                id=self.certificate_id
            ),
            headers=self._headers()
        )
        response.raise_for_status()
        return response.json()['security_file_content']


def get_mbed_cloud_credentials(
        api_key: str,
        api_url: str,
        certificate_name: str,
        output: Path
):
    cert = DeveloperCertificate(api_url, api_key, certificate_name)
    c_code = cert.get_c_source_code()
    output.write_text(c_code)
    LOG.info('Created %s', output.name)
