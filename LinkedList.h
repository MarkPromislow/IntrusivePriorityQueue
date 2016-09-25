#ifndef _INTRUSIVE_LINKED_LIST_H_
#define _INTRUSIVE_LINKED_LIST_H_

/*
** written by Mark Promislow of Green Frog Applications, LLC
*/

namespace Intrusive
{

class LinkedListObject
{
protected:
	LinkedListObject *_prev;
	LinkedListObject *_next;
	friend class IntrusiveLinkedList;
public:
	LinkedListObject() : _prev(0), _next(0) {}
	inline void link(LinkedListObject *o) { o->_prev = this; o->_next = _next; _next->_prev = o; _next = o; }
	inline void linkBefore(LinkedListObject *o) { o->_prev = _prev; o->_next = this; _prev->_next = o; _prev = o; }
	inline void unlink() { _next->_prev = _prev; _prev->_next = _next; }
	inline void erase() { if (_prev) { _next->_prev = _prev; _prev->_next = _next; _prev = 0; _next = 0; } }
	inline LinkedListObject *next() { return _next; }
	inline LinkedListObject *prev() { return _prev; }
};

class IntrusiveLinkedList: public LinkedListObject
{
protected:
	LinkedListObject _list;
public:
	IntrusiveLinkedList() { _list._next = _list._prev = &_list; }
	inline bool empty() { return _list._next == &_list; }
	inline void clear() { _list._next = _list._prev = &_list; }
	inline void push_back(LinkedListObject *o) { o->_prev = _list._prev; o->_next = &_list; _list._prev->_next = o; _list._prev = o; }
	inline void push_front(LinkedListObject *o) { o->_prev = &_list; o->_next = _list._next; _list._next->_prev = o; _list._next = o; }
	inline LinkedListObject *begin() { return _list._next; }
	inline LinkedListObject *rbegin() { return _list._prev; }
	inline LinkedListObject *end() { return &_list; }
	inline LinkedListObject *pop_front() { LinkedListObject *n(0); if (_list._next != &_list) {n = _list._next; n->erase(); } return n; }
	inline LinkedListObject *pop_back() { LinkedListObject *p(0);if (_list._prev != &_list)	{p = _list._prev; p->erase(); } return p; }
	size_t size() { size_t s = 0; for (LinkedListObject *o = _list._next; o != &_list; o = o->_next) ++s; return s; }
};

#include <functional>
template<typename T, typename L = std::less<T>>
class SortedList
{
protected:
	IntrusiveLinkedList _list;
	L _less;
public:
	SortedList(L &less = L()): _less(less) {}
	inline void adjust(T *obj);
	inline void insert(T *obj);
	inline bool empty() { return _list.empty(); }
	inline void clear() { _list.clear(); }
	inline T* begin() { return static_cast<T*>(_list.begin()); }
	inline T* rbegin() { return static_cast<T*>(_list.rbegin()); }
	inline T* end() { return static_cast<T*>(_list.end()); }
	inline T* pop_front() { return static_cast<T*>(_list.pop_front()); }
	inline T* pop_back() { return static_cast<T*>(_list.pop_back()); }
	size_t size() { return _list.size(); }
	bool check();
};

template<typename T, typename L>
void SortedList<T, L>::adjust(T *obj)
{
	LinkedListObject *prev = obj->prev(), *next = obj->next();
	if (prev != _list.end() && _less(*obj, *static_cast<T*>(prev)))
	{
		obj->unlink();
		for (prev = prev->prev(); prev != _list.end(); prev = prev->prev())
		{
			if (_less(*static_cast<T*>(prev), *obj)) break;
		}
		prev->link(obj);
	}
	else if(next != _list.end() && _less(*static_cast<T*>(next), *obj))
	{
		obj->unlink();
		for (next = next->next(); next != _list.end(); next = next->next())
		{
			if (_less(*obj, *static_cast<T*>(next))) break;
		}
		next->linkBefore(obj);
	}
}

template<typename T, typename L>
void SortedList<T, L>::insert(T *obj)
{
	LinkedListObject *next = _list.begin();
	for (; next != _list.end(); next = next->next())
	{
		if (_less(*obj, *static_cast<T*>(next))) break;
	}
	next->linkBefore(obj);
}

template<typename T, typename L>
bool SortedList<T, L>::check()
{
	LinkedListObject *next = _list.begin();
	if (next != _list.end())
	{
		for (next = next->next(); next != _list.end(); next = next->next())
		{
			if (_less(*static_cast<T*>(next), *static_cast<T*>(next->prev()))) return false;
		}
	}
	return true;
}

template<typename T, typename L = std::less<T>>
class MinimumSortedList : public IntrusiveLinkedList
{
protected:
	size_t _maxSize;
	size_t _size;
public:
	MinimumSortedList(size_t maxSize = 0, L &less = L()) : SortedList(less), _maxSize(maxSize), _size(0) {}
	void setMaxSize(size_t maxSize) { _maxSize = maxSize; }
	inline T *insert(T *obj);
	inline void clear() { _size = 0; _list.clear(); }
	inline T* pop_front() { if (!_list.empty()) --size;  return static_cast<T*>(_list.pop_front()); }
	inline T* pop_back() { if (!_list.empty()) --size; return static_cast<T*>(_list.pop_back()); }
	size_t size() { return _size; }
};

template<typename T, typename L>
T *MinimumSortedList<T, L>::insert(T *obj)
{
	T *removed(0);
	if (_size < _maxSize)
	{
		++_size;
		_list.insert(obj);
	}
	else if (_less(*obj, _list.back()))
	{
		removed = _list.pop_back();
		_list.insert(obj);
	}
	return removed;
}

} // namespace Intrusive

#endif
