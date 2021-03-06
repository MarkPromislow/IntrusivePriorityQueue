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
	void addTime(int64_t duration, const void *data) { _dataBucket->addTime(duration, data); }
	void subTime(int64_t duration, const void *data) { _dataBucket->subTime(duration, data); }

	void addBucket(const DataBucket &dataBucket) { *_dataBucket += dataBucket; }
	void subBucket(const DataBucket &dataBucket) { *_dataBucket -= dataBucket; }

	void reset() { _dataBucket->reset(); }
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

	Intrusive::LinkedList _realTimeAggregators;
	Intrusive::LinkedList _aggregators;

	DataBucket *_bucketForOffset(unsigned offset);
public:
	BucketHistory();

	template <typename BucketType>
	int initialize(unsigned bucketDuration, uint64_t beginTime, uint64_t endTime, BucketGenerator<BucketType> &bucketGenerator);

	void addAggregator(Aggregator *aggregator);

	unsigned bucketCount() const { return _bucketCnt; }
	unsigned bucketDuration() const { return _bucketDuration; }
	uint64_t beginTime() const { return _beginTime; }
	uint64_t endTime() const { return _lastTime + 1; }
	uint64_t lastUpdateTime() const { return _previousTime; }
	unsigned bucketOffsetForTime(uint64_t timeUnit) const;
	DataBucket& bucketForOffset(unsigned offset);

	int addData(uint64_t currentTime, const void *data);
	int addTime(uint64_t currentTime);
	void stop(uint64_t currentTime);

	void reset();

	~BucketHistory();
private:
	BucketHistory(const BucketHistory&) = delete;
	BucketHistory& operator = (const BucketHistory&) = delete;
};

template<typename BucketType>
inline int BucketHistory::initialize(unsigned bucketDuration, uint64_t beginTime, uint64_t endTime, BucketGenerator<BucketType>& bucketGenerator)
{
	if (! bucketDuration || endTime < beginTime + bucketDuration) return -1;

	// data buckets
	_bucketDuration = bucketDuration;
	if (_dataBuckets) delete[] _dataBuckets;
	_bucketCnt = (endTime - beginTime - 1) / bucketDuration + 1;
	_dataBuckets = bucketGenerator.generate(_bucketCnt);
	_bucketSize = bucketGenerator.bucketSize();

	_beginTime = beginTime;
	_lastTime = endTime - 1;

	reset();

	return 0;
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

	friend class TimedDataHistory;
	void setDataQueue(Intrusive::FiFoQueue *dataQueue) { _dataQueue = dataQueue; }
public:
	TimedDataAggregator() : _dataQueue(0), _newest(0), _oldest(0), _lastUpdateTime(0), _newestTime(0), _oldestTime(0) {}
	void addTime(uint64_t currentTime);
	void reset();
};

class TimedDataHistory
{
protected:
	Intrusive::FiFoQueue _dataQueue;
	Intrusive::BaseQueuedObjectPool &_timedObjectPool;
	Intrusive::LinkedList _aggregators;
	TimedData *_lastData;
	uint64_t _maximumDuration;
public:
	TimedDataHistory(Intrusive::BaseQueuedObjectPool &timedObjectPool) : _timedObjectPool(timedObjectPool), _lastData(0), _maximumDuration(0) {}
	void addAggregator(TimedDataAggregator *aggregator);
	int addData(TimedData *data);
	int addTime(uint64_t currentTime);
	void reset();

	~TimedDataHistory();
};
