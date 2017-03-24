#include <sys/types.h>
#include <stdint.h>
#include <list>
#include <sys/mman.h>
#include <functional>
namespace Seamless
{
enum TRANSFORM_CMD_TYPE
{
    TRANSFORM_CMD_SOCKET = 1,//套接字收发数据协议
    TRANSFORM_CMD_JOINT = 2,//过长协议分片
    TRANSFORM_CMD_JOINT_END = 3,//过长协议分片结束
    TRANSFORM_CMD_TEST = 4,//测试用协议，打印协议内容
    TRANSFORM_CMD_MAX = 255,//第一位用来表示是否为拼接协议
};

struct transform_cmd_t
{
    uint8_t cmd_type = 0;
    uint32_t cmd_size = 0;
};

//TODO(dmy) 不应该使用sock-fd，使用自己生成的唯一标志id
struct transform_cmd_socket_data_t : transform_cmd_t
{
    int sock_fd;
    uint32_t data_len;
    unsigned char data[0];
};

struct transform_cmd_joint_t : transform_cmd_t
{
    unsigned char data[0];
    static void split(transform_cmd_t* cmd, std::list<transform_cmd_t*>& cmd_list, size_t max_buffer_size);
    static void joint(transform_cmd_t** cmd, std::list<transform_cmd_joint_t*>& cmd_list);
};

struct transform_cmd_test_t : transform_cmd_t
{
    char test_msg[0];
    static transform_cmd_test_t* generate_cmd(const char* msg);
};

class RingBuffer
{
public:
    RingBuffer(int buffer_size);
    ~RingBuffer() ;

    bool atomic_read(char* buffer, size_t len);
    bool atomic_write(const char* data, size_t len);
// protected:
    size_t read(char* buffer, size_t len);
    size_t write(const char* data, size_t len);
private:
    volatile size_t _begin;
    volatile size_t _end;
    size_t _buffer_size;
    char _data[0];
};

class CmdQueue
{
public:
    CmdQueue(uint32_t buffer_size);
    void init(bool is_child_process, std::function<void(const transform_cmd_t*)> process_cmd_func);
    // void set_process_func()
    // void init();
    // void final();
    void put_cmd(transform_cmd_t* cmd);
    void process_cmd();
private:
    RingBuffer* _ring_buffer_1;
    RingBuffer* _ring_buffer_2;
    size_t _buffer_size;
private:
    RingBuffer* _read_queue;
    std::function<void(const transform_cmd_t*)> _process_cmd_func;
    char* _read_buffer;
    std::list<transform_cmd_joint_t*> _read_cmd;
private:
    std::list<transform_cmd_t*> _send_cmd;
    RingBuffer* _write_queue;
};

}