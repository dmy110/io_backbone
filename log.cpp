#include "log.h"
using namespace Seamless;
void log(const char* log_str, ...)
{
    if (!LogUtils::log_util_handler) {
        LogUtils::init();
    }
    va_list args;
    va_start(args, log_str);
    LogUtils::log(log_str, va_list);
    va_end(args);
}

void log(int log_lvl, const char* log_str, ...)
{
    if (!LogUtils::log_util_handler) {
        LogUtils::init();
    }
    va_list args;
    va_start(args, log_str);
    LogUtils::log(log_lvl, log_str, va_list);
    va_end(args);
}

void LogUtils::init()
{
    if (log_util_handler != nullptr) return;//已经初始化了
    log_util_handler = new LogUtils();
    log_util_handler->log_buffer = (char*)malloc(log_max_size);
    log_util_handler->log_handler = [](const char* msg){
        std::cout<<msg<<std::endl;//默认实现
    }
}

LogUtils::~LogUtils()
{
    free
}
