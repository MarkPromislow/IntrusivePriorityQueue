#include "PriorityQueue.h"
#include "LinkedList.h"

#include<set>
#include <queue>

#include <chrono>
#include <iostream>

class TestObject : public Intrusive::HeapObject, public Intrusive::LinkedListObject
{
public:
	int _id;
	int _value;
	TestObject(): _id(0), _value(0) {}
	inline bool operator () (const TestObject &lhs, const TestObject &rhs) const { return lhs._value < rhs._value; }
	// for std::priority_queue
	inline bool operator () (const TestObject *lhs, const TestObject *rhs) const { return lhs->_value < rhs->_value; }
};

template<typename T, typename L = less<T> >
class TestPriorityQueue : public Intrusive::PriorityQueue<T, L>
{
public:
	TestPriorityQueue(unsigned maxSize, L &less = L()) : PriorityQueue(maxSize, less) {}

	bool check()
	{
		for (size_t i = 1; i <= _size; ++i)
		{
			Intrusive::HeapObject *currentPtr = _heap[i];
			if (_heap + currentPtr->position() != &_heap[i])
			{
				printf("ERROR: position\n");
			}
			size_t next = i * 2;
			if (next <= _size)
			{
				if (_less(*static_cast<T*>(currentPtr), *static_cast<T*>(*(_heap + next))))
				{
					printf("ERROR: less\n");
				}
				if (++next <= _size)
				{
					if (_less(*static_cast<T*>(currentPtr), *static_cast<T*>(*(_heap + next))))
					{
						printf("ERROR: less 2\n");
					}
				}
			}
		}
		return true;
	}
};

struct SetCompare
{
	bool operator()(const TestObject *lhs, const TestObject *rhs) const
	{
		// greater
		return lhs->_value > rhs->_value || (!(lhs->_value < rhs->_value) && lhs < rhs);
	}
};

#include <list>
template<typename T, typename L = std::less<T>>
class MinList
{
protected:
	std::list<T> _list;
	L _less;
	size_t _maxSize;
	inline void _insert(T &value)
	{
		for (std::list<T>::iterator itr = _list.begin(); itr != _list.end(); ++itr)
		{
			if (_less(value, *itr))
			{
				if (itr == _list.begin()) _list.push_front(value);
				else _list.insert(--itr, value);
				return;
			}
		}
		_list.push_back(value);
	}
public:
	MinList(size_t maxSize, L less = L()) : _less(less), _maxSize(maxSize) {}
	void insert(T &value)
	{
		if (_list.size() < _maxSize)
		{
			_insert(value);
		}
		else if(_less(value, _list.back()))
		{
			_list.pop_back();
			_insert(value);
		}
	}

	inline size_t size() { return _list.size(); }

	T sum()
	{
		T sum(0);
		for (std::list<T>::iterator itr = _list.begin(); itr != _list.end(); ++itr) sum += *itr;
		return sum;
	}
};

#include <stdlib.h>

#define ITEM_CNT 4096//32768

int main(int argc, const char *argv[])
{
	int random[ITEM_CNT];
	TestObject objects[ITEM_CNT];
	for (unsigned i = 0; i < ITEM_CNT; ++i)
	{
		TestObject &object = objects[i];
		object._id = i;
		random[i] = object._value = std::rand() % ITEM_CNT;
	}

	std::cout << ",Loop|Insert,,,|Reprioritize,,,|Pop,,,|Erase\nn,Duration";
	for (int i = 0; i < 4; ++i)
	{
		std::cout << "|IntrusiveQueue,StdQueue,StdSet,SortedList";
	}
	std::cout << std::endl;

	std::chrono::hours hour(1);
	for (size_t n = 7; n < ITEM_CNT + 1; n += 8)
	{
		MinList<std::chrono::duration<long long, std::nano>> minLoop(5), minIntrusiveInsert(5), minStdInsert(5), minListInsert(5);
		std::chrono::duration<long long, std::nano> minLoopDuration(hour);
		std::chrono::duration<long long, std::nano> minInsertIntrusiveQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minInsertStdQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minStdSetInsertDuration(hour);
		std::chrono::duration<long long, std::nano> minInsertListDuration(hour);
		std::chrono::duration<long long, std::nano> minReprioritizeIntrusiveQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minStdSetReprioritizeDuration(hour);
		std::chrono::duration<long long, std::nano> minReprioritizeListDuration(hour);
		std::chrono::duration<long long, std::nano> minPopIntrusiveQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minPopStdQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minStdSetPopDuration(hour);
		std::chrono::duration<long long, std::nano> minPopListDuration(hour);
		std::chrono::duration<long long, std::nano> minEraseIntrusiveQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minEraseStdQueueDuration(hour);
		std::chrono::duration<long long, std::nano> minStdSetEraseDuration(hour);
		std::chrono::duration<long long, std::nano> minUnlinkListDuration(hour);
		for (size_t t(0); t < 40; ++t) {
			TestPriorityQueue<TestObject, TestObject> intrusivePriorityQueue(ITEM_CNT, TestObject());
			Intrusive::SortedList<TestObject, TestObject> sortedList;
			std::priority_queue<TestObject*, std::vector<TestObject*>, TestObject> stdPriorityQueue;
			std::set<TestObject*, SetCompare> stdSet;

			auto start = std::chrono::steady_clock::now();
			std::chrono::duration<long long, std::nano> duration;
			unsigned j(0);
			for (unsigned i = 0; i < n; ++i)
			{
				++j;
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minLoopDuration > duration) minLoopDuration = duration;
			minLoop.insert(duration);

			// Insert
			// insert intrusive priority queue
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				intrusivePriorityQueue.push(&objects[i]);
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minInsertIntrusiveQueueDuration > duration) minInsertIntrusiveQueueDuration = duration;
			minIntrusiveInsert.insert(duration);
			intrusivePriorityQueue.check();

			// insert std::priority_queue
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				stdPriorityQueue.push(&objects[i]);
			}
			std::chrono::duration<long long, std::nano> insertStdQueueDuration = std::chrono::steady_clock::now() - start;
			if (minInsertStdQueueDuration > insertStdQueueDuration) minInsertStdQueueDuration = insertStdQueueDuration;
			minStdInsert.insert(insertStdQueueDuration);

			// insert std::set
			std::set<TestObject*>::iterator stdSetIterators[ITEM_CNT];
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				stdSetIterators[i] = stdSet.insert(&objects[i]).first;
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minStdSetInsertDuration > duration) minStdSetInsertDuration = duration;

			// insert sorted list
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				sortedList.insert(&objects[i]);
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minInsertListDuration > duration) minInsertListDuration = duration;
			minListInsert.insert(duration);
			if (!sortedList.check()) printf("insert: failed\n");

			// reprioritize
			// reprioritize intrusive queue
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				TestObject &obj = objects[i];
				obj._value = random[n - i];
				intrusivePriorityQueue.reprioritize(&obj);
			}
			std::chrono::duration<long long, std::nano> reprioritizeIntrusiveQueueDuration = std::chrono::steady_clock::now() - start;
			if (minReprioritizeIntrusiveQueueDuration > reprioritizeIntrusiveQueueDuration) minReprioritizeIntrusiveQueueDuration = reprioritizeIntrusiveQueueDuration;
			intrusivePriorityQueue.check();
			for (unsigned i = 0; i < n; ++i)
			{
				TestObject &obj = objects[i];
				obj._value = random[i];
				intrusivePriorityQueue.reprioritize(&obj);
			}
			intrusivePriorityQueue.check();

			// reprioritize std::set
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				TestObject &obj = objects[i];
				stdSet.erase(stdSetIterators[i]);
				obj._value = random[n - i];
				stdSetIterators[i] = stdSet.insert(&obj).first;
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minStdSetReprioritizeDuration > duration) minStdSetReprioritizeDuration = duration;
			for (unsigned i = 0; i < n; ++i)
			{
				TestObject &obj = objects[i];
				stdSet.erase(stdSetIterators[i]);
				obj._value = random[i];
				stdSetIterators[i] = stdSet.insert(&obj).first;
			}

			// reprioritize intrusive list
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				TestObject &obj = objects[i];
				obj._value = random[n - i];
				sortedList.adjust(&obj);
			}
			std::chrono::duration<long long, std::nano> reprioritizeListDuration = std::chrono::steady_clock::now() - start;
			if (minReprioritizeListDuration > reprioritizeListDuration) minReprioritizeListDuration = reprioritizeListDuration;
			if (!sortedList.check()) printf("reprioritize: failed\n");
			for (unsigned i = 0; i < n; ++i)
			{
				TestObject &obj = objects[i];
				obj._value = random[i];
				sortedList.adjust(&obj);
			}

			// pop
			// pop intrusive priority queue
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				intrusivePriorityQueue.pop();
			}
			std::chrono::duration<long long, std::nano> popIntrusiveQueueDuration = std::chrono::steady_clock::now() - start;
			if (minPopIntrusiveQueueDuration > popIntrusiveQueueDuration) minPopIntrusiveQueueDuration = popIntrusiveQueueDuration;

			// pop std::priority_queue
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				stdPriorityQueue.top();
				stdPriorityQueue.pop();
			}
			std::chrono::duration<long long, std::nano> popStdQueueDuration = std::chrono::steady_clock::now() - start;
			if (minPopStdQueueDuration > popStdQueueDuration) minPopStdQueueDuration = popStdQueueDuration;

			// pop std::set
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				std::set<TestObject*>::iterator itr = stdSet.begin();
				stdSet.erase(itr);
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minStdSetPopDuration > duration) minStdSetPopDuration = duration;

			// pop sorted list
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				sortedList.pop_back();
			}
			std::chrono::duration<long long, std::nano> popListDuration = std::chrono::steady_clock::now() - start;
			if (minPopListDuration > popListDuration)minPopListDuration = popListDuration;
			if (!sortedList.check()) printf("Pop: failed\n");

			// erase
			// -- repopulate
			for (unsigned i = 0; i < n; ++i)
			{
				intrusivePriorityQueue.push(&objects[i]);
				stdSetIterators[i] = stdSet.insert(&objects[i]).first;
				sortedList.insert(&objects[i]);
			}

			// erase intrusive priority queue
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				intrusivePriorityQueue.erase(&objects[i]);
			}
			std::chrono::duration<long long, std::nano> eraseIntrusiveQueueDuration = std::chrono::steady_clock::now() - start;
			if (minEraseIntrusiveQueueDuration > eraseIntrusiveQueueDuration)minEraseIntrusiveQueueDuration = eraseIntrusiveQueueDuration;

			// erase std::set
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				stdSet.erase(stdSetIterators[i]);
				//stdSet.erase(&objects[i]);
			}
			duration = std::chrono::steady_clock::now() - start;
			if (minStdSetEraseDuration > duration) minStdSetEraseDuration = duration;

			// erase sorted list
			start = std::chrono::steady_clock::now();
			for (unsigned i = 0; i < n; ++i)
			{
				objects[i].unlink();
			}
			std::chrono::duration<long long, std::nano> unlinkListDuration = std::chrono::steady_clock::now() - start;
			if (minUnlinkListDuration > unlinkListDuration)minUnlinkListDuration = unlinkListDuration;
		}
		std::cout << n << ',' << (minLoop.size() ? minLoop.sum().count() / minLoop.size(): 0)
			//<< ',' << (minIntrusiveInsert.size() ? minIntrusiveInsert.sum().count() / minIntrusiveInsert.size() : 0)
			//<< ',' << (minStdInsert.size() ? minStdInsert.sum().count() / minStdInsert.size() : 0)
			//<< ',' << (minListInsert.size() ? minListInsert.sum().count() / minListInsert.size() : 0)
			<< '|' << minInsertIntrusiveQueueDuration.count() << ',' << minInsertStdQueueDuration.count() << ',' << minStdSetInsertDuration.count() << ',' << minInsertListDuration.count()
			<< '|' << minReprioritizeIntrusiveQueueDuration.count() << ",," << minStdSetReprioritizeDuration.count() << ',' << minReprioritizeListDuration.count()
			<< '|' << minPopIntrusiveQueueDuration.count() << ',' << minPopStdQueueDuration.count() << ',' << minStdSetPopDuration.count() << ',' << minPopListDuration.count()
			<< '|' << minEraseIntrusiveQueueDuration.count() << ",," << minStdSetEraseDuration.count() << ',' << minUnlinkListDuration.count()
			<< std::endl;
	}

	TestPriorityQueue<TestObject, TestObject> intrusivePriorityQueue(ITEM_CNT, TestObject());
	Intrusive::SortedList<TestObject, TestObject> sortedList;
	std::priority_queue<TestObject*, std::vector<TestObject*>, TestObject> stdPriorityQueue;
	std::set<TestObject*, SetCompare> stdSet;
	for (size_t i = 0; i < ITEM_CNT; ++i)
	{
		intrusivePriorityQueue.push(&objects[i]);
		stdPriorityQueue.push(&objects[i]);
		stdSet.insert(&objects[i]);
		sortedList.insert(&objects[i]);
	}
	for (unsigned i = 0; i < ITEM_CNT; ++i)
	{
		TestObject *a = intrusivePriorityQueue.pop();
		TestObject *b = stdPriorityQueue.top();
		stdPriorityQueue.pop();
		std::set<TestObject*>::iterator itr = stdSet.begin();
		TestObject *c = *itr;
		stdSet.erase(itr);
		TestObject *d = sortedList.pop_back();
		if (a->_value != b->_value || a->_value != c->_value || a->_value != d->_value)
			printf("ERROR: %d %d %d %d\n", a->_value, b->_value, c->_value, d->_value);
	}

	return 0;
}