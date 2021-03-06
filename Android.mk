#
# Copyright (C) 2014  Mozilla Foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

# Build wifid
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/wifid.cpp \
    src/WifiMessageHandler.cpp \
    src/IpcHandler.cpp \
    src/WifiIpcHandler.cpp \
    src/WifiIpcManager.cpp

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src \
    bionic

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/src \
    external/stlport/stlport \

LOCAL_SHARED_LIBRARIES += \
    libhardware \
    libhardware_legacy \
    libcutils \
    libutils \
    liblog

LOCAL_MODULE := wifid
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -DDEBUG -DPLATFORM_ANDROID -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DSILENT=1 -DNO_SIGNALS=1 -DNO_EXECUTE_PERMISSION=1 -D_GNU_SOURCE -D_REENTRANT -DUSE_MMAP -DUSE_MUNMAP -D_FILE_OFFSET_BITS=64 -DNO_UNALIGNED_ACCESS

include $(BUILD_EXECUTABLE)
