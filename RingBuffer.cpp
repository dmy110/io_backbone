#include "RingBuffer.hpp"
RingBuffer::RingBuffer(int buffer_size): _begin(0), _end(1)
{
	_data = static_cast<char*>(malloc(buffer_size));
}
RingBuffer::~RingBuffer() 
{
	free(_data);
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
		size_t foo = sizeof(this->_data) - (begin - end + 1);
		if (foo < len) {
			data_len = foo;
		} else {
			data_len = len;
		}

		if (data_len == 0) {
			return 0;
		}

		if (begin + data_len > sizeof(this->_data) - 1) {
			//copy twice
			size_t first_data_len = sizeof(this->_data) - 1 - begin;
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
		size_t foo = sizeof(this->_data) - (end - begin + 1);
		if (foo < len) {
			data_len = foo;
		} else {
			data_len = len;
		}

		if (data_len == 0) {
			return 0;
		}

		if (end + data_len <= sizeof(this->_data) - 1) {
			memcpy(this->_data + end, data, data_len);
			this->_end = end + data_len;
		} else {
			//copy twice
			size_t first_data_len = sizeof(this->_data) - end;
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
		size_t foo = sizeof(this->_data) - (begin - end + 1);
		if (foo < len) {
			data_len = foo;
		} else {
			data_len = len;
		}

		if (data_len == 0) {
			return false;
		}

		if (begin + data_len > sizeof(this->_data) - 1) {
			//copy twice
			size_t first_data_len = sizeof(this->_data) - 1 - begin;
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
		size_t foo = sizeof(this->_data) - (end - begin + 1);
		if (foo < len) {
			data_len = foo;
		} else {
			data_len = len;
		}

		if (data_len == 0) {
			return false;
		}

		if (end + data_len <= sizeof(this->_data) - 1) {
			memcpy(this->_data + end, data, data_len);
			this->_end = end + data_len;
		} else {
			//copy twice
			size_t first_data_len = sizeof(this->_data) - end;
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





// int main()
// {
	
// }
