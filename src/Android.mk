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

COMMON_SRC_FILES := \
    CommandLine.cpp \
    Daemon.cpp \
    ipc/IpcHandler.cpp \
    ipc/WifiIpcHandler.cpp \
    ipc/IpcManager.cpp \
    MessageConsumer.cpp \
    MessageHandlerListener.cpp \
    MessageHandler.cpp \
    MessageProducer.cpp \
    MessageQueueWorker.cpp \
    controller/Controller.cpp \
    controller/HostApdController.cpp \
    controller/WpaSupplicantController.cpp

COMMON_INCLUDE := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/ipc \
    $(LOCAL_PATH)/controller \
    $(LOCAL_PATH)/utils \
    bionic

COMMON_INCLUDE += \
    $(LOCAL_PATH) \
    external/stlport/stlport \

COMMON_INCLUDE += \
    $(call include-path-for, libhardware_legacy)/hardware_legacy

COMMON_CFLAGS := -DDEBUG -DPLATFORM_ANDROID -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_DLFCN_H=1 -DSILENT=1 -DNO_SIGNALS=1 -DNO_EXECUTE_PERMISSION=1 -D_GNU_SOURCE -D_REENTRANT -DUSE_MMAP -DUSE_MUNMAP -D_FILE_OFFSET_BITS=64 -DNO_UNALIGNED_ACCESS -D_STLP_USE_NEWALLOC

COMMON_SHARED_LIBRARIES += \
    libhardware \
    libhardware_legacy \
    libwpa_client \
    libcutils \
    libutils \
    liblog

# Build wifid
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    main.cpp \
    $(COMMON_SRC_FILES)

LOCAL_C_INCLUDES := $(COMMON_INCLUDE)

LOCAL_SHARED_LIBRARIES += $(COMMON_SHARED_LIBRARIES)
LOCAL_MODULE := wifid
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(COMMON_CFLAGS)

include $(BUILD_EXECUTABLE)

###############################################
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    test/main.cpp \
    $(COMMON_SRC_FILES)

LOCAL_C_INCLUDES := $(COMMON_INCLUDE)

LOCAL_SHARED_LIBRARIES += $(COMMON_SHARED_LIBRARIES)
LOCAL_MODULE := testwifid
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS := $(COMMON_CFLAGS)

include $(BUILD_EXECUTABLE)
