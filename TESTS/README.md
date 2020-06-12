## Validation and testing for the client configuration with Pelion end-to-end python test library

Basic requirements for Pelion capabilities:

- Connects to Pelion in developer mode.
- Firmware can be updated.
- Responsive to REST API commands.

Installation of the test-library:

- Install the prerequisites listed in the README of the [pelion-e2e-python-test-library](https://github.com/ARMmbed/pelion-e2e-python-test-library).
- Configure your API-key as instructed in the same README.


Basic tests can be then executed as:

`pytest TESTS/pelion-e2e-python-test-library/tests/dev-client-tests.py --update_bin=/home/user/mbed-cloud-client-example/mbed-cloud-client-example_update.bin`
