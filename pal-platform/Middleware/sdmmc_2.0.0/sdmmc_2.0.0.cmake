#################################################################################
#  Copyright 2016, 2017 ARM Ltd.
#  
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#  
#      http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#################################################################################

include_directories(./pal-platform/Middleware/sdmmc_2.0.0/sdmmc_2.0.0/inc)

#add_subdirectory ("./Platform/Middleware/lwip_1.4.1/lwip_1.4.1")
set (EXTRA_CMAKE_DIRS ${EXTRA_CMAKE_DIRS} "./pal-platform/Middleware/sdmmc_2.0.0/sdmmc_2.0.0")
list (APPEND PLATFORM_LIBS sdmmc_2.0.0)
      