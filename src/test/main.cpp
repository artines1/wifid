/*
 * Copyright (C) 2014-2015  Mozilla Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "Daemon.h"
#include "WifiDebug.h"
#include "WifiMessage.h"

/************************************************************************************
 **  Constants & Macros
 ************************************************************************************/

#define MAX_BUFFER_SIZE 2048
#define MOCK_SOCK_NAME "test"

/************************************************************************************
 **  Local type definitions
 ************************************************************************************/

/************************************************************************************
 **  Static variables
 ************************************************************************************/

static pthread_t sDaemonThread;
static pthread_t sMockIpcThread;
static int32_t sIpcFd;

/************************************************************************************
 **  Static functions
 ************************************************************************************/

static void* StaticDaemonThreadFunc(void* aArg);
static void* StaticMockIpcThreadFunc(void* aArg);
static void Log(const char *fmt_str, ...);

/************************************************************************************
 **  Test utils functions
 ************************************************************************************/

int32_t SendCommand(uint8_t* aData, size_t aDataLen);
int32_t PrintResponse(uint8_t* aData, size_t aDataLen);

using namespace wifi;
static void* StaticDaemonThreadFunc(void* aArg)
{
  CommandLineOptions options = { false, MOCK_SOCK_NAME, true };
  if (!Daemon::Initialize(&options)) {
    abort();
  }

  Daemon::GetInstance()->Start();
  Daemon::GetInstance()->ShutDown();
  return NULL;
}

static void Log(const char *fmt_str, ...)
{
  static char buffer[1024];
  va_list ap;

  va_start(ap, fmt_str);
  vsnprintf(buffer, 1024, fmt_str, ap);
  va_end(ap);

  fprintf(stdout, "%s\n", buffer);
}

static void* StaticMockIpcThreadFunc(void* aArg)
{
  int32_t ret;
  uint8_t buf[MAX_BUFFER_SIZE];
  struct sockaddr_un hostaddr, peeraddr;
  socklen_t socklen = sizeof(hostaddr);
  size_t len, siz;
  int32_t fd;
  int32_t sockFd;

  sockFd = socket(AF_UNIX, SOCK_SEQPACKET, 0);

  if (sockFd < 0) {
    Log("Could not create %s socket: %s\n", MOCK_SOCK_NAME, strerror(errno));
    abort();
  }

  len = strlen(MOCK_SOCK_NAME);
  hostaddr.sun_family = AF_UNIX;
  hostaddr.sun_path[0] = '\0'; /* abstract socket namespace */
  memcpy(hostaddr.sun_path + 1, MOCK_SOCK_NAME, len + 1);
  socklen_t addrLen = offsetof(struct sockaddr_un, sun_path) + len + 2;

  ret = bind(sockFd, reinterpret_cast<struct sockaddr*>(&hostaddr), addrLen);

  if (ret < 0) {
    Log("Could not bind %s socket: %s\n", MOCK_SOCK_NAME, strerror(errno));
    close(sockFd);
    abort();
  }

  ret = listen(sockFd, 4);

  if (ret < 0) {
    Log("Could not listen %s socket: %s\n", MOCK_SOCK_NAME, strerror(errno));
    close(sockFd);
    abort();
  }

  sIpcFd = accept(sockFd, (struct sockaddr*)&peeraddr, &socklen);

  if (sIpcFd < 0) {
    Log("Error on accept(): %s\n", strerror(errno));
    close(sockFd);
    abort();
  }

  while (1) {
    memset(buf, 0, MAX_BUFFER_SIZE * sizeof(uint8_t));
    int len = read(sIpcFd, buf, MAX_BUFFER_SIZE);
    PrintResponse(buf, len);
  }
  close(sIpcFd);
  return NULL;
}

/************************************************************************************
 **  Helper
 ************************************************************************************/
const char * GetTypeString(int val )
{
  return (const char *[]) {
    "WIFI_MESSAGE_TYPE_VERSION",
    "WIFI_MESSAGE_TYPE_LOAD_DRIVER",
    "WIFI_MESSAGE_TYPE_UNLOAD_DRIVER",
    "WIFI_MESSAGE_TYPE_START_SUPPLICANT",
    "WIFI_MESSAGE_TYPE_STOP_SUPPLICANT",
    "WIFI_MESSAGE_TYPE_CONNECT_TO_SUPPLICANT",
    "WIFI_MESSAGE_TYPE_CLOSE_SUPPLICANT_CONNECTION",
    "WIFI_MESSAGE_TYPE_COMMAND",
    "WIFI_MESSAGE_TYPE_CONNECT_TO_HOSTAPD",
    "WIFI_MESSAGE_TYPE_CLOSE_HOSTAPD_CONNECTION",
    "WIFI_MESSAGE_TYPE_HOSTAPD_GET_STATIONS",
    "WIFI_MESSAGE_TYPE_HOSTAPD_COMMAND"
  }
  [val];
}

const char * GetStatusString(int val )
{
  return (const char *[]) {
    "WIFI_STATUS_OK",
    "WIFI_STATUS_ERROR"
  }
  [val];
}

/************************************************************************************
 **  Setup
 ************************************************************************************/

int32_t SetUpDaemon()
{
  if (pthread_create(&sDaemonThread, NULL, StaticDaemonThreadFunc, NULL) != 0) {
    return -1;
  }

  Log("SetUpDaemon");
  return 1;
}

int32_t SetUpMockIpcSock()
{
  if (pthread_create(&sMockIpcThread, NULL, StaticMockIpcThreadFunc, NULL) != 0) {
    return -1;
  }

  Log("SetUpMockIpcSock");
  return 0;
}

/************************************************************************************
 **  Command and reponse
 ************************************************************************************/

int32_t SendCommand(uint8_t* aData, size_t aDataLen)
{
  size_t writeOffset = 0;
  int32_t size;
  uint16_t msgCategory;
  uint16_t msgType;

  msgCategory = WIFI_MSG_GET_CATEGORY(aData);

  if (msgCategory != WIFI_MESSAGE_REQUEST) {
    Log("Sent wrong category Message: %d.",
        msgCategory);
    return -1;
  }

  msgType = WIFI_MSG_GET_TYPE(aData);

  Log("SendCommand %s len %d", GetTypeString(msgType), aDataLen);

  if (!aData) {
    return -1;
  }

  while (writeOffset < aDataLen) {
    do {
      size = write(sIpcFd, aData + writeOffset, aDataLen - writeOffset);
    } while (size < 0 && errno == EINTR);

    if (size < 0) {
      Log("Response: unexpected error on write errno:%d", errno);
      return -1;
    }

    writeOffset += size;
  }

  return 0;
}

int32_t PrintResponse(uint8_t* aData, size_t aDataLen)
{
  uint16_t msgCategory;
  uint16_t msgType;
  uint16_t mStatus;

  msgCategory = WIFI_MSG_GET_CATEGORY(aData);

  if (msgCategory != WIFI_MESSAGE_RESPONSE && msgCategory != WIFI_MESSAGE_NOTIFICATION) {
    Log("The wifi daemon response wrong category Message: %d.",
        msgCategory);
    return -1;
  }

  if (msgCategory == WIFI_MESSAGE_NOTIFICATION) {
    Log("Receive notification %s len %d", aData, aDataLen);
  } else {
    msgType = WIFI_MSG_GET_TYPE(aData);
    mStatus = reinterpret_cast<WifiMsgResp*>(aData)->status;
    Log("Receive response %s status %s len %d", GetTypeString(msgType), GetStatusString(mStatus), aDataLen);
  }
  return 0;
}
/************************************************************************************
 **  Test functions
 ************************************************************************************/
void LoadDriver(void)
{
  // Command follows format in WifiMessage.h
  WifiMsgHeader hdr = { htons(0x00), htons(0x01), htonl(0x02) };
  uint16_t sessionId = htons(0x11);
  WifiMsgReq req = { hdr, sessionId, {} };
  SendCommand(reinterpret_cast<uint8_t*>(&req), sizeof(req));
}

int main(int argc, char* argv[]) {
  if (SetUpMockIpcSock() < 0) {
    return EXIT_FAILURE;
  }
  sleep(1);

  if (SetUpDaemon() < 0) {
    return EXIT_FAILURE;
  }

  sleep(1);
  LoadDriver();

  sleep(3);
  return EXIT_SUCCESS;
}
