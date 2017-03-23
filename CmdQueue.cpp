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

void CmdQueue::init(bool is_child_process, std::function<void(transform_cmd_t*)> process_cmd_func);
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
        //TODO(dmy) tried ,finish it tommorrow        } 
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

    // if (_read_cmd.size()) {
        //说明需要拼接packet
    // }
}
