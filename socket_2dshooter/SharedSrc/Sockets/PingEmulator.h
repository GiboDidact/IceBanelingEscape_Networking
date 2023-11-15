#pragma once
#include "../pch.h"
#include <random>
#include <queue>

//#define PING_EMULATOR
//takes in send messages and emulates ping
class PingEmulator
{
public:
	PingEmulator() { }
	~PingEmulator() 
	{
		while (!msgs.empty())
		{
			delete[] msgs.front().buf;
			msgs.pop();
		}
	}

	static void addmsg(SOCKET socket, const char* buf, int len)
	{
		send_data data;
		data.socket = socket;
		data.len = len;
		data.buf = new char[len];
		memcpy(data.buf, buf, len);
		data.timer.Begin();
		
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> dist(1.0, 4000.0);
		float ping = 0;
		if (client)
			ping = dist(mt);
		else
			ping = 0;
		data.ping = ping;
		//std::cout << ping << std::endl;

		msgs.push(data);
	}

	static void update()
	{
		while (!msgs.empty() && msgs.front().timer.TimeElapsed(msgs.front().ping))
		{
			send_data& data = msgs.front();
			send(data.socket, data.buf, data.len, 0);
			delete[] data.buf;

			msgs.pop();
		}
	}

	inline static bool client = true;
private:
	struct send_data
	{
		SOCKET socket;
		char* buf;
		int len;
		Timer timer;
		int ping; //milliseconds
	};

	inline static std::queue<send_data> msgs;
};

class PingEmulatorUDP
{
public:
	PingEmulatorUDP() { }
	~PingEmulatorUDP()
	{
		for (auto& msg : msgs)
		{
			delete[] msg.buf;
		}
		msgs.clear();
	}

	static void addmsg(SOCKET socket, const char* buf, int len, sockaddr addr)
	{
		send_data data;
		data.socket = socket;
		data.addr = addr;
		data.len = len;
		data.buf = new char[len];
		memcpy(data.buf, buf, len);
		data.timer.Begin();

		float ping = 0.0;
		if (client)
		{
			bool static_ping_test = false;
			bool ping_variability_test = true;
			bool dropped_packet_test = true;

			std::random_device rd;
			std::mt19937 mt(rd());
			if (static_ping_test)
			{
				ping = 150.0;
			}
			else if (ping_variability_test)
			{
				std::uniform_real_distribution<float> dist(0.0, 100.0);
				ping = dist(mt);
			}
			if (dropped_packet_test)
			{
				std::uniform_real_distribution<float> dist(0.0, 1.0);
				float drop_probability = .05;
				if (dist(mt) <= drop_probability)
				{
					//std::cout << "packet dropped!\n";
					delete[] data.buf;
					return;
				}
			}
		}

		data.ping = ping;
		//std::cout << ping << std::endl;
		msgs.push_back(data);
	}

	static void update()
	{
		for (int i = 0; i < msgs.size(); i++)
		{
			if (msgs[i].timer.TimeElapsed(msgs[i].ping))
			{
				sendto(msgs[i].socket, msgs[i].buf, msgs[i].len, 0, &msgs[i].addr, sizeof(sockaddr));
				delete[]msgs[i].buf;
				msgs.erase(msgs.begin() + i);
				i--;
			}
		}
	}

	inline static bool client = true;
private:
	struct send_data
	{
		SOCKET socket;
		sockaddr addr;
		char* buf;
		int len;
		Timer timer;
		int ping; //milliseconds
	};

	inline static std::vector<send_data> msgs;
};