#ifndef _INTRUSIVE_HASH_TABLE_
#define _INTRUSIVE_HASH_TABLE_

#include <math.h>
#include <vector>

namespace Intrusive
{

template <class Key, class Type>
struct DefaultEqual
{
	bool operator() (const Key &l, const Type &r) const { return l == r; }
};

// FNV Hash Key values
struct DefaultHashFunction
{
	enum FNV :size_t
	{
		FNV_32_PRIME = 0x1000193,  //   16777619
		FNV_32_INIT = 0x811C9DC5,   // 2166136261
		FNV_64_PRIME = 0x100000001B3,    //        1099511628211
		FNV_64_INIT = 0xCBF29CE484222325 // 14695981039346656037
	};

	template<typename Key>
	size_t operator() (const Key& key) const
	{
		size_t value(FNV_64_INIT);
		for (const char *itr = reinterpret_cast<const char *>(&key), *end = reinterpret_cast<const char *>(&key + 1); itr < end; ++itr)
		{
			value = (value ^ *itr) * FNV_64_PRIME;
		}
		return value;
	}

	template<const char*>
	size_t operator() (const char* &key) const
	{
		size_t value(FNV_64_INIT);
		for (char c = *key; c; c = *++key)
			value = (value ^ c) * FNV_64_PRIME;
		return value;
	}

	template<size_t>
	size_t operator() (const size_t &key) const
	{
		size_t value(FNV_64_INIT);
		const char *itr = reinterpret_cast<const char *>(&key);
		return ((((((((((((((((FNV_64_INIT ^ *itr) * FNV_64_PRIME)
			^ *(itr + 1)) * FNV_64_PRIME)
			^ *(itr + 2)) * FNV_64_PRIME)
			^ *(itr + 3)) * FNV_64_PRIME)
			^ *(itr + 4)) * FNV_64_PRIME)
			^ *(itr + 5)) * FNV_64_PRIME)
			^ *(itr + 6)) * FNV_64_PRIME)
			^ *(itr + 7)) * FNV_64_PRIME);
	}
};

class HashTableObject
{
private:
	HashTableObject *_prev;
	HashTableObject *_next;

	template<typename Key, typename Type, typename Equal, typename Hash>
	friend class HashTable;
public:
	HashTableObject(): _prev(this), _next(this) {}
	void removeFromHash() { _prev->_next = _next; _next->_prev = _prev; _prev = _next = this; }
};

template<typename Key, typename Type, typename Equal = DefaultEqual<Key, Type>, typename Hash = DefaultHashFunction>
class HashTable
{
protected:
	size_t _size;
	size_t _buckets;
	size_t _mask;

	struct HashList: public HashTableObject
	{
		void push_front(HashTableObject *o) { o->_prev = this; o->_next = _next; _next->_prev = o; _next = o; }
		size_t size() { size_t s(0); for (HashTableObject *o(_next); o != this; o = o->_next) ++s; return s; }
	};
	HashList *_listArray;

	Equal _equal;
	Hash _hash;
public:
	HashTable(size_t size, Equal &equal = Equal(), Hash &hash = Hash()):
		_size((size_t)log2(size-1)+1), _buckets(1LL << _size), _mask(_buckets - 1), _listArray(new HashList[_buckets]), _equal(equal), _hash(hash)
	{}

	bool insert(const Key &key, Type *item)
	{
		HashList &list = _listArray[(_hash(key) & _mask)];
		for (HashTableObject *o = list._next; o != &list; o = o->_next)
		{
			if (_equal(key, *static_cast<Type*>(o))) return false;
		}
		list.push_front(item);
		return true;
	}

	Type *find(const Key &key)
	{
		HashList &list = _listArray[(_hash(key) & _mask)];
		for (HashTableObject *o = list._next; o != &list; o = o->_next)
		{
			Type *item = static_cast<Type*>(o);
			if (_equal(key, *item)) return item;
		}
		return 0;
	}

	Type *remove(const Key &key)
	{
		HashList &list = _listArray[(_hash(key) & _mask)];
		for (HashTableObject *o = list._next; o != &list; o = o->_next)
		{
			Type *item = static_cast<Type*>(o);
			if (_equal(key, *item))
			{
				o->removeFromHash();
				return item;
			}
		}
		return 0;
	}

	size_t collisions(std::vector<size_t> &collisions);

	~HashTable()
	{
		if (_listArray) delete[] _listArray;
	}
};

template<typename Key, typename Type, typename Equal, typename Hash>
size_t HashTable<Key, Type, Equal, Hash>::collisions(std::vector<size_t> &collisions)
{
	size_t cnt(0);
	collisions.clear();
	for (HashList *listItr = _listArray, *end = _listArray + _buckets; listItr < end; ++listItr)
	{
		size_t size = listItr->size();
		cnt += size;
		if(!(size < collisions.size())) collisions.resize(size+1);
		++collisions[size];
	}
	return cnt ;
}

} // namespace Intrusive

#endif
