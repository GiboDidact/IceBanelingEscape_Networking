#pragma once

//timers are in milliseconds
struct Timer
{
public:
	typedef std::chrono::time_point<std::chrono::high_resolution_clock> time_point;
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start_frame_time;
	bool has_started = false;
public:
	static time_point getCurrentTime()
	{
		return std::chrono::high_resolution_clock::now();
	}

	static int getDifference(time_point new_point, time_point old_point)
	{
		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(old_point).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(new_point).time_since_epoch().count();
		auto duration = end - start;
		return duration;
	}

	static int getTimeSinceEpoch(time_point new_point)
	{
		return std::chrono::time_point_cast<std::chrono::milliseconds>(new_point).time_since_epoch().count();
	}

	Timer() = default;

	time_point getStartPoint() { return start_frame_time; }
	
	void Begin()
	{
		start_frame_time = std::chrono::high_resolution_clock::now();
		has_started = true;
	}

	bool TimeElapsed(int duration_milliseconds)
	{
		if (!has_started)
			return false;
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_frame_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(endTimepoint).time_since_epoch().count();
		auto duration = end - start;

		if (duration >= duration_milliseconds)
			return true;
		else
			return false;
	}

	float getTime()
	{
		auto endTimepoint = std::chrono::high_resolution_clock::now();

		auto start = std::chrono::time_point_cast<std::chrono::milliseconds>(start_frame_time).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::milliseconds>(endTimepoint).time_since_epoch().count();
		auto duration = end - start;
		return duration;
	}
};