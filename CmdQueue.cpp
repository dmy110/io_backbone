#include "CmdQueue.h"
RingBuffer::RingBuffer(int buffer_size): _begin(0), _end(1), _buffer_size(buffer_size)
{
    _data = mmap(nullptr, buffer_size + sizeof(RingBuffer), 
        PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANON, 0, 0);
}
RingBuffer::~RingBuffer() 
{
    munmap(_data, buffer_size);
}

size_t RingBuffer::read(void* buffer, void len)
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

size_t RingBuffer::write(const void* data, size_t len)
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

bool RingBuffer::atomic_read(void* buffer, size_t len)
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

bool RingBuffer::atomic_write(const void* data, size_t len)
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

CmdQueue::CmdQueue(uint32_t buffer_size):_ring_buffer_1(buffer_size),
    _ring_buffer_2(buffer_size), _buffer_size(buffer_size)
{
    _read_buffer = malloc(buffer_size);
}

void CmdQueue::init(bool is_child_process, std::function<void(const transform_cmd_t*)> process_cmd_func);
{
    if (is_child_process) {
        _read_queue = &_ring_buffer_1;
        _write_queue = &_ring_buffer_2;
    } else {
        _read_queue = &_ring_buffer_2;
        _write_queue = &_ring_buffer_1;
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
        
        //原来的包释放掉
        free(cmd);
    } else {
        _send_cmd.push_back(cmd);
    }

    while(_send_cmd.front()) {
        transform_cmd_t* cmd_to_send = _send_cmd.front();
        if (_write_buffer->atomic_write(cmd_to_send, cmd_to_send->cmd_size) == false) {
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
    size_t read_size = _read_queue->read(_read_buffer, buffer_size);
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
        
    }

    // if (_read_cmd.size()) {
        //说明需要拼接packet
    // }
}

void transform_cmd_joint_t::split(const transform_cmd_t* cmd, std::list<transform_cmd_joint_t*>& cmd_list, size_t max_buffer_size)
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
    (transform_cmd_t*)cmd_ptr = malloc(total_packet_len);
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
