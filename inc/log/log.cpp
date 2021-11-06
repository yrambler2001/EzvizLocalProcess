
#include <iostream>
#include <fstream>
#include <vector>

#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>

using namespace std;

#include "Log.h"
#include <windows.h>

namespace
{
    int gLevel = LOG::debug;
}


class Log
{
public:
    static Log *instance()
    {
        static Log inst;
        return &inst;
    }

    void init(const char *file_name, int max_file_num, int max_file_size)
    {
        if (file_name)
            m_file_name = file_name;

        m_max_file_num = max_file_num;
        m_max_file_size = max_file_size;

    }

    int get_file_size(const string &file_path)
    {
        ifstream file(file_path.data());
        if (!file.is_open())
        {
            return 0;
        }
        file.seekg(0, ios::end);
        int size = (int)file.tellg();
        file.close();

        return size;
    }

    string make_file_path(int index)
    {
        char path_str[256] = { 0 };
        snprintf(path_str, 256, "%s%s.%d.log", m_file_dir.data(), m_file_name.data(), index);
        return path_str;
    }

    string next_file_path()
    {
        string path = make_file_path(m_file_index);
        m_file_index++;
        if (m_file_index == m_max_file_num)
        {
            m_file_index = 0;
        }
        return path;
    }

    void write_log2(int level, const char *log_str)
    {
        string file_path;
        std::ios_base::openmode open_mode = ios::app;
        if (m_file_paths.empty())
        {
            file_path = next_file_path();
            m_file_paths.push_back(file_path);
        }
        else
        {
            file_path = m_file_paths.back();

            int size = get_file_size(file_path);
            if (size > m_max_file_size)
            {
                if (m_file_paths.size() >= (size_t)m_max_file_num)
                {
                    auto first = m_file_paths.begin();
                    if (DeleteFileA(first->c_str()))
                    {
                        m_file_paths.erase(first);
                    }
                }

                file_path = next_file_path();
                open_mode = ios::trunc;
                m_file_paths.push_back(file_path);
            }
        }

        ofstream file(file_path.data(), open_mode);
        if (!file.is_open())
        {
            return;
        }

        file << log_str << endl;

        file.close();
    }

private:
    Log()
    {
        m_max_file_num = 16;
        m_max_file_size = 10 * 1024 * 1024;
        m_file_index = 0;

        char pszModuleName[512];
        memset(pszModuleName, 0, 512);
        HMODULE hModule = NULL;
        GetModuleHandleExA(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCSTR)&Log::instance, &hModule);
        if (hModule)
            GetModuleFileNameA(hModule, pszModuleName, sizeof(pszModuleName));

        std::string strFolder = pszModuleName;
        int nPos = strFolder.find_last_of('\\');
        m_file_name = strFolder.substr(nPos + 1);
        strFolder.erase(strFolder.begin() + nPos, strFolder.end());
        m_file_dir = strFolder + "\\log\\";

        CreateDirectoryA(m_file_dir.data(), NULL);


        for (int index = 0; index<m_max_file_num; index++)
        {
            string file_path = make_file_path(index);
            int size = get_file_size(file_path);
            if (size<m_max_file_size)
            {
                m_file_index = index;
                break;
            }
        }
    }

    virtual ~Log()
    {

    }


    int m_max_file_num;
    int m_max_file_size;
    string m_file_name;
    string m_file_dir;
    int m_file_index;
    vector<string> m_file_paths;
};


void LOG::init(const char *file_name/*=nullptr*/, int max_file_num/*=16*/, int max_file_size/*=10240*/)
{
    Log::instance()->init(file_name, max_file_num, max_file_size);
}


void LOG::WriteLogToFile(int level, const char *prefix, const char *fmt, ...)
{
    va_list valst;
    va_start(valst, fmt);
    WriteLogToFile(level, prefix, fmt, valst);
    va_end(valst);
}

void LOG::WriteLogToFile(int level, const char *prefix, const char *fmt, va_list valst)
{
    if (level < gLevel)
      return;

    char buf[4096] = { 0 };
    vsnprintf(buf, 4096, fmt, valst);

    std::string msg(prefix);
    msg += buf;

    Log::instance()->write_log2(level, msg.c_str());
}

void LOG::WriteLogToDebug(int level, const char *prefix, const char *fmt, ...)
{
    va_list valst;
    va_start(valst, fmt);
    WriteLogToDebug(level, prefix, fmt, valst);
    va_end(valst);
}

void LOG::WriteLogToDebug(int level, const char *prefix, const char *fmt, va_list valst)
{
    char buf[2048] = { 0 };
    vsnprintf(buf, 2048, fmt, valst);

    std::string msg(prefix);
    msg += buf;

    strcat_s(buf, sizeof(buf), "\n");
    OutputDebugStringA(buf);
}


#define LOG_TYPE_FILE
//#define LOG_TYPE_MEMORY

void LOG::WriteLog(int level, const char *prefix, const char *fmt, ...)
{
#if !defined LOG_TYPE_FILE
#if !defined LOG_TYPE_MEMORY
    return;
#endif
#endif
    
    va_list valst;
    va_start(valst, fmt);

#ifdef LOG_TYPE_FILE
    WriteLogToFile(level, prefix, fmt, valst);
#elif defined LOG_TYPE_MEMORY
    WriteLogToDebug(level, prefix, fmt, valst);
#endif

    va_end(valst);
}

std::string LOG::GetTimeOfDay()
{
    std::ostringstream oss;
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    tm fmt;
    localtime_s(&fmt, &t);
    oss << std::put_time(&fmt, "%Y/%m/%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

const char * LOG::GetLevelMark(int level)
{
    switch (level)
    {
    case 0: return "D"; // debug
    case 2: return "W"; // warning
    case 3: return "E"; // error

    case 1:
    default:
        return "I";     // info
    }
}

uint32_t LOG::GetPid()
{
    return GetCurrentProcessId();
}

uint32_t LOG::GetTid()
{
    return GetCurrentThreadId();
}

std::string LOG::SpliteFileName(const char * p)
{
    std::string s(p);
    return s.substr(s.rfind('\\') + 1);
}

void LOG::SetLogLevel(int L)
{
    gLevel = (L < LOG::debug ? LOG::debug : (L > LOG::error ? LOG::error : L));  
}
