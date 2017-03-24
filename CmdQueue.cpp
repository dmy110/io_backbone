#include "CmdQueue.h"
#include <string.h>
#include "log.h"

using namespace Seamless;
RingBuffer::RingBuffer(int buffer_size): _begin(0), _end(1), _buffer_size(buffer_size)
{
    // _data = (char*)mmap(nullptr, buffer_size + sizeof(RingBuffer), 
    //     PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
}
RingBuffer::~RingBuffer() 
{
    // munmap(_data, _buffer_size);
}

size_t RingBuffer::read(char* buffer, size_t len)
{
    //get end first
    size_t end = this->_end;

    size_t begin = this->_begin;

    size_t data_len;
    if (end > begin) {
        size_t foo = end - begin - 1;
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return 0;
        }
        memcpy(buffer, this->_data + begin + 1, data_len);
        this->_begin = begin + data_len;
    } else {
        size_t foo = _buffer_size - (begin - end + 1);
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return 0;
        }

        if (begin + data_len > _buffer_size - 1) {
            //copy twice
            size_t first_data_len = _buffer_size - 1 - begin;
            if (first_data_len != 0) {
                memcpy(buffer, this->_data + begin + 1, first_data_len);
            }

            size_t second_data_len = data_len - first_data_len;
            memcpy(buffer + first_data_len, this->_data, second_data_len);

            this->_begin = second_data_len - 1;
        } else {
            memcpy(buffer, this->_data + begin + 1, data_len);
            this->_begin = begin + data_len;
        }
    }
    begin = this->_begin;
    end = this->_end;
    // printf("read:begin:%u,end%u\n",begin, end);
    return data_len;
}

size_t RingBuffer::write(const char* data, size_t len)
{
    //get begin first
    size_t begin = this->_begin;

    size_t end = this->_end;

    size_t data_len;
    if (begin > end) {
        size_t foo = begin - end - 1;
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return 0;
        }

        memcpy(this->_data + end, data, data_len);
        this->_end = end + data_len ;
    } else {
        size_t foo = _buffer_size - (end - begin + 1);
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return 0;
        }

        if (end + data_len <= _buffer_size - 1) {
            memcpy(this->_data + end, data, data_len);
            this->_end = end + data_len;
        } else {
            //copy twice
            size_t first_data_len = _buffer_size - end;
            memcpy(this->_data + end, data, first_data_len);
            size_t second_data_len = data_len - first_data_len;
            if (second_data_len != 0) {
                memcpy(this->_data, data, second_data_len);
            }

            this->_end = second_data_len;
        }
    }
    begin = this->_begin;
    end = this->_end;
    // printf("write:begin:%u,end%u\n",begin, end);
    return data_len;
}

bool RingBuffer::atomic_read(char* buffer, size_t len)
{
        //get end first
    size_t end = this->_end;

    size_t begin = this->_begin;

    size_t data_len;
    if (end > begin) {
        size_t foo = end - begin - 1;
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return false;
        }
        memcpy(buffer, this->_data + begin + 1, data_len);
        this->_begin = begin + data_len;
    } else {
        size_t foo = _buffer_size - (begin - end + 1);
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return false;
        }

        if (begin + data_len > _buffer_size - 1) {
            //copy twice
            size_t first_data_len = _buffer_size - 1 - begin;
            if (first_data_len != 0) {
                memcpy(buffer, this->_data + begin + 1, first_data_len);
            }

            size_t second_data_len = data_len - first_data_len;
            memcpy(buffer + first_data_len, this->_data, second_data_len);

            this->_begin = second_data_len - 1;
        } else {
            memcpy(buffer, this->_data + begin + 1, data_len);
            this->_begin = begin + data_len;
        }
    }
    begin = this->_begin;
    end = this->_end;
    // printf("read:begin:%u,end%u\n",begin, end);
    return true;
}

bool RingBuffer::atomic_write(const char* data, size_t len)
{
    //get begin first
    size_t begin = this->_begin;

    size_t end = this->_end;

    size_t data_len;
    if (begin > end) {
        size_t foo = begin - end - 1;
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return false;
        }

        memcpy(this->_data + end, data, data_len);
        this->_end = end + data_len ;
    } else {
        size_t foo = _buffer_size - (end - begin + 1);
        if (foo < len) {
            data_len = foo;
        } else {
            data_len = len;
        }

        if (data_len == 0) {
            return false;
        }

        if (end + data_len <= _buffer_size - 1) {
            memcpy(this->_data + end, data, data_len);
            this->_end = end + data_len;
        } else {
            //copy twice
            size_t first_data_len = _buffer_size - end;
            memcpy(this->_data + end, data, first_data_len);
            size_t second_data_len = data_len - first_data_len;
            if (second_data_len != 0) {
                memcpy(this->_data, data, second_data_len);
            }

            this->_end = second_data_len;
        }
    }
    begin = this->_begin;
    end = this->_end;
    // printf("write:begin:%u,end%u\n",begin, end);
    return true;
}

CmdQueue::CmdQueue(uint32_t buffer_size):_buffer_size(buffer_size)
{
    _read_buffer = (char*)malloc(buffer_size);
    _ring_buffer_1 = (RingBuffer*)mmap(nullptr, buffer_size + sizeof(RingBuffer), 
         PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
    _ring_buffer_2 = (RingBuffer*)mmap(nullptr, buffer_size + sizeof(RingBuffer), 
         PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
}

void CmdQueue::init(bool is_child_process, std::function<void(const transform_cmd_t*)> process_cmd_func)
{
    if (is_child_process) {
        _read_queue = _ring_buffer_1;
        _write_queue = _ring_buffer_2;
    } else {
        _read_queue = _ring_buffer_2;
        _write_queue = _ring_buffer_1;
    }
    _process_cmd_func = process_cmd_func;
}

void CmdQueue::put_cmd(transform_cmd_t* cmd)
{
    if (!cmd) return;
    if (cmd->cmd_size > _buffer_size) {
        //如果大于队列最大的大小，打个日志
        log(LOG_LVL_WARNING, "big packet, size:%d", cmd->cmd_size);
        //拆包
        transform_cmd_joint_t::split(cmd, _send_cmd, _buffer_size);
    } else {
        _send_cmd.push_back(cmd);
    }

    while(!_send_cmd.empty()) {
        transform_cmd_t* cmd_to_send = _send_cmd.front();
        if (_write_queue->atomic_write((char*)cmd_to_send, cmd_to_send->cmd_size) == false) {
            //如果无法写入，说明要么包过大，要么对端进程忙不过了，打个日志
            log(LOG_LVL_TRACE, "write_buffer is full,next packet size : %d", cmd->cmd_size);
            break;
        }
        _send_cmd.pop_front();
        free(cmd_to_send);
    }
}

void CmdQueue::process_cmd()
{
    size_t read_size = _read_queue->read(_read_buffer, _buffer_size);
    if (read_size == 0) return;

    size_t cmd_begin_idx = 0;
    while (cmd_begin_idx < read_size) {
        if (read_size - cmd_begin_idx < sizeof(transform_cmd_t)) {
            //不应该出现这种情况
            //丢弃这些数据
            log(LOG_LVL_ERROR, "error data, %d", read_size);
            break;
        }

        transform_cmd_t* cmd = (transform_cmd_t*)(_read_buffer + cmd_begin_idx);
        cmd_begin_idx += cmd->cmd_size;
        if (cmd->cmd_type != TRANSFORM_CMD_JOINT && cmd->cmd_type != TRANSFORM_CMD_JOINT_END) {
            _process_cmd_func(cmd);
            continue;
        } else {
            transform_cmd_joint_t* split_cmd = (transform_cmd_joint_t*)malloc(cmd->cmd_size);
            memcpy(split_cmd, cmd, cmd->cmd_size);
            _read_cmd.push_back(split_cmd);
            if (cmd->cmd_type == TRANSFORM_CMD_JOINT_END) {
                transform_cmd_t* joint_cmd;
                transform_cmd_joint_t::joint(&joint_cmd, _read_cmd);
                _process_cmd_func(joint_cmd);
                free(joint_cmd);
            }
        }
    }

    // if (_read_cmd.size()) {
        //说明需要拼接packet
    // }
}

void transform_cmd_joint_t::split(transform_cmd_t* cmd, std::list<transform_cmd_t*>& cmd_list, size_t max_buffer_size)
{
    size_t cmd_size = cmd->cmd_size;
    size_t split_size = 0;
    size_t _max_buffer_size = max_buffer_size - sizeof(transform_cmd_joint_t);
    while (split_size < cmd_size) {
        size_t total_packet_size = cmd_size < _max_buffer_size ? cmd_size : _max_buffer_size;
        transform_cmd_joint_t* joint_cmd = (transform_cmd_joint_t*)malloc(total_packet_size);
        joint_cmd->cmd_size = total_packet_size;
        memcpy(joint_cmd->data, cmd + split_size, total_packet_size);
        split_size += total_packet_size;
        if (split_size < cmd_size) {        
            joint_cmd->cmd_type = TRANSFORM_CMD_JOINT;
        } else {
            joint_cmd->cmd_type = TRANSFORM_CMD_JOINT_END;
        }
        cmd_list.push_back(joint_cmd);
    }
    free(cmd);
}

void transform_cmd_joint_t::joint(transform_cmd_t** cmd, std::list<transform_cmd_joint_t*>& cmd_list)
{
    size_t total_packet_len = 0;
    for (auto& it : cmd_list) {
        total_packet_len += it->cmd_size - sizeof(transform_cmd_joint_t);
    }
    transform_cmd_t* cmd_ptr = (transform_cmd_t*)malloc(total_packet_len);
    *cmd = cmd_ptr;
    
    size_t append_packet_len = 0;
    for (auto& it : cmd_list) {
        size_t packet_len = it->cmd_size - sizeof(transform_cmd_joint_t);
        memcpy(cmd_ptr + append_packet_len, it->data, packet_len);
        append_packet_len += packet_len;
        free(it);
    }

    cmd_list.clear();
}

transform_cmd_test_t* transform_cmd_test_t::generate_cmd(const char* msg)
{
    size_t cmd_size = sizeof(transform_cmd_test_t) + strlen(msg);
    transform_cmd_test_t* ret = (transform_cmd_test_t*)malloc(cmd_size);
    ret->cmd_type = TRANSFORM_CMD_TEST;
    ret->cmd_size = cmd_size;
    memcpy(ret->test_msg, msg, strlen(msg));
    return ret;
}

#include <unistd.h>
int main()
{
    CmdQueue cq(1024);
    int pid = fork();
    if (pid) {
        cq.init(false, [](const transform_cmd_t* cmd){
            //do nothing
        });
        while (true) {
            auto cmd = transform_cmd_test_t::generate_cmd("呵呵思密达");
            cq.put_cmd(cmd);
            sleep(1);
        }
    } else {
        cq.init(true, [](const transform_cmd_t* cmd){
            if (cmd->cmd_type == TRANSFORM_CMD_TEST) {
                std::string s(((transform_cmd_test_t*)cmd)->test_msg, cmd->cmd_size - sizeof(transform_cmd_test_t));
                log(s.c_str());
            }
        });
        while (true) {
            cq.process_cmd();
            sleep(1);
        }
    }

}