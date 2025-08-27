#pragma once

/*
** written by Mark Promislow of Green Frog Applications, LLC
*/

#include <string.h>

#include <functional>

namespace Intrusive
{

	template<typename T, typename L>
	class PriorityQueue;

	class HeapObject
	{
	protected:
		size_t _position;

		template<typename T, typename L>
		friend class PriorityQueue;
	public:
		HeapObject() : _position(0) {}
		size_t position() const noexcept { return _position; }
	};

	template<typename T, typename L = std::less<T> >
	class PriorityQueue
	{
	protected:
		HeapObject** _heap;
		size_t _maxSize;
		size_t _size;
		L _less;
	public:
		PriorityQueue(size_t maxSize, L less = L()) :
			_heap(new HeapObject* [maxSize + 1]), _maxSize(maxSize), _size(0), _less(less) {
			*_heap = 0;
		}
		// remove all items from the queue
		void clear() noexcept;
		// remove item from the queue
		void erase(T *item) noexcept;
		T* getPosition(size_t p) const noexcept { return !p || p > _size ? nullptr : static_cast<T*>(_heap[p]); }
		// remove item at the top of the queue
		T* pop() noexcept;
		// add item to the queue
		void push(T* item) noexcept;
		// move item to new position in the queue
		void reprioritize(T* item) noexcept;
		// access item at the top of the queue
		T* top() const noexcept { return static_cast<T*>(_heap[1]); }
		// number of items in the queue
		size_t size() const noexcept { return _size; }

		~PriorityQueue() { clear(); }
	};

	template<typename T, typename L>
	void PriorityQueue<T, L>::clear() noexcept
	{
		for (HeapObject** ptr(_heap + 1), **end(_heap + _size + 1); ptr < end; ++ptr)
			(*ptr)->_position = 0;
		_size = 0;
	}

	template<typename T, typename L>
	void PriorityQueue<T, L>::erase(T *item) noexcept
	{
		if (!item || !item->HeapObject::_position) return;

		// replace current with last
		HeapObject* lastItem = _heap[_size];
		--_size;
		size_t current = item->HeapObject::_position;
		HeapObject** currentPtr = _heap + current;
		*currentPtr = lastItem;
		lastItem->_position = current;
		reprioritize(static_cast<T*>(lastItem));

		item->HeapObject::_position = 0;
	}

	template<typename T, typename L>
	T* PriorityQueue<T, L>::pop() noexcept
	{
		if (!_size) return 0;

		HeapObject** currentPtr = _heap + 1;
		HeapObject* top = *currentPtr;

		HeapObject* lastItem = _heap[_size];

		// start at top
		size_t current(1);
		for (size_t next(current * 2); next < _size + 1; next = (current = next) * 2)
		{
			HeapObject** nextPtr = _heap + next;
			if (next < _size && _less(*static_cast<T*>(*nextPtr), *static_cast<T*>(*(nextPtr + 1))))
			{
				++nextPtr;
				++next;
			}

			if (_less(*static_cast<T*>(lastItem), *static_cast<T*>(*nextPtr)))
			{
				// move
				*currentPtr = *nextPtr;
				(*currentPtr)->_position = current;
				currentPtr = nextPtr;
			}
			else
				break;
		}
		*currentPtr = lastItem;
		lastItem->_position = current;

		--_size;
		top->_position = 0;
		return static_cast<T*>(top);
	}

	template<typename T, typename L>
	void PriorityQueue<T, L>::push(T* item) noexcept
	{
		// check size
		if (++_size > _maxSize)
		{
			HeapObject** newHeap = new HeapObject * [2 * _maxSize + 1];
			memcpy(newHeap, _heap, (_maxSize + 1) * sizeof(HeapObject**));
			delete[] _heap;
			_heap = newHeap;
			_maxSize = 2 * _maxSize;
		}

		// start at bottom
		HeapObject** currentItem = _heap + _size;
		HeapObject** nextItem;
		size_t current(_size);
		for (size_t next(current / 2);
			next && _less(*static_cast<T*>(*(nextItem = _heap + next)), *item);
			next = (current = next) / 2)
		{
			// move down
			*currentItem = *nextItem;
			(*currentItem)->_position = current;
			currentItem = nextItem;
		}
		*currentItem = item;
		item->HeapObject::_position = current;
	}

	template<typename T, typename L>
	void PriorityQueue<T, L>::reprioritize(T* item) noexcept
	{
		size_t current = item->HeapObject::_position;
		size_t next = current / 2;
		HeapObject** currentPtr = _heap + current, ** nextPtr;
		if (next && _less(*static_cast<T*>(*(nextPtr = _heap + next)), *item))
		{
			// move next down
			*currentPtr = *nextPtr;
			(*currentPtr)->_position = current;
			currentPtr = nextPtr;

			for (next = (current = next) / 2;
				next && _less(*static_cast<T*>(*(nextPtr = _heap + next)), *item);
				next = (current = next) / 2)
			{
				// move next down
				*currentPtr = *nextPtr;
				(*currentPtr)->_position = current;
				currentPtr = nextPtr;
			}
		}
		else
		{
			for (size_t next(current * 2), end(_size + 1); next < end; next = (current = next) * 2)
			{
				HeapObject** nextPtr = _heap + next;
				if (next < _size && _less(*static_cast<T*>(*nextPtr), *static_cast<T*>(*(nextPtr + 1))))
				{
					++nextPtr;
					++next;
				}

				if (_less(*static_cast<T*>(item), *static_cast<T*>(*nextPtr)))
				{
					// move
					*currentPtr = *nextPtr;
					(*currentPtr)->_position = current;
					currentPtr = nextPtr;
				}
				else
					break;
			}
		}
		*currentPtr = item;
		item->HeapObject::_position = current;
	}

} // namespace Intrusive

