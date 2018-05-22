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

SET(COMPILATION_DETAILS_FILE_NAME "${CMAKE_SOURCE_DIR}/compilation_info.txt")

macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
        SET(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
        SET(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
endmacro(SET_COMPILER_DBG_RLZ_FLAG)



MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

MACRO(ADD_GLOBALDIR dirname)
	include_directories(${dirname})
	file(APPEND ${INCLUDE_FILE_NAME} "-I${dirname} ")
ENDMACRO()


MACRO(ADDSUBDIRS_CMAKE)
        SUBDIRLIST(SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR})
        FOREACH(subdir ${SUBDIRS})
                if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt)
                    if(NOT (${subdir} MATCHES "__.+"))
                        ADD_SUBDIRECTORY(${subdir} ${CMAKE_CURRENT_BINARY_DIR}/${subdir})
                    endif()
                endif()
        ENDFOREACH()
ENDMACRO(ADDSUBDIRS_CMAKE)

MACRO(ADDSUBDIRS)
        ADDSUBDIRS_CMAKE()
        get_directory_property(hasParent PARENT_DIRECTORY)
        if(hasParent)
                set (SRC_LIBS ${SRC_LIBS} PARENT_SCOPE)
                set (TEST_LIBS ${TEST_LIBS} PARENT_SCOPE)
        endif()
ENDMACRO()

macro(CREATE_LIBRARY NAME SOURCE_LIST DEFINES)
      file(APPEND "${COMPILATION_DETAILS_FILE_NAME}" "\n${NAME}:\n${SOURCE_LIST}")
      add_library(${NAME} STATIC ${SOURCE_LIST})
      list (APPEND SRC_LIBS ${NAME})
      get_directory_property(hasParent PARENT_DIRECTORY)
      if(hasParent)
              set (SRC_LIBS ${SRC_LIBS} PARENT_SCOPE)
      endif()
      target_compile_definitions(${NAME} PRIVATE  ${DEFINES})
endmacro()



macro(CREATE_TEST_LIBRARY NAME SOURCE_LIST DEFINES)
      file(APPEND "${COMPILATION_DETAILS_FILE_NAME}" "\n${NAME}:\n${SOURCE_LIST}")
      add_library(${NAME} STATIC ${SOURCE_LIST})
      list (APPEND TEST_LIBS ${NAME})
      get_directory_property(hasParent PARENT_DIRECTORY)
      if(hasParent)
              set (TEST_LIBS ${TEST_LIBS} PARENT_SCOPE)
      endif()
      target_compile_definitions(${NAME} PRIVATE  ${DEFINES})
endmacro()

macro(SET_COMPILER_DBG_RLZ_FLAG flag value)
    SET(${flag}_DEBUG "${${flag}_DEBUG} ${value}")
    SET(${flag}_RELEASE "${${flag}_RELEASE} ${value}")
#enable this if for debugging
if (0)
 message("flag = ${flag}")
 message("value = ${value}")
 message("MY_C_FLAGS_RELEASE2 = ${CMAKE_C_FLAGS_RELEASE}")
endif(0) # comment end
endmacro(SET_COMPILER_DBG_RLZ_FLAG)
