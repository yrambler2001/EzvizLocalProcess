
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include "json/json.h"
#include "log/log.h"
#include "utils/utils.h"
#include <Windows.h>
#include "cmdline/cmdline.h"

#include "OpenSDK/OpenNetStreamInterFace.h"

using namespace std;

struct File
{
  std::string         mStartTime;
  std::string         mEndTime;
};

class Output {
public:
  bool            mFinish{ false };
  bool            mError{ false };
  bool            mReceiveStreamEnd{ false };
  vector<uint8_t> mContent;
};

Output gResult;

void __stdcall Receiver_MessageHandler(const char* szSessionId, unsigned int iMsgType, unsigned int iErrorCode, const char* pMessageInfo, void* pUser)
{
  LOG_INFO("[+] callback, session id: %s, msg type: %d, err code: %d, msg info: %s", szSessionId, iMsgType, iErrorCode, pMessageInfo);

  string sessionId = szSessionId;
  uint32_t msgType = iMsgType;
  uint32_t errorCode = iErrorCode;
  string messageInfo = pMessageInfo;


  switch (msgType)
  {
  case INS_DOWNLOAD_START:
  case INS_PLAY_START:
    if (errorCode != 0) {
      LOG_ERROR("[-] error exit");
      gResult.mError = true;
      gResult.mFinish = true;
      return;
    }
    Assert(gResult.mContent.empty());
    LOG_INFO("[+] receive download start");
    break;

  case INS_DOWNLOAD_STOP:
  case INS_PLAY_ARCHIVE_END: {
    if (errorCode == 0) {
      LOG_INFO("[%c] callback, download finish", "-+"[gResult.mReceiveStreamEnd]);
    }
    else {
      LOG_ERROR("[-] error exit");
      gResult.mError = true;
    }
    gResult.mFinish = true;
    break;
  }

  case INS_DOWNLOAD_EXCEPTION:
  case INS_PLAY_ARCHIVE_EXCEPTION:
    LOG_WARN("[-] callback, download failed");
    gResult.mFinish = true;
    break;

  case 92:
    LOG_INFO("update network info: %s", messageInfo.c_str());
    break;

  default:
    LOG_INFO("ignored message: %u", msgType);
    break;
  }
}

void __stdcall Receiver_StreamDataHandler(DataType enType, char* const pData, int iLen, void* pUser)
{
  LOG_DEBUG("data callback, data type: %d, data length: %d", enType, iLen);

  switch (enType)
  {
  case NET_DVR_SYSHEAD:
    LOG_INFO("receive stream header");
    Assert(gResult.mContent.empty());
    break;
  case NET_DVR_RECV_END:
    LOG_INFO("receive stream end");
    gResult.mReceiveStreamEnd = true;
    // go on
  case NET_DVR_STREAMDATA:
    gResult.mContent.insert(gResult.mContent.end(), pData, pData + iLen);
    break;
  default:
    LOG_WARN("unknown type pkg: %d, length: %d", enType, iLen);
    break;
  }
}

bool InitSDK(const std::string& accessToken, std::string& sessionId) {
  LOG_INFO("[+] init sdk, token size: %u", accessToken.size());

  OPENSDK_RESULT r;
  r = OpenSDK_InitLib("https://openauth.ezvizlife.com", "https://open.ezvizlife.com", "");
  if (!OpenSDK_Success(r)) {
    LOG_ERROR("[-] init lib failed, %u", r);
    return false;
  }

  OpenSDK_SetConfigInfo(CONFIG_LOG_LEVEL, 1); // EZOPENSDK_LogLevel
  OpenSDK_SetConfigInfo(CONFIG_OPEN_STREAMTRANS, 1);
  OpenSDK_SetConfigInfo(CONFIG_DATA_UTF8, 1);

  char* _sessionId = nullptr;
  int   sessionIdLen = 0;
  r = OpenSDK_AllocSessionEx(Receiver_MessageHandler, nullptr, &_sessionId, &sessionIdLen);
  if (!OpenSDK_Success(r)) {
    LOG_ERROR("[-] alloc session failed, %u", r);
    return false;
  }
  sessionId.assign(_sessionId, _sessionId + sessionIdLen);
  LOG_INFO("[+] session id: %s", sessionId.c_str());
  r = OpenSDK_SetDataCallBack(sessionId.c_str(), Receiver_StreamDataHandler, nullptr);
  if (!OpenSDK_Success(r)) {
    LOG_ERROR("[-] set data callback failed, %u", r);
    return false;
  }
  r = OpenSDK_SetAccessToken(accessToken.c_str());
  if (!OpenSDK_Success(r)) {
    LOG_ERROR("[-] set access token failed, %u", r);
    return false;
  }
  return OpenSDK_Success(r);
}

bool DownloadLocalRecords(
  const std::string& sessionId,
  const std::string& deviceSerial,
  const std::string& secretKey,
  int cameraNo,
  const std::string& startTime,
  const std::string& endTime
) {
  LOG_INFO("[+] start downloading local records, session: %s, serial: %s, key len: %u, camera: %d, range: %s ~ %s",
    sessionId.c_str(), deviceSerial.c_str(), secretKey.size(), cameraNo, startTime.c_str(), endTime.c_str());

  // 先搜索，不然提示没录像...
  OPENSDK_RESULT r2 = OpenSDK_StartSearchEx(
    sessionId.c_str(),
    deviceSerial.c_str(),
    cameraNo,
    startTime.c_str(),
    endTime.c_str()
  );
  this_thread::sleep_for(2s);
  if (!OpenSDK_Success(r2)) {
      LOG_ERROR("[-] r2 failed, %u", r2);
  }

  OPENSDK_RESULT r = OpenSDK_StartPlayBackEx(
    sessionId.c_str(),
    NULL,
    deviceSerial.c_str(),
    cameraNo,
    secretKey.c_str(),
    startTime.c_str(),
    endTime.c_str()
  );
  //OPENSDK_RESULT r = OpenSDK_StartDownload(
  //    sessionId.c_str(),
  //    deviceSerial.c_str(),
  //    cameraNo,
  //    "",
  //    startTime.c_str(),
  //    endTime.c_str()
  //);

  //void* pBuf = NULL;
  //int length = 0;
  //OPENSDK_RESULT r = OpenSDK_Data_GetDevListEx(
  //    0,
  //    1000,
  //    &pBuf, &length
  //);
  //auto json = static_cast<char*>(pBuf);

  if (!OpenSDK_Success(r)) {
    LOG_ERROR("[-] start donwloading local record failed, %u", r);
    return false;
  }
  return OpenSDK_Success(r);
}

int main(int argc, char** argv) {
  const char* ACCESSTOKEN = "-accessToken";
  const char* DEVICESERIAL = "-deviceSerial";
  const char* SECRETKEY = "-secretKey";
  const char* CAMERANO = "-cameraNo";
  const char* STARTTIME = "-startTime";
  const char* ENDTIME = "-endTime";
  const char* SAVEDIR = "-savePath";

  CmdLineParser cp;
  cp.AddPartern(ACCESSTOKEN, 1, true);
  cp.AddPartern(DEVICESERIAL, 1, true);
  cp.AddPartern(SECRETKEY, 1, true);
  cp.AddPartern(CAMERANO, 1, true);
  cp.AddPartern(STARTTIME, 1, true);
  cp.AddPartern(ENDTIME, 1, true);
  cp.AddPartern(SAVEDIR, 1, true);
  cp.AddHelp(
    string(argv[0]) + " " +
    ACCESSTOKEN     + " <token> " +
    DEVICESERIAL    + " <serial> " +
    SECRETKEY       + " <key> " +
    CAMERANO        + " <No.> " +
    STARTTIME       + " <2020-01-01 00:00:00> " + 
    ENDTIME         + " <2020-01-01 23:59:59> " + 
    SAVEDIR         + " <path>"
  );
  if (!cp.Parse(argc, argv)) {
    cout << cp.HelpInfo() << "\n";
    return 1;
  }

  string sessionId;
  if (!InitSDK(cp.GetValue(ACCESSTOKEN).GetValue0(), sessionId)) {
    cout << "[-] Init yssdk failed.\n";
    return 1;
  }

  if (!DownloadLocalRecords(
    sessionId,
    cp.GetValue(DEVICESERIAL).GetValue0(),
    cp.GetValue(SECRETKEY).GetValue0(),
    strtol(cp.GetValue(CAMERANO).GetValue0().c_str(), nullptr, 10),
    cp.GetValue(STARTTIME).GetValue0(),
    cp.GetValue(ENDTIME).GetValue0()
  )) {
    cout << "[-] Start downloading failed.\n";
    return 1;
  }

  while (!gResult.mFinish)
    this_thread::sleep_for(1s);

  LOG_INFO("[+] finished with error: %d, save path: %s", gResult.mError, cp.GetValue(SAVEDIR).GetValue0().c_str());
  bool success = !gResult.mError && utils::SaveDataTo(cp.GetValue(SAVEDIR).GetValue0(), gResult.mContent, true);
  LOG_INFO("[%c] save file %s", "-+"[success], (success ? "success" : "failed"));
  return !success;
}