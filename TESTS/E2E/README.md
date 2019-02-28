# Device Management E2E testing

Device Management E2E tests are platform agnostic. They verify that a target platform can reliably perform basic critical operations.
There are two different test sets:

- Tests for verifying the basic functionality of a target platform.
- Advanced tests for more complex use cases (for example `device_update` test case).

## Requirements

1. [IceTea](https://github.com/ARMmbed/icetea) v1.2.1 or later.
1. [Mbed Cloud SDK for Python](https://cloud.mbed.com/docs/current/mbed-cloud-sdk-python/index.html) v2.0.5 or later.

## Installation

```
pip install icetea mbed-cloud-sdk
```

## Basic usage

1. Register your device to Device Management. For examples, see the [tutorials](https://cloud.mbed.com/docs/current/connecting/device-management-client-tutorials.html).
1. Enter your [device_id](https://cloud.mbed.com/docs/current/connecting/device-identity.html) to `TESTS/E2E/pelion.tc_cfg`.
1. Enter your [API key](https://cloud.mbed.com/docs/current/integrate-web-app/api-keys.html) to `TESTS/E2E/pelion.tc_cfg`.

### Running the test suite

To run the test suite, use the following command:

```
icetea --suite basic_tests.json --suitedir TESTS/E2E/ --tcdir ./TESTS/E2E/ --tc_cfg TESTS/E2E/pelion.tc_cfg
```

If you have prepared a manifest (see below the instructions for `device_update` testcase), you can run the full suite with:

```
icetea --suite full_tests.json --suitedir TESTS/E2E/ --tcdir ./TESTS/E2E/ --tc_cfg TESTS/E2E/pelion.tc_cfg
```

### Running a single test

To run a single test, use the following command:

```
icetea --tc basic_get --tcdir ./TESTS/E2E/ --tc_cfg TESTS/E2E/pelion.tc_cfg
```

## Current tests

| Test name        | Main functions                             | Notes                                 |
| ---------------- | ------------------------------------------ | --------------------------------------|
| `register`       | Verify that the device is registered.      |                                       |
| `get`            | Verify that the device responds to GET.    | Uses OMA resource `/3/0/0`            |
| `put`            | Verify that the device responds to PUT.    | Uses custom resource `/3201/0/5853`   |
| `post`           | Verify that the device responds to POST.   | Uses custom resource `/3201/0/5850`   |
| `observation`    | Verify that the device can send notifications. | Uses custom resource `/3201/0/5853`   |
| `deregister`     | Verify that the device can deregister.     | Uses custom resource `/5000/0/1`      |
| `device_update`  | Performs the firmware update.              | This testcase verifies that the device can perform a firmware update. For this testcase, you need to manually generate an update image and a manifest. |

### Executing `device_update` test case

The test case does not automatically generate an update image or generate a manifest. Read [Update documentation](https://cloud.mbed.com/docs/current/updating-firmware/index.html) for more information.

To run the test case:

1. Compile a new update image.
1. Upload the image to Device Management.
1. Generate a manifest for the update image.
1. Upload the manifest to Device Management.
1. Add the `<manifest-id>` to `TESTS/E2E/pelion.tc_cfg`.

#### Example for executing the `device_update` test case on Mbed OS platform

To prepare the latest Device Management Client example on Mbed OS for testing, run the following commands:

1. `mbed dm init -d arm.com --model-name example-app --force -q`
1. `mbed compile -t <toolchain> -m <MCU>`
1. `mbed dm update prepare`

The last command uploads the `mbed-cloud-client-example_update.bin`, generates a new manifest and uploads the manifest to Device Management. Record the resulting Manifest ID and add it to `TESTS/E2E/pelion.tc_cfg`.

```
[INFO] 2018-12-12 15:06:42 - manifesttool.prepare - Manifest ID: <manifest-id>
```

To execute the test, use the following command:

```
icetea --tc device_update --tcdir ./TESTS/E2E/ --tc_cfg TESTS/E2E/pelion.tc_cfg
```

 <span class="notes">**Note:** After each successful update, you need to generate a new manifest for a new firmware update test. Otherwise, the test will pass without actually performing any device side updates. The update campaign ends automatically because the manifest has already been applied to the device.</span>
