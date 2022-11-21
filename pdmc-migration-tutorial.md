# Device Management Client migration tutorial

Starting with release 4.13.0 PDMC supports migration from one bootstrap server to another.  
This is available now only for Mbed OS-based devices that use KVStore as the storage solution.

Valid use-cases:
* Transferring devices from a US-based server to an EU-based server due to GDPR requirements.
* Transferring devices from one standard OMA LwM2M server to another.

Follow the next steps to compile an image with the migration feature enabled.
1. Enable the migration feature by modifying `mbed_cloud_client_user_config.h` file as follows:
   ```  
   #define MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP
   #define MBED_CLOUD_CLIENT_MIGRATE_BOOTSTRAP_TO "coaps://new-bootstrap-server-uri.com:port"
   ```
2. Generate a .C-based file for the Bootstrap server certificate.
   ```
   python utils/cnvfile2hex.py --server-cert server_ca_cert.der --output migrate_server_cert.c
   ```
   `server_car_cert.der` - the Bootstrap server certificate in DER format of the new server. 
3. Compile the image with the relevant `*.json` configuration file.
   For transferring the device from one standard OMA LwM2M server to another, use `mbed_app_lwm2m_compliant.json` file.    
   ```
   mbed compile --app-config mbed_app_lwm2m_compliant.json
   ```
   **Note**: the image must be compiled in production mode, i.e. following modifications in the `*.json` file are needed:
   ```
   "*": {
       ...
       "target.OUTPUT_EXT" : "hex"
   },
   ...
   "developer-mode": {
          ...
          "value"     : null
   },   
   ```

Update the device with the compiled image via the firmware update (FOTA) campaign.

**Notes:**
   * As part of the migration, the LwM2M credentials are wiped from the device, thus forcing a new bootstrap operation with the new server URI.
   * The migration state is tracked via a new KVStore entry `pelion_wMIGR`.   
     If you need to migrate the device a 2nd time you should either remove that KVStore entry via another FOTA or modify the code to use a different entry for the 2nd migration (for example `pelion_wMIGR2`).
