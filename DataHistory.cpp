#include "DataHistory.h"

int Aggregator::initialize(uint64_t beginOffset, uint64_t endOffset, DataBucket * dataBucket)
{
	_beginOffset = beginOffset;
	_endOffset = endOffset;
	_dataBucket = dataBucket;
	return _beginOffset < _endOffset ? 0: -1;
}

BucketHistory::BucketHistory():
	_bucketCnt(0), _bucketDuration(0), _beginTime(0), _lastTime(0), _previousBucket(0), _previousTime(0), _dataBuckets(0)
{
}

void BucketHistory::addAggregator(BucketAggregator * aggregator)
{
	Intrusive::LinkedListObject *obj = _aggregators.begin();
	for (; obj != _aggregators.end(); obj = obj->next())
	{
		if (aggregator->beginOffset() > static_cast<BucketAggregator*>(obj)->beginOffset()) break;
	}
	obj->linkBefore(aggregator);
}

int BucketHistory::addTime(uint64_t currentTime)
{
	if (currentTime < _previousTime) return -1;

	if (currentTime > _beginTime && currentTime > _previousTime)
	{
		if (_previousTime < _beginTime) _previousTime = _beginTime;
		if (currentTime > _lastTime)
		{
			if (_previousBucketInt == _bucketCnt) return 0;
			currentTime = _lastTime + 1;
		}

		unsigned currentBucketInt = static_cast<unsigned>((currentTime - _beginTime) / _bucketCnt);

		// close out prior buckets
		while (currentBucketInt > _previousBucketInt)
		{
			uint64_t bucketEndTime = (_previousBucketInt + 1) * _bucketDuration + _beginTime;

			// update buckets
			if (_previousData)
			{
				int64_t duration = bucketEndTime - _previousTime;
				_previousBucket->addTime(duration, _previousData);

				// add data to real time aggregators
				for (Intrusive::LinkedListObject *obj = _aggregators.begin(); obj != _aggregators.end(); obj = obj->next())
				{
					Aggregator *aggregator = static_cast<Aggregator*>(obj);
					if (aggregator->beginOffset()) break;
					aggregator->addTime(duration, _previousData);
				}
			}

			// add buckets to aggregators
			for (Intrusive::LinkedListObject *obj = _aggregators.begin(); obj != _aggregators.end(); obj = obj->next())
			{
				BucketAggregator *aggregator = static_cast<BucketAggregator*>(obj);
				if (aggregator->beginOffset())
				{
					unsigned addBucketOffset = static_cast<unsigned>(aggregator->beginOffset() / _bucketDuration);
					if (addBucketOffset > _previousBucketInt) break;
					aggregator->addBucket(bucketForOffset(_previousBucketInt - addBucketOffset));
				}

				unsigned subBucketOffset = static_cast<unsigned>(aggregator->endOffset() / _bucketDuration);
				if (subBucketOffset < _previousBucketInt) continue;
				aggregator->subBucket(bucketForOffset(_previousBucketInt - subBucketOffset));
			}

			_previousTime = bucketEndTime;
			if (++_previousBucketInt == _bucketCnt) return 0;
			_previousBucket = reinterpret_cast<DataBucket*>(reinterpret_cast<char*>(_previousBucket) + _bucketSize);
		}

		// update current bucket
		if (_previousData)
		{
			int64_t duration = currentTime - _previousTime;
			_previousBucket->addTime(duration, _previousData);

			// add data to real time aggregators
			for (Intrusive::LinkedListObject *obj = _aggregators.begin(); obj != _aggregators.end(); obj = obj->next())
			{
				Aggregator *aggregator = static_cast<Aggregator*>(obj);
				if (aggregator->beginOffset()) break;
				aggregator->addTime(duration, _previousData);
			}
		}
	}

	_previousTime = currentTime;
	return 0;
}

int BucketHistory::addData(uint64_t currentTime, const void *data)
{
	if (currentTime < _previousTime) return -1;
	if (currentTime > _previousTime) addTime(currentTime);
	_previousBucket->addData(data);

	// add data to real time aggregators
	for (Intrusive::LinkedListObject *obj = _aggregators.begin(); obj != _aggregators.end(); obj = obj->next())
	{
		Aggregator *aggregator = static_cast<Aggregator*>(obj);
		if (aggregator->beginOffset()) break;
		aggregator->addData(data);
	}

	_previousData = data;

	return 0;
}

void BucketHistory::stop(uint64_t currentTime)
{
	if (_previousData && currentTime > _previousTime) addTime(currentTime);
	_previousData = 0;
}

void BucketHistory::reset()
{
	for (unsigned bucketInt(0); bucketInt < _bucketCnt; ++bucketInt)
	{
		bucketForOffset(bucketInt).reset();
	}

	for (Intrusive::LinkedListObject *obj = _aggregators.begin(); obj != _aggregators.end(); obj = obj->next())
	{
		Aggregator *aggregator = static_cast<Aggregator*>(obj);
		aggregator->reset();
	}

	_previousTime = 0;
	_previousBucketInt = 0;
	_previousBucket = _dataBuckets;
	_previousData = 0;
}

BucketHistory::~BucketHistory()
{
	if (_dataBuckets) delete[] _dataBuckets;
}
