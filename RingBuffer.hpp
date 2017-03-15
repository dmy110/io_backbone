#include <pthread.h>
#include <atomic>
#include <queue>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>

class RingBuffer
{
public:
	RingBuffer(int buffer_size);
	~RingBuffer() ;

	bool atomic_read(char* buffer, size_t len);
	bool atomic_write(const char* data, size_t len);
protected:
	size_t read(char* buffer, size_t len);
	size_t write(const char* data, size_t len);
private:
	std::atomic<size_t> _begin;
	std::atomic<size_t> _end;
	char* _data;
};

template <typename _Tp>
class ConcurrenyList
{
public:
	ConcurrenyList()
	{
		_wrb = new RingBuffer(1024 * sizeof(_Tp));
		_rb_queue.push(_wrb);
		_rrb = _wrb;
		_rb_size = 1024;

		pthread_spin_init(&_lock, 0);
	}
	~ConcurrenyList()
	{
		for (int i = 0; i < _rb_queue; ++i) {
			delete _rb_queue.front();
		}
		pthread_spin_destroy(&_lock);
	}
public:
	void push(const _Tp& data)
	{
	PUSH:
		if (_wrb->atomic_write(&data, sizeof(_Tp)) == false) {
			_rb_size *= 2;
			_wrb = new RingBuffer(_rb_size * (sizeof(_Tp)));
			pthread_spin_lock(&_lock);
			_rb_queue.push(_wrb);
			pthread_spin_unlock(&_lock);
			goto PUSH;
		}
	}

	bool pull(_Tp* buffer)
	{
	PULL:
		if (_rrb->atomic_read(buffer, sizeof(_Tp)) == false) {
			pthread_spin_lock(&_lock);
			if (_rb_queue.size() > 1) {
				_rb_queue.pop();
				delete _rrb;
				_rrb = _rb_queue.front();
				pthread_spin_unlock(&_lock);
				goto PULL;
			} else {
				pthread_spin_unlock(&_lock);
				return false;
			}
		} else {
			return true;
		}
	}
private:
	RingBuffer* _wrb;
	RingBuffer* _rrb;
	std::queue<_Tp*> _rb_queue;
	int _rb_size;

	pthread_spinlock_t _lock;
};

template <typename _Tp>
class BlockingQueue
{
public:
	BlockingQueue()
	{
		_cl = new ConcurrenyList<_Tp>();
		sem_init(&_sem, 0, 0);
	}
	~BlockingQueue()
	{
		delete _cl;
		sem_destroy(&_sem);
	}
public:
	void push(const _Tp& data)
	{
		_cl->push(data);
		sem_post(&_sem);
	}

	bool pull(_Tp* buffer)
	{
		sem_wait(&_sem);
		return _cl->pull(buffer);
	}
private:
	ConcurrenyList<_Tp>* _cl;
	sem_t _sem;
};