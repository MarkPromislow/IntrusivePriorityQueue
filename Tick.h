#ifndef _TICK_EVENT_H_
#define _TICK_EVENT_H_

#include "LinkedList.h"
#include "History.h"

#include <time.h>

// event
enum TickType {BID, ASK, TRADE};

struct Tick: public TimedData
{
	//unsigned millisecond;
	unsigned price;
	unsigned size;
	TickType type;
	unsigned cnt;
	Tick(): price(0), size(0) {}
	void reset() { TimedData::reset();  price = 0; size = 0; cnt = 0; }
};

#endif
