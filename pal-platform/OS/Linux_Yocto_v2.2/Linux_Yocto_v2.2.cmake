#################################################################################
#  Copyright 2016-2020 ARM Ltd.
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

set (OS_BRAND Linux)

cmake_policy(SET CMP0003 NEW)
cmake_policy(SET CMP0011 OLD)

add_definitions(-DTARGET_IS_PC_LINUX)
add_definitions(-D__LINUX__)

option(STORAGE_ESFS "Enable ESFS" ON)
