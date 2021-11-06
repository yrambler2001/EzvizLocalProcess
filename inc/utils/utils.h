#pragma once

#include <array>
#include <chrono>
#include <string>
#include "json/json.h"
#include <Windows.h>

#define OpenSDK_Success(x) ((x) == 0)
#define Assert_S(x) do { LOG_TO_FILE_ASSERT_IF(!OpenSDK_Success(x), LOG::error, "code: %d, desc: %s", OpenSDK_GetLastErrorCode(), OpenSDK_GetLastErrorDesc()) } while (0)
#define Assert(x) do { LOG_TO_FILE_ASSERT_IF(!!!(x), LOG::error, #x) } while (0)

namespace utils
{
  constexpr int                        RecordTypeNone{ 0 };
  constexpr int                        RecordTypeLocal{ 1 };
  constexpr int                        RecordTypeCloud{ 2 };
  constexpr std::array<const char*, 3> RecordTypeStr{ "none", "local", "cloud" };

  Json::Value ParseJson(const std::string& jstr);

  std::string JsonToString(const Json::Value& j, bool beauty);

  void WaitFlag(HANDLE f);

  void ClearFlag(HANDLE f);

  void TriggerFlag(HANDLE f);

  bool FileExist(const std::string& path);

  void CreateDirectoryRecursively(const std::string& directory);

  void CreateDirectoryRecursivelyForFile(const std::string& file);

  bool SaveDataTo(const std::string& path, const std::vector<uint8_t>& D);
  bool SaveDataTo(const std::string& path, const std::vector<uint8_t>& D, bool overwrite);

  std::string MakePath(const std::string& s);
  template <class T, class ...Args>
  std::string MakePath(T L, Args... rest)
  {
    return L + std::string("\\") + MakePath(rest...);
  }

  std::string FormatRecordTime(tm tt);
  std::string FormatRecordTime(time_t t);

  tm ParseTimeStringToTm(const std::string& str);
  time_t ParseTimeStringToTimeT(const std::string& str);

  tm GetTodayStart(tm tt);

  tm GetNextDayStart(tm tt);

  std::string CurrentTimeToRecordFormat(tm* pt, time_t* ptt);
}