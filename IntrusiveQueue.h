#pragma once

namespace Intrusive
{

class QueuedObject
{
protected:
	QueuedObject *_next;
	friend class LiFoQueue;
	friend class FiFoQueue;
	template <typename TYPE>
	friend class QueuedObjectPool;
public:
	QueuedObject() : _next(0) {}
	void linkAfter(QueuedObject *object) { object->_next = _next; _next = object; }
	void linkBefore(QueuedObject *object) { object->_next = this; }
	QueuedObject *next() { return _next; }
};

class LiFoQueue
{
protected:
	QueuedObject _front;
public:
	LiFoQueue() { _front._next = &_front; }
	bool empty() { return _front._next == &_front; }
	QueuedObject *end() { return &_front; }
	QueuedObject *front() { return _front._next; }
	QueuedObject *pop_front() { QueuedObject *obj = _front._next; _front._next = _front._next->_next; return obj; }
	void push_front(QueuedObject *obj) { obj->_next = _front._next; _front._next = obj; }
	size_t size() { size_t cnt(0); for (QueuedObject *obj = _front._next; obj != &_front; obj = obj->_next) ++cnt; return cnt; }
};

class FiFoQueue
{
protected:
	struct Queue: public QueuedObject
	{
		QueuedObject *_back;
		Queue() { _next = _back = this; }
	} _queue;
public:
	FiFoQueue() {}
	QueuedObject *back() { return _queue._back; }
	bool empty() { return _queue._next == &_queue; }
	QueuedObject *end() { return &_queue; }
	QueuedObject *front() { return _queue._next; }
	QueuedObject *pop_front() { QueuedObject *obj(_queue._next); if((_queue._next = _queue._next->_next) == &_queue) _queue._back = &_queue; return obj; }
	void push_back(QueuedObject *obj) { obj->_next = &_queue; _queue._back->_next = obj; _queue._back = obj; }
	size_t size() { size_t cnt(0); for (QueuedObject *obj(_queue._next); obj != &_queue; obj = obj->_next) ++cnt; return cnt; }
};

template <typename TYPE>
class QueuedObjectPool
{
private:
	QueuedObject *_next;
	unsigned _blockSize;
public:
	QueuedObjectPool(unsigned blockSize = 256) : _next(0), _blockSize(blockSize) {}
	void setBlockSize(unsigned blockSize) { _blockSize = blockSize; }

	TYPE *allocate()
	{
		TYPE *object;
		if (!(object = static_cast<TYPE*>(_next)))
		{
			TYPE *itr = object = new TYPE[_blockSize];
			for (TYPE *end = itr + _blockSize - 1; itr < end; ++itr)
				static_cast<QueuedObject*>(itr)->_next = itr + 1;
			static_cast<QueuedObject*>(itr)->_next = 0;
		}
		_next = static_cast<QueuedObject*>(object)->_next;
		return object;
	}

	void free(TYPE *object)
	{
		static_cast<QueuedObject*>(object)->_next = _next;
		_next = object;
	}
};

} // namespace Intrusive
