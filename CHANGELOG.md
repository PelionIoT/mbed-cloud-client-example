# Changelog for Mbed Cloud Client Reference Example

## Release R1.3.1 (19.04.2018)

* Converted the LED blinking callback from a blocking loop to an event-based tasklet.
* Updated to Mbed OS 5.8.1.
* The platform-specific code has been rewritten to have common implementation which can be shared with other Cloud applications (source/platform/).
  * Enabled multipartition support for an application.
  * Enabled LittleFS support.
  * Enabled autoformat/autopartition for the storage (controllable via compile-time flags).

## Release R1.3.0 (27.3.2018)

Initial public release.

