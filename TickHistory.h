#ifndef _TICK_HISTORY_H_
#define _TICK_HISTORY_H_

#include "History.h"
#include "Tick.h"

#include <cstdint>

struct TradeData: public TimedData
{
	Tick trade;
	Tick bid;
	Tick ask;
	TradeData() {}
	TradeData(unsigned millisecond, const Tick &t, const Tick &b, const Tick &a) : TimedData(millisecond), trade(t), bid(b), ask(a) {}
	void initialize(unsigned millisecond, const Tick &t, const Tick &b, const Tick &a) { _time = millisecond; trade = t; bid = b; ask = a; }
};

struct QuoteData: public TimedData
{
	Tick bid;
	Tick ask;
	QuoteData() {}
	QuoteData(unsigned millisecond, const Tick &b, const Tick &a) : TimedData(millisecond), bid(b), ask(a) {}
	void initialize(unsigned millisecond, const Tick &b, const Tick &a) { _time = millisecond; bid = b; ask = a; }
};

class QuoteDataPool
{
	static ObjectPool<QuoteData> pool;
};

class TradeDataPool
{
	static ObjectPool<TradeData> pool;
};

class QuoteDataBucket : public DataBucket
{
protected:
	uint64_t _bid;
	uint64_t _ask;
	uint64_t _bidSize;
	uint64_t _askSize;
	unsigned _quotes;
public:
	QuoteDataBucket(): _bid(0), _ask(0), _bidSize(0), _askSize(0), _quotes(0) {}

	virtual void addData(unsigned duration, const void *data, bool newDataFlag);
	virtual void subData(unsigned duration, const void *data, bool newDataFlag);
	virtual DataBucket &operator += (const DataBucket &dataBucket);
	virtual DataBucket &operator -= (const DataBucket &dataBucket);
	virtual void reset();

	double bid() const { return _duration ? (double)_bid / 10000 / _duration: 0; }
	double ask() const { return _duration ? (double)_ask / 10000 / _duration: 0; }
	double bidSize() const { return _duration ? (double)_bidSize / _duration : 0; }
	double askSize() const { return _duration ? (double)_askSize / _duration : 0; }
};

class TradeDataBucket : public DataBucket
{
protected:
	double _price;
	uint64_t _volume;
	unsigned _trades;
	int64_t _relativeValue;
public:
	TradeDataBucket() : _price(0), _volume(0), _trades(0), _relativeValue(0) {}
	virtual void addData(unsigned duration, const void *data, bool newDataFlag);
	virtual void subData(unsigned duration, const void *data, bool newDataFlag);
	virtual DataBucket &operator += (const DataBucket &dataBucket);
	virtual DataBucket &operator -= (const DataBucket &dataBucket);
	virtual void reset();

	unsigned price() const { return static_cast<unsigned>(_volume ? _price / _volume : 0); }
	unsigned volume() const { return static_cast<unsigned>(_volume); }
	unsigned trades() const { return _trades; }
	int64_t relativeValue() const { return _relativeValue; }
};


#endif
