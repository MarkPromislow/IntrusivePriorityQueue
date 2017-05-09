#ifndef _DATA_HISTORY_H_
#define _DATA_HISTORY_H_

#include "LinkedList.h"

// accumulates data in buckets
class DataBucket
{
protected:
	unsigned _duration;
public:
	DataBucket(): _duration(0) {}
	virtual void addData(unsigned duration, const void *data, bool newFlag) = 0;
	virtual void subData(unsigned duration, const void *data, bool newFlag) = 0;
	void addDuration(unsigned duration) { _duration += duration; }
	void subDuration(unsigned duration) { _duration -= duration; }
	virtual DataBucket &operator += (const DataBucket &dataBucket ) = 0;
	virtual DataBucket &operator -= (const DataBucket &dataBucket) = 0;
	unsigned duration() const { return _duration; }
	virtual void reset() { _duration = 0; }
	virtual ~DataBucket() {}
};

struct BucketGenerator
{
	virtual DataBucket *generate(size_t bucketCnt) = 0;
	virtual size_t bucketSize() = 0;
};

template <typename TYPE>
struct DefaultBucketGenearator: public BucketGenerator
{
	DataBucket *generate(size_t bucketCnt) { return new TYPE[bucketCnt]; }
	size_t bucketSize() { return sizeof(TYPE); }
};

class Aggregator : public LinkedObject
{
protected:
	unsigned _beginOffset;
	unsigned _endOffset;
	DataBucket *_dataBucket;
public:
	Aggregator(): _beginOffset(0), _endOffset(0), _dataBucket(0) { }

	int initialize(unsigned beginOffset, unsigned endOffset, DataBucket *dataBucket)
	{
		_beginOffset = beginOffset;
		_endOffset = endOffset;
		_dataBucket = dataBucket;
		if (endOffset <= beginOffset) return -1;

		return 0;
	}

	unsigned beginOffset() { return _beginOffset; }
	unsigned endOffset() { return _endOffset; }
	const DataBucket *dataBucket() { return _dataBucket; }

	virtual void addData(unsigned duration, const void *data, bool newDataFlag) { _dataBucket->addData(duration, data, newDataFlag); }
	virtual void subData(unsigned duration, const void *data, bool newDataFlag) { _dataBucket->subData(duration, data, newDataFlag); }
	void addDuration(unsigned duration) { _dataBucket->addDuration(duration); }
	unsigned duration() { return _dataBucket->duration(); }

	virtual void reset() { _dataBucket->reset(); }

	virtual ~Aggregator() { if (_dataBucket) delete _dataBucket; }
};

class BucketAggregator: public Aggregator
{
public:
	virtual void addBucket(const DataBucket &dataBucket) { *_dataBucket += dataBucket; };
	virtual void substractBucket(const DataBucket &dataBucket) { *_dataBucket -= dataBucket; };
};

class BucketHistory
{
protected:
	unsigned _bucketCnt;
	unsigned _bucketDuration;
	unsigned _beginTime;
	unsigned _lastTime;

	unsigned _previousBucketInt;
	unsigned _previousTime;
	bool _stopped;

	DataBucket *_previousBucket;
	DataBucket *_dataBucket;
	size_t _bucketSize;

	LinkedList _aggregators;
public:
	BucketHistory();

	int BucketHistory::initialize(unsigned bucketDuration, unsigned beginTime, unsigned endTime, BucketGenerator &bucketGenerator);
	void addAggregator(BucketAggregator *aggregator);

	unsigned bucketCount() { return _bucketCnt; }
	unsigned bucketDuration() { return _bucketDuration; }
	unsigned beginTime() { return _beginTime; }
	unsigned endTime() { return _lastTime + 1; }
	unsigned bucketCountForTime(unsigned timeUnit);
	DataBucket &bucketForOffset(unsigned bucketInt);
	DataBucket *incrementDataBucket(const DataBucket *dataBucket);

	virtual unsigned addData(unsigned currentTime, const void *data);
	virtual unsigned addTime(unsigned currentTime);
	virtual unsigned stop(unsigned currentTime);

	virtual void reset();

	virtual ~BucketHistory();
private:
	BucketHistory(const BucketHistory &);
	BucketHistory &operator=(const BucketHistory &) { return *this; }
};

inline DataBucket &BucketHistory::bucketForOffset(unsigned bucketInt)
{
	return *(DataBucket*)((char*)_dataBucket + bucketInt * _bucketSize);
}

inline DataBucket *BucketHistory::incrementDataBucket(const DataBucket *dataBucket)
{
	return (DataBucket*)((char*)dataBucket +  _bucketSize);
}

// Adds Previous TimedData
class TimeWeightedBucketHistory : public BucketHistory
{
protected:
	void *_previousData;
	bool _newDataFlag;
public:
	TimeWeightedBucketHistory(): _previousData(0), _newDataFlag(false) {}

	unsigned addData(unsigned currentTime, void *data);
	unsigned addTime(unsigned currentTime);
	unsigned stop(unsigned currentTime);

	void reset() { BucketHistory::reset();  _previousData = 0; _newDataFlag = false; }
};

//
// Short term histiory implemented as a queue of data points
//
class TimedData : public QueuedObject
{
public:
	unsigned _time;
public:
	TimedData() : _time(0) {}
	TimedData(unsigned time): _time(time) {}
	inline unsigned time() const { return _time; }
	void reset() { _time = 0; }
};

class TimedHistory;
// Adds TimedData
class TimedAggregator : public Aggregator
{
protected:
	FiFoQueue *_dataQueue;
	TimedData *_newest;
	TimedData *_oldest;
	unsigned _lastUpdateTime;
	unsigned _newestTime;
	unsigned _oldestTime;
	bool _newNewestFlag;
	bool _newOldestFlag;
public:
	TimedAggregator() : _dataQueue(0), _newest(0), _oldest(0), _lastUpdateTime(0), _newestTime(0), _oldestTime(0) {}
	void setDataQueue(FiFoQueue *dataQueue) { _dataQueue = dataQueue; }
	virtual void addTime(unsigned currentTime);
	virtual void reset();
};

// Adds Previous TimedData
class TimeWeightedAggregator : public TimedAggregator
{
protected:
public:
	TimeWeightedAggregator() {}
	virtual void addTime(unsigned currentTime);
};

class TimedHistory
{
protected:
	FiFoQueue _dataQueue;
	TimedData *_lastData;
	LinkedList _aggregators;
	unsigned _maximumDuration;
public:
	TimedHistory(): _lastData(0), _maximumDuration(0) {}
	void addAggregator(TimedAggregator *aggregator);
	unsigned addData(unsigned currentTime, TimedData *data);
	unsigned addTime(unsigned currentTime);
	void reset();
};

#endif
