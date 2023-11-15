#pragma once
#include "../Sockets/SocketInc.h"
#include <deque>

namespace NetworkDelivery
{
	struct acknowledgerange_s
	{
		acknowledgerange_s() : start_id(0), count(0) {}
		acknowledgerange_s(uint16_t start) : start_id(start), count(1) {}
		bool extendRange(uint16_t id)
		{
			if (id == start_id + count)
			{
				count++;
				return true;
			}
			return false;
		}

		uint16_t start_id;
		uint16_t count;
	};

	struct packet_data_s
	{
		packet_data_s() : data(nullptr), len(0) {}
		~packet_data_s() 
		{ 
			if (data) {
				delete[] data;
				data = nullptr;
			}
		}

		packet_data_s(const packet_data_s& other)
		{
			//std::cout << "constructor\n";
			len = other.len;
			addr = other.addr;
			data = new char[len];
			memcpy(data, other.data, len);
		}
		packet_data_s& operator=(const packet_data_s& other)
		{
			//std::cout << "assignment\n";
			if (data) {
				delete[] data;
				data = nullptr;
			}
			len = other.len;
			addr = other.addr;
			data = new char[len];
			memcpy(data, other.data, len);

			return *this;
		}

		char* data;
		int len;
		sockaddr addr;
	};

	struct packet_header_s
	{
		packet_header_s(uint16_t id, Timer::time_point time) : packet_id(id), timestamp(time) {}
		uint16_t packet_id;
		packet_data_s msg_data;
		Timer::time_point timestamp;
	};
}

class NetworkDeliveryReciever
{
public:
	NetworkDeliveryReciever();

	void unlabelpacket(BitStreamReader& bitreader, uint16_t packet_id, bool& packet_valid);
	void writeacknowledgementpacket(BitStreamWriter& bitwriter);
	bool anyacknowledgements() { return !acknowledgements.empty(); }
private:
	//receiver
	uint16_t nextexpected_packetid;
	std::vector<NetworkDelivery::acknowledgerange_s> acknowledgements;
};

class NetworkDeliverySender
{
public:
	NetworkDeliverySender();

	void labelpacketGarunteeBegin(BitStreamWriter& bitwriter);
	void labelpacketGarunteeEnd(const char* data, int len, sockaddr addr);
	void labelpacket(BitStreamWriter& bitwriter);
	void labelpacketfake(BitStreamWriter& bitwriter);

	void clearGarunteeMessages();

	void handleacknowledgementsPacket(BitStreamReader& bitreader, BitStreamWriter& bitwriter, std::shared_ptr<UDP_Socket> sender_socket, bool& resent);
	void checkinflightTimeout(BitStreamWriter& bitwriter, std::shared_ptr<UDP_Socket> sender_socket, bool& resent);

	size_t getinflightcount() const { return inflightpackets_garuntee.size(); }
private:
	//sender
	uint16_t current_packetid;
	std::deque<NetworkDelivery::packet_header_s> inflightpackets_garuntee;
	int TIMEOUT_PERIOD = 3000; //ms

	bool garuntee_validate;
	uint16_t last_garuntee_id;
private:
};

