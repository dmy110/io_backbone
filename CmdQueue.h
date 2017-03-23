struct transform_cmd_t;

namespace Seamless
{
enum TRANSFORM_CMD_TYPE
{
    TRANSFORM_CMD_SOCKET = 1,
    TRANSFORM_CMD_MAX = 127,//第一位用来表示是否为拼接协议
}

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
    unsigned char* data[0];
};

class RingBuffer
{
public:
    RingBuffer(int buffer_size);
    ~RingBuffer() ;

    bool atomic_read(void* buffer, size_t len);
    bool atomic_write(const void* data, size_t len);
protected:
    size_t read(void* buffer, size_t len);
    size_t write(const void* data, size_t len);
private:
    volatile size_t _begin;
    volatile size_t _end;
    void* _data;
    size_t _buffer_size;
};

class CmdQueue
{
public:
    CmdQueue(uint32_t buffer_size);
    void init(bool is_child_process, std::function<void(transform_cmd_t*)> process_cmd_func);
    // void set_process_func()
    // void init();
    // void final();
    void put_cmd(transform_cmd_t* cmd);
    void process_cmd();
private:
    RingBuffer _ring_buffer_1;
    RingBuffer _ring_buffer_2;
    size_t _buffer_size;
private:
    RingBuffer* _read_queue;
    std::function<void(transform_cmd_t*)> _process_cmd_func;
    void* _read_buffer;
    std::list<transform_cmd_t*> _read_cmd;
private:
    std::list<transform_cmd_t*> _send_cmd;
    RingBuffer* _write_queue;
};

}