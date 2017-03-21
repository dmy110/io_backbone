#include "log.h"
#include <iostream>
using namespace Seamless;

LogUtils* LogUtils::log_util_handler = nullptr;
void Seamless::log(const char* log_str, ...)
{
    // if (!LogUtils::log_util_handler) {
    //     LogUtils::init();
    // }
    va_list args;
    va_start(args, log_str);
    LogUtils::log(LOG_LVL_DEBUG, log_str, args);
    va_end(args);
}

void Seamless::log(int log_lvl, const char* log_str, ...)
{
    // if (!LogUtils::log_util_handler) {
    //     LogUtils::init();
    // }
    va_list args;
    va_start(args, log_str);
    LogUtils::log(log_lvl, log_str, args);
    va_end(args);
}

void LogUtils::init()
{
    if (log_util_handler != nullptr) return;//已经初始化了
    log_util_handler = new LogUtils();
    log_util_handler->log_max_size = 1024;
    log_util_handler->log_buffer = (char*)malloc(log_util_handler->log_max_size);
    log_util_handler->log_handler = [](int log_lvl, const char* msg){
        std::cout<<msg<<std::endl;//默认实现
    };
}

LogUtils::~LogUtils()
{
    free(log_buffer);
}

void LogUtils::final()
{
    delete log_util_handler;
}

void LogUtils::set_log_handler(std::function<void(int log_lvl, const char* msg)> func)
{
    if (!log_util_handler) {
        LogUtils::init();
    }
    log_util_handler->log_handler = func;
}

void LogUtils::set_log_max_len(uint32_t max_len)
{
    if (!log_util_handler) {
        LogUtils::init();
    }
    log_util_handler->log_max_size = max_len;
    free(log_util_handler->log_buffer);
    log_util_handler->log_buffer = (char*)malloc(max_len);
    if (log_util_handler->log_buffer == nullptr) {
        std::cerr<<"init log buffer error:"<<std::to_string(max_len)<<std::endl;
        log_util_handler->log_max_size = 0;
    }
}

void LogUtils::log(int log_lvl, const char* fmt, va_list& vargs)
{
    if (log_util_handler == nullptr) {
        LogUtils::init();
    }
    std::vsnprintf(log_util_handler->log_buffer, log_util_handler->log_max_size, fmt, vargs);
    log_util_handler->log_handler(log_lvl, log_util_handler->log_buffer);
}
