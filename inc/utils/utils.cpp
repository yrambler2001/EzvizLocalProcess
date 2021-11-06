
#include <fstream>
#include <iomanip>

#include "utils.h"
#include "log/log.h"

using namespace std;

namespace utils
{
  Json::Value ParseJson(const string& jstr)
  {
    Json::Value root;
    Json::CharReaderBuilder builder;
    Json::CharReader* jsonReader = builder.newCharReader();
    if (!jsonReader->parse(jstr.data(), jstr.data() + jstr.size(), &root, nullptr))
      throw;
    if (!root.isObject())
      throw;
    return root;
  }

  std::string JsonToString(const Json::Value& j, bool beauty)
  {
    Json::StreamWriterBuilder b;
    Json::Value settings;
    b.setDefaults(&settings);
    if (beauty)
      settings["indentation"] = "  ";
    else
      settings["indentation"] = "";
    b.settings_ = settings;
    Json::StreamWriter* sw = b.newStreamWriter();
    stringstream ss;
    sw->write(j, &ss);
    return ss.str();
  }


  void WaitFlag(HANDLE f)
  {
    WaitForSingleObject(f, INFINITE);
  }

  void ClearFlag(HANDLE f)
  {
    ResetEvent(f);
  }

  void TriggerFlag(HANDLE f)
  {
    SetEvent(f);
  }

  bool FileExist(const string& path)
  {
    ifstream in;
    in.open(path, ios::binary | ios::in);
    bool r = in.is_open();
    if (in.is_open())
      in.close();
    return r;
  }

  void CreateDirectoryRecursively(const std::string& directory)
  {
    static const std::string separators("\\/");

    DWORD fileAttributes = GetFileAttributesA(directory.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES)
    {
      std::size_t slashIndex = directory.find_last_of(separators);
      if (slashIndex != std::string::npos)
        CreateDirectoryRecursively(directory.substr(0, slashIndex));

      if (!CreateDirectoryA(directory.c_str(), nullptr))
        throw std::runtime_error("Could not create directory");
    }
    else
    {
      bool isDirectoryOrJunction =
        ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
        ((fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0);

      if (!isDirectoryOrJunction)
        throw std::runtime_error("Could not create directory because a file with the same name exists");
    }
  }

  void CreateDirectoryRecursivelyForFile(const std::string& file)
  {
    static const std::string separators("\\/");
    std::size_t slashIndex = file.find_last_of(separators);
    if (slashIndex != std::string::npos)
      CreateDirectoryRecursively(file.substr(0, slashIndex));
  }

  bool SaveDataTo(const string& path, const std::vector<uint8_t>& D)
  {
    if (D.empty() || FileExist(path))
      return false;

    vector<char> path2(2048);
    LPSTR pFile{ nullptr };
    GetFullPathNameA(path.c_str(), path.size(), path2.data(), &pFile);
    CreateDirectoryRecursivelyForFile(path2.data());

    CreateDirectoryRecursivelyForFile(path);

    Assert(!FileExist(path));
    ofstream out(path, ios::binary | ios::out);
    Assert(out.is_open());
    out.write((char*)D.data(), D.size());
    out.close();
    return true;
  }

  bool SaveDataTo(const string& path, const std::vector<uint8_t>& D, bool overwrite)
  {
    if (D.empty())
      return false;
    if (!overwrite && FileExist(path))
      return false;

    vector<char> path2(2048);
    LPSTR pFile{ nullptr };
    GetFullPathNameA(path.c_str(), path.size(), path2.data(), &pFile);
    CreateDirectoryRecursivelyForFile(path2.data());

    CreateDirectoryRecursivelyForFile(path);

    ofstream out(path, ios::binary | ios::out);
    Assert(out.is_open());
    out.write((char*)D.data(), D.size());
    out.close();
    return true;
  }

  std::string MakePath(const std::string& s) { return s; }

  string FormatRecordTime(tm tt)
  {
    stringstream ss;
    ss << (tt.tm_year + 1900);
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << (tt.tm_mon + 1);
    ss << "-";
    ss << std::setfill('0') << std::setw(2) << tt.tm_mday;
    ss << " ";
    ss << std::setfill('0') << std::setw(2) << tt.tm_hour;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << tt.tm_min;
    ss << ":";
    ss << std::setfill('0') << std::setw(2) << tt.tm_sec;
    return ss.str();
  }

  std::string FormatRecordTime(time_t t)
  {
    tm local_tm;
    localtime_s(&local_tm, &t);
    return FormatRecordTime(local_tm);
  }

  tm ParseTimeStringToTm(const std::string& str)
  {
    if (str.size() != 19)
      throw;

    tm tt;
    sscanf_s(str.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
      &tt.tm_year, &tt.tm_mon, &tt.tm_mday, &tt.tm_hour, &tt.tm_min, &tt.tm_sec);
    tt.tm_year -= 1900;
    tt.tm_mon -= 1;
    Assert(tt.tm_year > 0);
    Assert(tt.tm_mon >= 0 && tt.tm_mon <= 11);
    Assert(tt.tm_mday >= 1 && tt.tm_mday <= 31);
    return tt;
  }

  time_t ParseTimeStringToTimeT(const std::string& str)
  {
    tm t = ParseTimeStringToTm(str);
    return mktime(&t);
  }

  tm GetTodayStart(tm tt)
  {
    // 时间戳 0 对应 1970/1/1 8:0:0

    time_t tmp = mktime(&tt);
    // 当天零点
    tmp = tmp - (tmp % 86400) + 16 * 3600;
    tm day_start;
    localtime_s(&day_start, &tmp);
    return day_start;
  }

  tm GetNextDayStart(tm tt)
  {
    // 时间戳 0 对应 1970/1/1 8:0:0

    time_t tmp = mktime(&tt);
    // 当天零点
    tmp = tmp - (tmp % 86400) + 16 * 3600;
    // 第二天零点
    tmp += 86400;
    tm day_start;
    localtime_s(&day_start, &tmp);
    return day_start;
  }

  string CurrentTimeToRecordFormat(tm* pt, time_t* ptt)
  {
    chrono::system_clock::time_point now = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(now);
    tm local_tm;
    localtime_s(&local_tm, &tt);
    if (ptt) *ptt = tt;
    if (pt)  *pt = local_tm;
    return utils::FormatRecordTime(local_tm);
  }
}