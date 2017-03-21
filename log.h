#include <cstdarg>
#include <functional>
namespace Seamless
{

class LogUtils
{
// public:
// friend void log(const char* log_str, ...);
// friend void log(int log_lvl, const char* log_str, ...);
public:
    static void init();
    static void set_log_handler(std::function<void(int log_lvl, const char* msg)> func);
    static void set_log_max_len(uint32_t max_len);
    static void final();
    static void log(int log_lvl, const char*, va_list&);
private:
    // static void log(const char*, va_list&);
    std::function<void(int log_lvl, const char* msg)> log_handler;
    uint32_t log_max_size;
    char* log_buffer = nullptr;
private:
    static LogUtils* log_util_handler;
    LogUtils(){};
    ~LogUtils();
    LogUtils(const LogUtils& copy_obj) = delete;
    void operator=(const LogUtils& copy_obj) = delete;
};

enum LOG_LVL
{
    LOG_LVL_DEBUG = 1,
};
//直接调用LogUtils的对应函数，这里只是方便调用
void log(char const* log_str, ...);
void log(int log_lvl, const char* log_str, ...);


};