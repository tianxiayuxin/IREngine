#ifndef _TIMER_H
#define _TIMER_H

#include <sys/time.h>


namespace IRBox{
class Timer{
	Timer():_elapsedTime(0), _enabled(false)
	{

	}

	void setEnabled(bool enabled)
	{
		if(enabled && !_enabled)
		{
			_startTime = getTime();
			_enabled = true;
		}else if( !enabled && _enabled )
		{
			double endTime = getTime();
			_elapsedTime += endTime - _startTime;
			_enabled = false;
		}
	}

	void reset()
	{
		_elapsedTime = 0;
		if( _enabled ) _startTime = getTime();
	}

	float getElapsedTimeMS()
	{
		if( _enabled )
		{
			double endTime = getTime();
			_elapsedTime += endTime - _startTime;
			_startTime = endTime;
		}
		return (float)_elapsedTime;
	}


protected:
	double getTime()
	{
		timeval tv;
		gettimeofday(&tv, 0x0);
		return (double)tv.tv_sec*1000.0 + (double)tv.tv_usec/1000.0;
	}

protected:
	double     _startTime;
	double     _elapsedTime;
	bool       _enabled;
};
}


#endif // _TIMER_H
