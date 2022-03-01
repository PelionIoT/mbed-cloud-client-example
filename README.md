# Device Management Client reference application for connectivity

The [`mbed-cloud-client-example`](https://github.com/PelionIoT/mbed-cloud-client-example) is a reference application that uses [Pelion Device Management Client library](https://github.com/PelionIoT/mbed-cloud-client) and demonstrates how to build a connectivity application.

## Device Management Client connection tutorial  

This tutorial builds and flashes a Device Management Client application using either Linux (running on a PC) or Mbed OS.   
The application can then connect to a standard OMA Lightweight M2M server.  
The application uses developer mode that relies on a developer certificate, which you add to your software binary to allow test devices to connect to the server.  
In the production, you should use the factory flow.

### Linux

#### Requirements

This requires a Linux PC (64-bit Ubuntu/XUbuntu OS desktop environment).
See also the [Mbed CLI instructions](https://os.mbed.com/docs/mbed-os/latest/tools/developing-mbed-cli.html).

#### Connecting the device

1. Open a terminal, and clone the example repository to a convenient location on your development environment:

   ```
   git clone https://github.com/PelionIoT/mbed-cloud-client-example
   cd mbed-cloud-client-example
   ```
   
   <span class="notes">**Note:** If you want to speed up `mbed deploy`, you can remove components that are unnecessary for Linux, such as `mbed-os.lib` and the `drivers/` folder.</span>

2. Deploy the example repository:

   ```
   mbed deploy
   ```

3. [Create a developer certificate](#create-developer-cert).   

4. Copy the `mbed_cloud_dev_credentials.c` file to the root folder of the example.

5. Deploy Linux dependencies:

   ```
   python pal-platform/pal-platform.py deploy --target=x86_x64_NativeLinux_mbedtls generate
   cd __x86_x64_NativeLinux_mbedtls
   ```
   **Note: python2 is needed for the above command**

6. Generate `cmake` files based on your configuration and build profile (**Release** or **Debug**):

   - For the **Release** profile:
     ```
     cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define_lwm2m_compliant.txt
     ```
   
   - For the **Debug** profile:   
     ```
     cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define_lwm2m_compliant.txt
     ```
   - If you want your application to bypass the Bootstrap server and work directly with LwM2M server, please add `DISABLE_BOOTSTRAP` cmake flag:
     ```
     cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DDISABLE_BOOTSTRAP=ON -DCMAKE_TOOLCHAIN_FILE=./../pal-platform/Toolchain/GCC/GCC.cmake -DEXTERNAL_DEFINE_FILE=./../define_lwm2m_compliant.txt
     ```

7. Compile the application:

   ```
   make mbedCloudClientExample.elf
   ```
   
8. The build creates binaries under `mbed-cloud-client-example/__x86_x64_NativeLinux_mbedtls`. In both cases, there are subdirectories `Debug` and `Release` respectively created for the two profiles.

9. Run the application (at the respective path, see above):

   ```
   ./mbedCloudClientExample.elf
   ```

   You should see a message when the device connects to LwM2M server:

   ```
   Client registered
   Endpoint Name: <YOUR_ENDPOINT_NAME>
   Device Id: <YOUR_ENDPOINT_NAME>
   ```
10. If you want to run the application with a clean storage, you can remove the `pal` folder that is created in the location where you run your application. 


### Mbed OS

#### Prerequisites

To work with the Device Management Client example application, you need:

* A supported board with a network connection and an SD card attached. Currently FRDM K64F and NUCLEO F429ZI boards are supported.
* [Serial connection](https://os.mbed.com/docs/latest/tutorials/serial-comm.html) to your device with open terminal connection (baud rate 115200, 8N1).
* [Arm Mbed CLI](https://os.mbed.com/docs/mbed-os/latest/tools/index.html) installed. See [installation instructions](https://os.mbed.com/docs/latest/tools/installation-and-setup.html).
  * Make sure that all the Python components are in par with the `pip` package [requirements.txt](https://github.com/PelionIoT/mbed-os/blob/master/requirements.txt) list from Mbed OS.
* Updated [DAPLink](https://github.com/ARMmbed/DAPLink/releases) software (version 250 or later), if your board uses DAPLink.

#### Connecting the device

1. Clone the embedded application's GitHub repository to your local computer and navigate to the new folder:

    ```
    mbed import https://github.com/PelionIoT/mbed-cloud-client-example
    cd mbed-cloud-client-example
    ```

2. Configure Mbed CLI to use your board:

    ```
    mbed target <MCU>
    mbed toolchain GCC_ARM
    ```

3. [Create a developer certificate](#create-developer-cert).

4. Copy the `mbed_cloud_dev_credentials.c` file to the root folder of the example application.

5. Configure the example application:
   1. If you want your application to bypass the Bootstrap server and work directly with LwM2M server,
      please set the `disable-bootstrap-feature` feature to `true` in [mbed_app_lwm2m_compliant.json](https://github.com/PelionIoT/mbed-cloud-client-example/blob/master/mbed_app_lwm2m_compliant.json#L21).      
   
      ```
      mbed-client.disable-bootstrap-feature: true
      ```
      
   2. Currently, the application will always start with a clean storage. 
      If you want to avoid this, remove the `RESET_STORAGE` from the `"target.macros_add"` line in the `mbed_app_lwm2m_compliant.json`:
   
      ```
      "target.macros_add" : ["LWM2M_COMPLIANT","DISABLE_SERVER_CERT_VERIFY"],
      ```
            
7. Compile the example application:

   ```
   mbed compile --app-config mbed_app_lwm2m_compliant.json
   ```
   For more information about Mbed CLI parameters, please see the [Mbed OS documentation site](https://os.mbed.com/docs/mbed-os/latest/build-tools/mbed-cli-1.html).

8. Flash the binary to the device
   1. Connect the device to your computer over USB. It's listed as a mass storage device.   
   2. Drag and drop `mbed-cloud-client-example.bin` to the device, or alternatively add the `-f` flag to the build command (if your device is connected to the build machine). This flashes the binary to the device. You should see the LED blink rapidly; wait for it to stop.

9. Press the **Reset** button to restart the device and reset the terminal.
10. When the client has successfully connected, the terminal shows:

    ```
    Client registered
    Endpoint Name: <YOUR_ENDPOINT_NAME>
    Device ID: <YOUR_ENDPOINT_NAME>
    ```

<h3 id="create-developer-cert">Create a developer certificate</h3>

1. Download the server CA certificate from the LwM2M service you want to connect to and copy it to the scripts' folder:

   ```
   cp <SERVER_CA_CERT_FILE> utils/server_ca_cert.der
   ```

2. Run the python script `cert_convert.py` to generate a `mbed_cloud_dev_credentials.c` file.

   ```
   cd utils
   python cert_convert.py --endpoint <YOUR_ENDPOINT_NAME> --uri <The URI of the bootstrap or Device management service> --use-ca
   ```

   The script will do the following steps:
   1. Generate a root CA key and certificate on the first time the script is running. All CA outputs are stored in `CA` folder.
   1. Generate a private key and a certificate signed by this CA.
   1. Convert the private key, certificate and the server certificate to a C file.
   1. All non CA outputs are stored in a folder named `YOUR_ENDPOINT_NAME`.



    

