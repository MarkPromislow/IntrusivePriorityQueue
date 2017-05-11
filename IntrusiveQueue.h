#ifndef _INTRUSIVE_QUEUE_H_
#define _INTRUSIVE_QUEUE_H_

namespace Intrusive
{

class QueuedObject
{
protected:
	QueuedObject *_next;

	friend class LiFoQueue;
	friend class FiFoQueue;
	friend class BaseQueuedObjectPool;
	template<typename TYPE>
	friend class QueuedObjectPool;
public:
	QueuedObject() : _next(0) {}
	void linkAfter(QueuedObject *object) { object->_next = _next; _next = object; }
	QueuedObject *next() { return _next; }
};

class LiFoQueue
{
protected:
	QueuedObject *_front;
public:
	LiFoQueue() : _front(0) {}
	void push_front(QueuedObject *object) { object->_next = _front; _front = object; }
	QueuedObject *pop_front() { QueuedObject *object = _front; if (_front) _front = _front->_next; return object; }
	QueuedObject *front() { return _front; }
	size_t size() { size_t cnt(0); for (QueuedObject *object = _front; object; object = object->_next) ++cnt; return cnt; }
};

class FiFoQueue
{
protected:
	QueuedObject _front;
	QueuedObject _back;
public:
	FiFoQueue() { _front._next = &_back; _back._next = &_front; }
	void push_front(QueuedObject *obj) { obj->_next = &_front; _front._next->_next = obj; _front._next = obj; }
	bool empty() { return _front._next == &_back; }
	QueuedObject *front() { return _front._next != &_back ? _front._next: 0; }
	QueuedObject *back() { return _back._next != &_front ? _back._next : 0; }
	QueuedObject *pop_back() { QueuedObject *obj(0); if (_back._next != &_front) { obj = _back._next; if((_back._next = obj->_next) == &_front) _front._next = &_back; } return obj; }
	const QueuedObject *end() { return &_front; }
	size_t size() { size_t cnt(0); for (QueuedObject *obj(_back._next); obj != &_front; obj = obj->_next) ++cnt; return cnt; }
};

/*
** BaseQueuedObjectPool: A type less ObjectPool used in class DataHistory
*/
class BaseQueuedObjectPool
{
protected:
	QueuedObject *_freeQueue;
	size_t _blockSize;

	BaseQueuedObjectPool(size_t blockSize) : _freeQueue(0), _blockSize(blockSize) {}
public:
	void setBlockSize(unsigned blockSize) { _blockSize = blockSize; }

	void freeObject(QueuedObject *object) { object->_next = _freeQueue; _freeQueue = object; }
};

template <typename TYPE>
class QueuedObjectPool: public BaseQueuedObjectPool
{
public:
	QueuedObjectPool(size_t blockSize = 256) : BaseQueuedObjectPool(blockSize) {}

	TYPE *newObject()
	{
		TYPE *object = static_cast<TYPE*>(_freeQueue);
		if (! object)
		{
			object = new TYPE[_blockSize];
			TYPE *itr(object + _blockSize - 1);
			for (itr->_next = 0; itr > object; --itr)
			{
				(itr-1)->_next = itr;
			}
		}
		_freeQueue = object->_next;
		return object;
	}

};

} // namespace Intrusive

#endif
