#pragma once

#include "IntrusiveLinkedList.h"
#include "IntrusiveQueue.h"

#include <stdint.h>

/*
** DataBucket
** - accumulates data
*/
class DataBucket
{
protected:
	int64_t _duration;
	int64_t _count;
public:
	DataBucket() : _duration(0), _count(0) {}

	virtual void addData(const void *data) { ++_count; }
	virtual void subData(const void *data) { --_count; }
	virtual void addTime(int64_t duration, const void *data) { _duration += duration; }
	virtual void subTime(int64_t duration, const void *data) { _duration -= duration; }
	virtual DataBucket& operator += (const DataBucket &dataBucket) { _duration += dataBucket._duration; _count += dataBucket._count; return *this; }
	virtual DataBucket& operator -= (const DataBucket &dataBucket) { _duration -= dataBucket._duration; _count -= dataBucket._count; return *this; }
	int64_t duration() const { return _duration; }
	int64_t count() const { return _count; }

	virtual void reset() { _duration = _count = 0; }

	virtual ~DataBucket() {}
};

/*
** Aggregator
** - accumulates data for a time range
*/
class Aggregator : public Intrusive::LinkedListObject
{
protected:
	uint64_t _beginOffset;
	uint64_t _endOffset;
	DataBucket *_dataBucket;
public:
	Aggregator() : _beginOffset(0), _endOffset(0), _dataBucket(0) {}

	int initialize(uint64_t beginOffset, uint64_t endOffset, DataBucket *dataBucket);
	uint64_t beginOffset() const { return _beginOffset; }
	uint64_t endOffset() const { return _endOffset; }
	const DataBucket* dataBucket() const { return _dataBucket; }

	void addData(const void *data) { _dataBucket->addData(data); }
	void subData(const void * data) { _dataBucket->subData(data); }
	void addTime(uint64_t duration, const void *data) { _dataBucket->addTime(duration, data); }
	void subTime(uint64_t duration, const void *data) { _dataBucket->subTime(duration, data); }

	void reset() { _dataBucket->reset(); }
};

class BucketAggregator : public Aggregator
{
public:
	void addBucket(const DataBucket &dataBucket) { *_dataBucket += dataBucket; };
	void subBucket(const DataBucket &dataBucket) { *_dataBucket -= dataBucket; }
};

/*
** BucketHistory
** - uses a vector of DataBuckets to aggregate data
** - aggregators sum data over time ranges
** - good for summing over a long period of time
*/
template <typename BucketType>
struct BucketGenerator
{
	DataBucket *generate(size_t bucketCnt) { return new BucketType[bucketCnt]; }
	size_t bucketSize() { return sizeof(BucketType); }
};

class BucketHistory
{
protected:
	unsigned _bucketCnt;
	unsigned _bucketDuration;
	uint64_t _beginTime;
	uint64_t _lastTime;

	unsigned _previousBucketInt;
	uint64_t _previousTime;

	DataBucket *_previousBucket;
	DataBucket *_dataBuckets;
	size_t _bucketSize;

	const void *_previousData;

	Intrusive::LinkedList _aggregators;
public:
	BucketHistory();

	template <typename BucketType>
	int initialize(unsigned bucketDuration, uint64_t beginTime, uint64_t endTime, BucketGenerator<BucketType> &bucketGenerator);

	void addAggregator(BucketAggregator *aggregator);

	unsigned bucketCount() const { return _bucketCnt; }
	unsigned bucketDuration() const { return _bucketDuration; }
	uint64_t beginTime() const { return _beginTime; }
	uint64_t endTime() const { return _lastTime + 1; }
	unsigned bucketOffsetForTime(uint64_t timeUnit) const;
	DataBucket& bucketForOffset(unsigned offset);

	int addData(uint64_t currentTime, const void *data);
	int addTime(uint64_t currentTime);
	void stop(uint64_t currentTime);

	void reset();

	~BucketHistory();
};

template<typename BucketType>
inline int BucketHistory::initialize(unsigned bucketDuration, uint64_t beginTime, uint64_t endTime, BucketGenerator<BucketType>& bucketGenerator)
{
	_bucketCnt = (endTime - beginTime - 1) / bucketDuration + 1;
	_bucketDuration = bucketDuration;
	_beginTime = beginTime;
	_endTime = endTime;

	return 0;
}

inline
unsigned BucketHistory::bucketOffsetForTime(uint64_t timeUnit) const
{
	unsigned offset(0);
	if (timeUnit > _beginTime)
	{
		if (timeUnit > _lastTime) offset = _bucketCnt - 1;
		else offset = static_cast<unsigned>((timeUnit = _beginTime) / _bucketDuration);
	}
	return offset;
}

inline
DataBucket& BucketHistory::bucketForOffset(unsigned offset)
{
	return *(DataBucket*)(reinterpret_cast<char*>(_dataBuckets) + offset * _bucketSize);
}

/*
** TimedDataHistory
** - a queue of timed data
** - short term histories
*/
class TimedData : public Intrusive::QueuedObject
{
protected:
	uint64_t _time;
public:
	TimedData(uint64_t time = 0) : _time(time) {}
	uint64_t time() { return _time; }
	void reset() { _time = 0; }
	virtual ~TimedData() {}
};

class TimedDataAggregator : public Aggregator
{
protected:
	Intrusive::FiFoQueue *_dataQueue;
	TimedData *_newest;
	TimedData *_oldest;
	uint64_t _lastUpdateTime;
	uint64_t _newestTime;
	uint64_t _oldestTime;
public:
	TimedDataAggregator() : _dataQueue(0), _newest(0), _oldest(0), _lastUpdateTime(0), _newestTime(0), _oldestTime(0) {}
	void initialize(Intrusive::FiFoQueue *dataQueue) { _dataQueue = dataQueue; }
	void addTime(uint64_t currentTime);
	void reset();
};

class TimedDataHistory
{
protected:
	Intrusive::FiFoQueue _dataQueue;
	Intrusive::LinkedList _aggregators;
	TimedData *_lastData;
	uint64_t _maximumDuration;
public:
	TimedDataHistory() : _lastData(0), _maximumDuration(0) {}
	void addAggregator(TimedDataAggregator *aggregator);
	int addData(TimedData *data);
	int addTime(uint64_t currentTime);
	void reset();
};
