
#pragma once

#include <string>

namespace LOG
{
    static constexpr int debug = 0;
    static constexpr int info  = 1;
    static constexpr int warn  = 2;
    static constexpr int error = 3;

    // (有特殊需要时进行)初始化
    // file_name：日志文件文件名
    // max_file_num：日志文件的最大数量
    // max_file_size：单个日志文件的最大存储容量
    void init(const char *file_name = nullptr, int max_file_num = 16, int max_file_size = 10 * 1024 * 1024);

    void WriteLogToFile(int level, const char *prefix, const char *fmt, va_list valst);
    void WriteLogToDebug(int level, const char *prefix, const char *fmt, va_list valst);
    void WriteLogToFile(int level, const char *prefix, const char *fmt, ...);
    void WriteLogToDebug(int level, const char *prefix, const char *fmt, ...);
    void WriteLog(int level, const char *prefix, const char* format, ...);
    std::string GetTimeOfDay();
    const char *GetLevelMark(int level);
    uint32_t GetPid();
    uint32_t GetTid();
    std::string SpliteFileName(const char* p);
    void SetLogLevel(int L);
}

#define FILENAME(x) (LOG::SpliteFileName(x).c_str())

#define WRITE_LOG(level, fmt, ...) do { \
		char msg1[1024] = { 0 }; \
		sprintf_s(msg1, sizeof(msg1), "[%u:%u] [%s] [%s] [%s] [Line:%d] ", \
            LOG::GetPid(), LOG::GetTid(), LOG::GetTimeOfDay().c_str(), LOG::GetLevelMark(level), FILENAME(__FILE__), __LINE__); \
		LOG::WriteLog(level, msg1, fmt, __VA_ARGS__); \
	} while (0)

#define WRITE_LOG_TO_FILE(level, fmt, ...) do { \
		char msg1[1024] = { 0 }; \
		sprintf_s(msg1, sizeof(msg1), "[%u:%u] [%s] [%s] [%s] [Line:%d] ", \
            LOG::GetPid(), LOG::GetTid(), LOG::GetTimeOfDay().c_str(), LOG::GetLevelMark(level), FILENAME(__FILE__), __LINE__); \
		LOG::WriteLogToFile(level, msg1, fmt, __VA_ARGS__); \
	} while (0)

#define WRITE_LOG_TO_DBG_OUT(level, fmt, ...) do { \
		char msg1[1024] = { 0 }; \
		sprintf_s(msg1, sizeof(msg1), "[%u:%u] [%s] [%s] [%s] [Line:%d] ", \
            LOG::GetPid(), LOG::GetTid(), LOG::GetTimeOfDay().c_str(), LOG::GetLevelMark(level), FILENAME(__FILE__), __LINE__); \
		LOG::WriteLogToDebug(level, msg1, fmt, __VA_ARGS__); \
	} while (0)

#define LOG_DEBUG(fmt, ...)	WRITE_LOG(LOG::debug, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...)	WRITE_LOG(LOG::info,  fmt, __VA_ARGS__)
#define LOG_WARN(fmt, ...)	WRITE_LOG(LOG::warn,  fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...)	WRITE_LOG(LOG::error, fmt, __VA_ARGS__)

#define LOG_TO_FILE_IF(cond, level, fmt, ...) \
    if (cond) { WRITE_LOG_TO_FILE(level, fmt, __VA_ARGS__); }

#define LOG_TO_FILE_ASSERT_IF(cond, level, fmt, ...) \
    if (cond) { WRITE_LOG_TO_FILE(level, fmt, __VA_ARGS__); __debugbreak(); }

