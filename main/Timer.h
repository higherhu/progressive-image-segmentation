#pragma once

#pragma warning(disable:4512)
#include <string>
#include <stdio.h>
#include <assert.h>
#include <time.h>

using namespace std;
class Timer
{
public:	
	Timer::Timer(string t = "Timer"):title(t) { is_started = false; start_clock = 0; cumulative_clock = 0; n_starts = 0; }

	~Timer(){	/*if (is_started) printf("Timer '%s' is started and is being destroyed.\n", title.c_str());*/	}

	inline void Start();
	inline void Stop();
	inline void Reset();

	inline bool Report();
	inline bool StopAndReport() { Stop(); return Report(); }
	inline float TimeInSeconds();

	inline float AvgTime(){assert(is_started == false); return TimeInSeconds()/n_starts;}

private:
	string title;
	
	bool is_started;
	clock_t start_clock;
	clock_t cumulative_clock;
	unsigned int n_starts;
};

/************************************************************************/
/*                       Implementations                                */
/************************************************************************/

void Timer::Start()
{
	if (is_started){
		//printf("Timer '%s' is already started. Nothing done.\n", title.c_str());
		start_clock = clock();
		return;
	}

	is_started = true;
	n_starts++;
	start_clock = clock();
}

void Timer::Stop()
{
	if (!is_started){
		//printf("Timer '%s' is started. Nothing done\n", title.c_str());
		return;
	}

	cumulative_clock += clock() - start_clock;
	is_started = false;
}

void Timer::Reset()
{
	if (!is_started)	{
		//printf("Timer '%s'is started during reset request.\n Only reset cumulative time.\n");
		return;
	}
	cumulative_clock = 0;
}

bool Timer::Report()
{
	if (!is_started){
		//printf("Timer '%s' is started.\n Cannot provide a time report.", title.c_str());
		return false;
	}

	float timeUsed = TimeInSeconds();
	printf("[%s] CumuTime: %4gs, #run: %4d, AvgTime: %4gs\n", title.c_str(), timeUsed, n_starts, timeUsed/n_starts);
	return true;
}

float Timer::TimeInSeconds()
{
	if (!is_started){
		//printf("Timer '%s' is started. Nothing done\n", title.c_str());
		return 0;
	}
	/*return float(cumulative_clock) / CLOCKS_PER_SEC;*/
	return float(clock() - start_clock) / CLOCKS_PER_SEC;
}

