## ----------------------------------------------------------------------------
## Copyright 2016-2018 ARM Ltd.
##
## SPDX-License-Identifier: Apache-2.0
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
## ----------------------------------------------------------------------------

## This makefile is only for SX OS platform

## ----------------------------------------------------------- ##
## Don't touch the next line unless you know what you're doing.##
## ----------------------------------------------------------- ##
include ${SOFT_WORKDIR}/env/compilation/compilevars.mk

export MBED_CLOUD_SERVICE = ${EXTERNAL_APP_FOLDER}

# Name of the module
LOCAL_NAME := ${MBED_CLOUD_SERVICE}

# list all modules APIs that are necessary to compile this module
LOCAL_API_DEPENDS := \
                    ${LOCAL_NAME}/mbed-cloud-client \
                    platform/service/posix \
                    ${API_PLATFORM_DEPENDS} \

LOCAL_ADD_INCLUDE += . \
                         ${LOCAL_NAME} \
                         ${LOCAL_NAME}/source \
                         ${LOCAL_NAME}/source/include \
                         ${LOCAL_NAME}/source/platform/include \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/key-config-manager/source/include \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/key-config-manager/key-config-manager \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/factory-configurator-client/factory-configurator-client \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/fcc-output-info-handler/fcc-output-info-handler \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/fcc-bundle-handler/fcc-bundle-handler \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/storage/storage \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/mbed-client-esfs/source/include \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/crypto-service/crypto-service \
                         ${LOCAL_NAME}/mbed-cloud-client/factory-configurator-client/mbed-trace-helper/mbed-trace-helper \
                         ${LOCAL_NAME}/mbed-cloud-client \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-client \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-client/mbed-client \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-trace \
                         ${LOCAL_NAME}/mbed-cloud-client/source \
                         ${LOCAL_NAME}/mbed-cloud-client/ns-hal-pal \
                         ${LOCAL_NAME}/mbed-cloud-client/nanostack-libservice/mbed-client-libservice \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-client-pal/Source/PAL-Impl/Services-API \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-client-pal/Configs/pal_config \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-client-pal/Configs/pal_config/SXOS \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-client/mbed-client-c \
                         ${LOCAL_NAME}/mbed-cloud-client/mbed-coap/mbed-coap \
                         ${LOCAL_NAME}/mbed-cloud-client/nanostack-libservice \
                         ${LOCAL_NAME}/mbed-cloud-client/sal-stack-nanostack-eventloop \
                         ${LOCAL_NAME}/mbed-cloud-client/certificate-enrollment-client/certificate-enrollment-client \
                         ${LOCAL_NAME}/mbed-cloud-client/update-client-hub \
                         ${LOCAL_NAME}/mbed-cloud-client/update-client-hub/modules/common \

LOCAL_EXPORT_FLAG += __SXOS__

LOCAL_EXPORT_FLAG += "'PAL_USER_DEFINED_CONFIGURATION=\"SXOS/sxos_sotp.h\"'"
LOCAL_EXPORT_FLAG += "'MBED_CLIENT_USER_CONFIG_FILE=\"mbed_cloud_client_user_config.h\"'"
LOCAL_EXPORT_FLAG += "'MBED_CLOUD_CLIENT_USER_CONFIG_FILE=\"mbed_cloud_client_user_config.h\"'"

LOCAL_EXPORT_FLAG += "'MBED_CONF_MBED_CLIENT_EVENT_LOOP_SIZE=12000'"
LOCAL_EXPORT_FLAG += "PAL_SIMULATOR_FLASH_OVER_FILE_SYSTEM=1"
LOCAL_EXPORT_FLAG += "MBED_CONF_MBED_TRACE_ENABLE"
LOCAL_EXPORT_FLAG += "MBED_CONF_APP_CLOUD_MODE=1"
LOCAL_EXPORT_FLAG += "MBED_CONF_APP_DEVELOPER_MODE=1"

# not yet
LOCAL_EXPORT_FLAG += "MBED_CONF_MBED_CLOUD_CLIENT_DISABLE_CERTIFICATE_ENROLLMENT=1"

# Disable code using STL as it not available on SDK
LOCAL_EXPORT_FLAG += "MBED_CLOUD_CLIENT_STL_API=0"
LOCAL_EXPORT_FLAG += "MBED_CLOUD_CLIENT_STD_NAMESPACE_POLLUTION=0"

# Compile the sub-modules, except when the "service" must be used as a library.
# list all the modules that need to be compiled prior to using this module

LOCAL_MODULE_DEPENDS += \
                         ${LOCAL_NAME}/mbed-cloud-client \

# One can compile and run the PAL unit tests instead of example-app
# by "PAL_UNIT_TESTING=1 ctmake CT_PRODUCT=20180930 dbmerge -j" command.
#
# By default, the example app itself is built. By having the PAL tests in
# same makefile, one can easily be sure the configuration is the same and
# the changes in PAL during porting can be tested by its own unit tests and
# then running the example itself. 
ifndef PAL_UNIT_TESTING

# this enables the mbed_cloud_application_entrypoint(void) from main.cpp
LOCAL_EXPORT_FLAG += MBED_CLOUD_APPLICATION_NONSTANDARD_ENTRYPOINT

else
# the PAL tests have mbed_cloud_application_entrypoint(void) too, which is enabled
# from pal_tests_nonstandard_entrypoint.c by PAL_UNIT_TESTING_NONSTANDARD_ENTRYPOINT.
LOCAL_EXPORT_FLAG += "PAL_UNIT_TESTING_NONSTANDARD_ENTRYPOINT"

# build the tests
LOCAL_MODULE_DEPENDS += ${LOCAL_NAME}/mbed-cloud-client/mbed-client-pal/Test

# These are needed for building PAL SOTP tests. Note: the flag is renamed on yet-to-be-merged 
# version of esfs without backwards compatibility, so we need to have both here for some time.
LOCAL_EXPORT_FLAG += "SOTP_TESTING"
LOCAL_EXPORT_FLAG += "RBP_TESTING"
endif

# Generate the revision (version) file automatically during the make process.
AUTO_GEN_REVISION_HEADER := no

# This is a top-level module
IS_TOP_LEVEL := yes

# Generates the CoolWatcher headers automatically.
AUTO_XMD2H ?= no

# code is not in one "src/" directory as SDK expects by default
USE_DIFFERENT_SOURCE_LAYOUT := yes
USE_DIFFERENT_SOURCE_LAYOUT_ARM := yes

C_SRC := ${wildcard *.c}
C++_SRC := ${wildcard *.cpp}

C_SRC += ${wildcard source/*.c}
C++_SRC += ${wildcard source/*.cpp}

C_SRC += ${wildcard source/platform/SXOS/*.c}
C++_SRC += ${wildcard source/platform/SXOS/*.cpp}

C_SRC += ${wildcard mbed-cloud-client/mbed-trace/source/*.c}
C_SRC += ${wildcard mbed-cloud-client/nanostack-libservice/source/libip6string/*.c}


## ------------------------------------- ##
##  Do Not touch below this line         ##
## ------------------------------------- ##
include ${SOFT_WORKDIR}/env/compilation/compilerules.mk
