#include <cstdarg>
namespace Seamless
{

class LogUtils
{
friend void log(const char* log_str, ...);
friend void log(int log_lvl, const char* log_str, ...);
public:
    static void init();
    static void set_log_handler(std::function<void(const char* msg)>);
    static void set_log_max_len(uint32_t max_len);
    static void final();
private:
    static void log(const char*, va_list);
    static void log(int log_lvl, const char*, va_list);
    std::function<void(const char* msg)> log_handler;
    uint32_t log_max_size = 1024;
    char* log_buffer = nullptr;
private:
    static LogUtils* log_util_handler = nullptr;
    LogUtils(){};
    ~LogUtils();
    LogUtils(const LogUtils& copy_obj) = delete;
    void operator=(const LogUtils& copy_obj) = delete;
};

//直接调用LogUtils的对应函数，这里只是方便调用
void log(const char* log_str, ...);
void log(int log_lvl, const char* log_str, ...);


};