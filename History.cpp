#include "History.h"

#include <iostream>

BucketHistory::BucketHistory() :
	_bucketDuration(0), _beginTime(0), _lastTime(0), _bucketCnt(0), _previousTime(0), _previousBucketInt(0), _dataBucket(0)
{}

int BucketHistory::initialize(unsigned bucketDuration, unsigned beginTime, unsigned endTime, BucketGenerator &bucketGenerator)
{
	if (bucketDuration < 1 || endTime <= beginTime) return -1;
	_bucketDuration = bucketDuration;
	_beginTime = beginTime;
	_lastTime = endTime - 1;
	_stopped = true;

	_bucketCnt = (endTime - _beginTime) / _bucketDuration;

	if (_dataBucket) delete[] _dataBucket;
	_dataBucket = bucketGenerator.generate(_bucketCnt);
	_bucketSize = bucketGenerator.bucketSize();

	reset();

	return _bucketCnt;
}

void BucketHistory::addAggregator(BucketAggregator *aggregator)
{
	LinkedObject *object = _aggregators.begin();
	for (; object != _aggregators.end(); object = object->next())
	{
		if (aggregator->beginOffset() > static_cast<Aggregator*>(object)->beginOffset()) break;
	}
	object->linkBefore(aggregator);
}

unsigned BucketHistory::bucketCountForTime(unsigned timeUnit)
{
	if (timeUnit < _beginTime) return 0;
	if (timeUnit > _lastTime) return _bucketCnt - 1;
	return (timeUnit - _beginTime) / _bucketDuration;
}

unsigned BucketHistory::addTime(unsigned currentTime)
{
	if (currentTime < _beginTime || currentTime > _lastTime)
	{
		_previousTime = currentTime;
		return -1;
	}

	if (_previousTime < _beginTime) _previousTime = _beginTime;
	if (currentTime > _lastTime)
	{
		if (_previousBucketInt == _bucketCnt) return _previousBucketInt;
		currentTime = _lastTime + 1;
	}
	unsigned currentBucketInt = (currentTime - _beginTime) / _bucketDuration;

	// close out prior buckets
	while (currentBucketInt > _previousBucketInt)
	{
		unsigned bucketEndTime = (_previousBucketInt + 1) * _bucketDuration + _beginTime;

		// Update Buckets
		unsigned duration = bucketEndTime - _previousTime;
		if (_previousTime)
		{
			_previousBucket->addDuration(duration);

			// Add Data to Real Time Aggregators
			for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
			{
				Aggregator *aggregator = static_cast<Aggregator*>(object);
				if (aggregator->beginOffset()) break;
				aggregator->addDuration(duration);
			}

			// Add Buckets to Aggregators
			for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
			{
				BucketAggregator *aggregator = static_cast<BucketAggregator*>(object);
				if (aggregator->beginOffset())
				{
					unsigned addBucketOffset = aggregator->beginOffset() / _bucketDuration;
					if (addBucketOffset > _previousBucketInt) break;
					aggregator->addBucket(bucketForOffset(_previousBucketInt - addBucketOffset));
				}
				unsigned subBucketOffset = aggregator->endOffset() / _bucketDuration - 1;
				if (_previousBucketInt >= subBucketOffset)
				{
					aggregator->substractBucket(bucketForOffset(_previousBucketInt - subBucketOffset));
				}
			}
		}

		_previousTime = bucketEndTime;
		if (++_previousBucketInt == _bucketCnt) return _previousBucketInt;
		_previousBucket = incrementDataBucket(_previousBucket);
	}

	// Update Current Bucket
	unsigned duration = currentTime - _previousTime;
	_previousBucket->addDuration(duration);

	// Add Data to Real Time Aggregators
	for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
	{
		Aggregator *aggregator = static_cast<Aggregator*>(object);
		if (aggregator->beginOffset()) break;
		aggregator->addDuration(duration);
	}

	_previousTime = currentTime;
	return _previousBucketInt;
}

unsigned BucketHistory::addData(unsigned currentTime, const void *data)
{
	if (currentTime < _beginTime || currentTime > _lastTime)
	{
		_previousTime = currentTime;
		return -1;
	}

	if (_previousTime < _beginTime) _previousTime = _beginTime;
	if (currentTime > _lastTime)
	{
		if (_previousBucketInt == _bucketCnt) return _previousBucketInt;
		currentTime = _lastTime + 1;
	}
	unsigned currentBucketInt = (currentTime - _beginTime) / _bucketDuration;

	// close out prior buckets
	while (currentBucketInt > _previousBucketInt)
	{
		unsigned bucketEndTime = (_previousBucketInt + 1) * _bucketDuration + _beginTime;

		// Update Buckets
		unsigned duration = bucketEndTime - _previousTime;
		if (_previousTime)
		{
			_previousBucket->addDuration(duration);

			// Add Data to Real Time Aggregators
			for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
			{
				Aggregator *aggregator = static_cast<Aggregator*>(object);
				if (aggregator->beginOffset()) break;
				aggregator->addDuration(duration);
			}

			// Add Buckets to Aggregators
			for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
			{
				BucketAggregator *aggregator = static_cast<BucketAggregator*>(object);
				if (aggregator->beginOffset())
				{
					unsigned addBucketOffset = aggregator->beginOffset() / _bucketDuration;
					if (addBucketOffset > _previousBucketInt) break;
					aggregator->addBucket(bucketForOffset(_previousBucketInt - addBucketOffset));
				}
				unsigned subBucketOffset = aggregator->endOffset() / _bucketDuration - 1;
				if (_previousBucketInt >= subBucketOffset)
				{
					aggregator->substractBucket(bucketForOffset(_previousBucketInt - subBucketOffset));
				}
			}
		}

		_previousTime = bucketEndTime;
		if (++_previousBucketInt == _bucketCnt) return _previousBucketInt;
		_previousBucket = incrementDataBucket(_previousBucket);
	}

	// Update Current Bucket
	unsigned duration = currentTime - _previousTime;
	_previousBucket->addData(duration, data, true);

	// Add Data to Real Time Aggregators
	for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
	{
		Aggregator *aggregator = static_cast<Aggregator*>(object);
		if (aggregator->beginOffset()) break;
		aggregator->addData(duration, data, true);
	}

	_previousTime = currentTime;
	return _previousBucketInt;
}

unsigned BucketHistory::stop(unsigned currentTime)
{
	if (currentTime < _beginTime || currentTime > _lastTime) return -1;
	addTime(currentTime);
	_stopped = true;
	return 0;
}

void BucketHistory::reset()
{
	for (unsigned bucketInt(0); bucketInt < _bucketCnt; ++bucketInt)
		bucketForOffset(bucketInt).reset();

	for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
		static_cast<BucketAggregator*>(object)->reset();

	_previousTime = 0;
	_previousBucketInt = 0;

	_previousBucket = _dataBucket;
}

BucketHistory::~BucketHistory()
{
	if (_dataBucket) delete[] _dataBucket;
}

/*
** TimeWeightedBucketHistory
*/

unsigned TimeWeightedBucketHistory::addTime(unsigned currentTime)
{
	if (currentTime <= _previousTime) return -1;

	if (currentTime >= _beginTime)
	{
		if (_previousTime < _beginTime)
			_previousTime = _beginTime;
		if (currentTime > _lastTime)
		{
			if (_previousBucketInt == _bucketCnt) return _previousBucketInt;
			currentTime = _lastTime + 1;
		}
		unsigned currentBucketInt = (currentTime - _beginTime) / _bucketDuration;

		// close out prior buckets
		while (currentBucketInt > _previousBucketInt)
		{
			unsigned bucketEndTime = (_previousBucketInt + 1) * _bucketDuration + _beginTime;

			// Update Buckets
			if (_previousData)
			{
				unsigned duration = bucketEndTime - _previousTime;
				_previousBucket->addData(duration, _previousData, _newDataFlag);

				// Add Data to Real Time Aggregators
				for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
				{
					Aggregator *aggregator = static_cast<Aggregator*>(object);
					if (aggregator->beginOffset()) break;
					aggregator->addData(duration, _previousData, _newDataFlag);
				}
				_newDataFlag = false;
			}

			// Add Buckets to Aggregators
			for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
			{
				BucketAggregator *aggregator = static_cast<BucketAggregator*>(object);
				if (aggregator->beginOffset())
				{
					unsigned addBucketOffset = aggregator->beginOffset() / _bucketDuration;
					if (addBucketOffset > _previousBucketInt) break;
					aggregator->addBucket(bucketForOffset(_previousBucketInt - addBucketOffset));
				}
				unsigned subBucketOffset = aggregator->endOffset() / _bucketDuration - 1;
				if (_previousBucketInt >= subBucketOffset)
				{
					aggregator->substractBucket(bucketForOffset(_previousBucketInt - subBucketOffset));
				}
			}

			_previousTime = bucketEndTime;
			if (++_previousBucketInt == _bucketCnt) return _previousBucketInt;
			_previousBucket = incrementDataBucket(_previousBucket);
		}

		// Update Current Bucket
		unsigned duration = currentTime - _previousTime;
		if (_previousData)
		{
			unsigned duration = currentTime - _previousTime;
			_previousBucket->addData(duration, _previousData, _newDataFlag);

			// Add Data to Real Time Aggregators
			for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
			{
				Aggregator *aggregator = static_cast<Aggregator*>(object);
				if (aggregator->beginOffset()) break;
				aggregator->addData(duration, _previousData, _newDataFlag);
			}

			_newDataFlag = false;
		}
	}

	_previousTime = currentTime;
	return _previousBucketInt;
}

unsigned TimeWeightedBucketHistory::addData(unsigned currentTime, void *data)
{
	if (currentTime < _previousTime) return -1;

	if (currentTime > _previousTime) addTime(currentTime);
	_previousData = data;
	_newDataFlag = true;

	return _previousBucketInt;
}

unsigned TimeWeightedBucketHistory::stop(unsigned currentTime)
{
	if (! _stopped && currentTime > _previousTime) addTime(currentTime);
	_stopped = true;

	if (currentTime < _previousTime) return -1;

	return _previousBucketInt;
}

/*
** TimedAggregator
*/

void TimedAggregator::addTime(unsigned currentTime)
{
	if(currentTime > _updateTime) _updateTime = currentTime;

	if (!_newest)
	{
		_oldest = _newest = static_cast<TimedData*>(_dataQueue->back());
		if (!_newest) return;
		_oldestTime = _newestTime = _newest->time();
		_newOldestFlag = _newNewestFlag = true;
	}

	// add new data
	if (_updateTime < _beginOffset) return;
	unsigned startTime = _updateTime - _beginOffset;
	for (TimedData *data = static_cast<TimedData*>(_newest->next());
		data && startTime >= data->time();
		data = static_cast<TimedData*>(_newest->next()))
	{
		unsigned duration(0);
		if (data->time() > _newestTime)
		{
			duration = data->time() - _newestTime;
			_newestTime = data->time();
		}
		if (_newNewestFlag) addData(0, _newest, true);
		addData(duration, data, true);
		_newest = data;
		_newNewestFlag = false;
	}
	if (startTime > _newestTime)
	{
		if (_newNewestFlag)
		{
			addData(startTime - _newestTime, _newest, true);
			_newNewestFlag = false;
		}
		else
		{
			_dataBucket->addDuration(startTime - _newestTime);
		}
		_newestTime = startTime;
	}

	// substract old data
	unsigned endTime = _updateTime - _endOffset;
	for (TimedData *data = static_cast<TimedData*>(_oldest->next());
		data && endTime >= data->time();
		data = static_cast<TimedData*>(_oldest->next()))
	{
		unsigned duration(0);
		if (data->time() > _oldestTime)
		{
			duration = data->time() - _oldestTime;
			_oldestTime = data->time();
		}
		if (_newOldestFlag) subData(0, _oldest, true);
		subData(duration, data, true);
		_oldest = data;
		_newOldestFlag = false;
	}
	if (endTime > _oldestTime)
	{
		if (_newOldestFlag)
		{
			subData(endTime - _oldestTime, _oldest, true);
			_newOldestFlag = false;
		}
		else
		{
			_dataBucket->subDuration(endTime - _oldestTime);
		}
		_oldestTime = endTime;
	}
}

void TimedAggregator::reset()
{
	Aggregator::reset();
	_oldest = 0;
	_newest = 0;
	_updateTime = 0;
	_newestTime = 0;
	_oldestTime = 0;
}


/*
** TimeWeightedAggregator
*/
void TimeWeightedAggregator::addTime(unsigned currentTime)
{
	if (currentTime > _updateTime) _updateTime = currentTime;
	
	if (!_newest)
	{
		_oldest = _newest = static_cast<TimedData*>(_dataQueue->back());
		if (!_newest) return;
		_oldestTime = _newestTime = _newest->time();
		_newOldestFlag = _newNewestFlag = true;
	}

	// add new data
	if (_updateTime < _beginOffset) return;
	unsigned startTime = _updateTime - _beginOffset;
	for (TimedData *data = static_cast<TimedData*>(_newest->next());
		data && startTime >= data->time();
		data = static_cast<TimedData*>(_newest->next()))
	{
		unsigned duration(0);
		if (data->time() > _newestTime)
		{
			duration = data->time() - _newestTime;
			_newestTime = data->time();
		}
		addData(duration, _newest, _newNewestFlag);
		_newest = data;
		_newNewestFlag = true;
	}
	if (startTime > _newestTime)
	{
		_dataBucket->addData(startTime - _newestTime, _newest, _newNewestFlag);
		_newNewestFlag = false;
		_newestTime = startTime;
	}

	// substract old data
	if (_updateTime < _endOffset) return;
	unsigned endTime = _updateTime - _endOffset;
	for (TimedData *data = static_cast<TimedData*>(_oldest->next());
		data && endTime >= data->time();
		data = static_cast<TimedData*>(_oldest->next()))
	{
		unsigned duration(0);
		if (data->time() > _oldestTime)
		{
			duration = data->time() - _oldestTime;
			_oldestTime = data->time();
		}
		subData(duration, _oldest, _newOldestFlag);
		_oldest = data;
		_newOldestFlag = true;
	}
	if (endTime > _oldestTime)
	{
		_dataBucket->subData(endTime - _oldestTime, _oldest, _newOldestFlag);
		_newOldestFlag = false;
		_oldestTime = endTime;
	}
}

void TimedHistory::addAggregator(TimedAggregator *aggregator)
{
	aggregator->setDataQueue(&_dataQueue);
	LinkedObject *object = _aggregators.begin();
	for (; object != _aggregators.end(); object = object->next())
	{
		if (aggregator->beginOffset() > static_cast<Aggregator*>(object)->beginOffset()) break;
	}
	object->linkBefore(aggregator);
	if (_maximumDuration < aggregator->endOffset()) _maximumDuration = aggregator->endOffset();
}

unsigned TimedHistory::addTime(unsigned currentTime)
{
	for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
	{
		static_cast<TimedAggregator*>(object)->addTime(currentTime);
	}
	
	for (TimedData *data = static_cast<TimedData*>(_dataQueue.back()); data; data = static_cast<TimedData*>(_dataQueue.back()))
	{
		if (currentTime < data->time())
			continue;
		if(currentTime - data->time() <= _maximumDuration)
			break;
		_dataQueue.pop_back();
		if (_dataQueue.back())
		{
			if (_lastData) delete _lastData;
			_lastData = data;
		}
		else
		{
			_dataQueue.push_front(data);
			break;
		}
	}

	return 0;
}

unsigned TimedHistory::addData(unsigned currentTime, TimedData *data)
{
	_dataQueue.push_front(data);
	return addTime(currentTime);
}

void TimedHistory::reset()
{
	for (LinkedObject *object = _aggregators.begin(); object != _aggregators.end(); object = object->next())
	{
		Aggregator *aggregator = static_cast<Aggregator*>(object);
		aggregator->reset();
	}
	for (QueuedObject *object = _dataQueue.pop_back(); object; object = _dataQueue.pop_back())
	{
		TimedData *timedData = static_cast<TimedData*>(object);
		delete timedData;
	}
	if (_lastData) delete _lastData;
	_lastData = 0;
}
