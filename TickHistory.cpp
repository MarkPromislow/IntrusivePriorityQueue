#include "TIckHistory.h"

void TradeDataBucket::addData(unsigned duration, const void *data, bool newDataFlag)
{
	_duration += duration;
	if (newDataFlag)
	{
		const TradeData *tradeData = static_cast<const TradeData*>(data);
		const Tick &trade = tradeData->trade;
		const Tick &bid = tradeData->bid;
		const Tick &ask = tradeData->ask;
		if (trade.size > 20000)
			return;
		_price += static_cast<uint64_t>(trade.price) * trade.size;
		_volume += trade.size;
		++_trades;
		_relativeValue += (static_cast<int64_t>(trade.price) - (bid.price + ask.price) / 2) * trade.size;
	}
}
#include <iostream>
void TradeDataBucket::subData(unsigned duration, const void *data, bool newDataFlag)
{
	_duration -= duration;
	if (newDataFlag)
	{
		const TradeData *tradeData = static_cast<const TradeData*>(data);
		const Tick &trade = tradeData->trade;
		const Tick &bid = tradeData->bid;
		const Tick &ask = tradeData->ask;
		if (trade.size > 20000)
			return;
		_price -= static_cast<double>(trade.price) * trade.size;
		if (_volume < trade.size || !_trades)
		{
			std::cout << "ERROR: volume " << _volume << '-' << trade.size << std::endl;
			return;
		}
		_volume -= trade.size;
		--_trades;
		_relativeValue -= (static_cast<int64_t>(trade.price) - (bid.price + ask.price) / 2) * trade.size;
	}
}

DataBucket &TradeDataBucket::operator+=(const DataBucket & dataBucket)
{
	const TradeDataBucket &tradeDataBucket = static_cast<const TradeDataBucket&>(dataBucket);
	_duration += tradeDataBucket._duration;
	_price += tradeDataBucket._price;
	_volume += tradeDataBucket._volume;
	_trades += tradeDataBucket._trades;
	_relativeValue += tradeDataBucket._relativeValue;
	return *this;
}

DataBucket &TradeDataBucket::operator-=(const DataBucket & dataBucket)
{
	const TradeDataBucket &tradeDataBucket = static_cast<const TradeDataBucket&>(dataBucket);
	_duration -= tradeDataBucket._duration;
	_price -= tradeDataBucket._price;
	_volume -= tradeDataBucket._volume;
	_trades -= tradeDataBucket._trades;
	_relativeValue -= tradeDataBucket._relativeValue;
	return *this;
}

void TradeDataBucket::reset()
{
	DataBucket::reset();
	_price = 0;
	_volume = 0;
	_trades = 0;
	_relativeValue = 0;
}
#include <iostream>

void QuoteDataBucket::addData(unsigned duration, const void *data, bool newDataFlag)
{
	const QuoteData *quoteData = static_cast<const QuoteData*>(data);
	const Tick &bid = quoteData->bid;
	const Tick &ask = quoteData->ask;
	//std::cout << "addData," << bid.cnt << ',' << bid.time() << ',' << bid.price << ',' << duration << ',' << newDataFlag << ',' << _quotes + 1 << std::endl;
	//if (duration && bid->price && bid->price < ask->price)
	{
		_duration += duration;
		_bid += static_cast<uint64_t>(bid.price) * duration;
		_ask += static_cast<uint64_t>(ask.price) * duration;
		_bidSize += static_cast<uint64_t>(bid.size) * duration;
		_askSize += static_cast<uint64_t>(ask.size) * duration;
	}
	if(newDataFlag) ++_quotes;
}

void QuoteDataBucket::subData(unsigned duration, const void *data, bool newDataFlag)
{
	const QuoteData *quoteData = static_cast<const QuoteData*>(data);
	const Tick &bid = quoteData->bid;
	const Tick &ask = quoteData->ask;
	//std::cout << "subData," << bid.cnt << ',' << bid.time() << ',' << bid.price << ',' << duration << ',' << newDataFlag << ',' << _quotes - 1 << std::endl;
	//if (duration && bid->price && bid->price < ask->price)
	{
		_duration -= duration;
		_bid -= static_cast<uint64_t>(bid.price) * duration;
		_ask -= static_cast<uint64_t>(ask.price) * duration;
		_bidSize -= static_cast<uint64_t>(bid.size) * duration;
		_askSize -= static_cast<uint64_t>(ask.size) * duration;
	}
	if(newDataFlag) --_quotes;
}

DataBucket & QuoteDataBucket::operator+=(const DataBucket &dataBucket)
{
	const QuoteDataBucket &quoteDataBucket = static_cast<const QuoteDataBucket&>(dataBucket);
	_duration += quoteDataBucket._duration;
	_bid += quoteDataBucket._bid;
	_ask += quoteDataBucket._ask;
	_bidSize += quoteDataBucket._bidSize;
	_askSize += quoteDataBucket._askSize;
	_quotes += quoteDataBucket._quotes;
	return *this;
}

DataBucket & QuoteDataBucket::operator-=(const DataBucket &dataBucket)
{
	const QuoteDataBucket &quoteDataBucket = static_cast<const QuoteDataBucket&>(dataBucket);
	_duration -= quoteDataBucket._duration;
	_bid -= quoteDataBucket._bid;
	_ask -= quoteDataBucket._ask;
	_bidSize -= quoteDataBucket._bidSize;
	_askSize -= quoteDataBucket._askSize;
	_quotes -= quoteDataBucket._quotes;
	return *this;
}
void QuoteDataBucket::reset()
{
	DataBucket::reset();
	_bid = 0;
	_ask = 0;
	_quotes = 0;
}

ObjectPool<QuoteData> QuoteDataPool::pool;
ObjectPool<TradeData> TradeDataPool::pool;
