#include "../pch.h"
#include "NetworkDelivery.h"
#include "game_packets.h"

//this packet_id is used as a dummy so we completely ignore it, but it allows us to use the same system if we don't actually want to use system
const uint16_t reserved_packid = 0;

//sender
NetworkDeliverySender::NetworkDeliverySender()
{
	current_packetid = 1;
	garuntee_validate = false;
	last_garuntee_id = 0;
}

void NetworkDeliverySender::labelpacketGarunteeBegin(BitStreamWriter& bitwriter)
{
	my_assert(garuntee_validate == false, "labelpacketGarunteeBegin");

	bitwriter.Write(current_packetid);
		
	inflightpackets_garuntee.emplace_back(current_packetid, Timer::getCurrentTime());
	last_garuntee_id = current_packetid;

	current_packetid = (current_packetid + 1) % std::numeric_limits<uint16_t>::max();
	if (current_packetid == reserved_packid) current_packetid++;

	garuntee_validate = true;
}

void NetworkDeliverySender::labelpacketGarunteeEnd(const char* data, int len, sockaddr addr)
{
	my_assert(garuntee_validate == true, "labelpacketGarunteeEnd");

	auto itr = inflightpackets_garuntee.begin();
	while (itr != inflightpackets_garuntee.end())
	{
		if (itr->packet_id == last_garuntee_id)
			break;
		itr++;
	}

	if (itr != inflightpackets_garuntee.end())
	{
		//ignore the packet_id in the data (uint16_t, 2 bytes)
		my_assert(len - 2 > 0, "labelpacketGarunteeEnd-data should be more than just 2 bytes");
		itr->msg_data.data = new char[len - 2];
		itr->msg_data.len = len - 2;
		itr->msg_data.addr = addr;
		memcpy(itr->msg_data.data, data + 2, len - 2);
	}
	else
	{
		std::cout << "label garuntee id couldn't be found!\n";
	}

	garuntee_validate = false;
}

void NetworkDeliverySender::labelpacket(BitStreamWriter& bitwriter)
{
	bitwriter.Write(current_packetid);

	current_packetid = (current_packetid + 1) % std::numeric_limits<uint16_t>::max();
	if (current_packetid == reserved_packid) current_packetid++;
}

//we just want to stop all transmission of messages so clear this out. 
//*Only call this when its okay to destroy all previous garuntees
void NetworkDeliverySender::clearGarunteeMessages()
{
	while (!inflightpackets_garuntee.empty())
		inflightpackets_garuntee.pop_front();
}

void NetworkDeliverySender::labelpacketfake(BitStreamWriter& bitwriter)
{
	bitwriter.Write(reserved_packid);
}

void NetworkDeliverySender::handleacknowledgementsPacket(BitStreamReader& bitreader, BitStreamWriter& bitwriter, std::shared_ptr<UDP_Socket> sender_socket, bool& resent)
{
	std::vector<NetworkDelivery::acknowledgerange_s> incoming_acks;
	ReadGamePacket_acknowledgement(bitreader, incoming_acks);
	resent = false;
	for (int i = 0; i < incoming_acks.size(); i++)
	{
		NetworkDelivery::acknowledgerange_s& curr_ackrange = incoming_acks[i];
		auto itr = inflightpackets_garuntee.begin();
		int remove_count = 0;

		while (itr != inflightpackets_garuntee.end() && itr->packet_id < curr_ackrange.start_id)
		{
			remove_count++;
			itr++;
		}
		for (int r = 0; r < remove_count; r++)
		{
			//std::cout << "resending because of acknowledgement packet\n";
			//resend message
			NetworkDelivery::packet_data_s& packet_data = inflightpackets_garuntee.front().msg_data;

			bitwriter.reset();
			labelpacketGarunteeBegin(bitwriter);
			bitwriter.WriteBytes(packet_data.data, packet_data.len);
			sender_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), packet_data.addr);
			labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), packet_data.addr);
			
			inflightpackets_garuntee.pop_front();
			resent = true;
		}
	
		while (!inflightpackets_garuntee.empty() && inflightpackets_garuntee.front().packet_id < curr_ackrange.start_id + curr_ackrange.count)
		{
			inflightpackets_garuntee.pop_front();
		}
	}
}

void NetworkDeliverySender::checkinflightTimeout(BitStreamWriter& bitwriter, std::shared_ptr<UDP_Socket> sender_socket, bool& resent)
{
	Timer::time_point curr_time = Timer::getCurrentTime();
	if (inflightpackets_garuntee.size() > 1000)
	{
		std::cout << "warning garuntee packages waiting are getting out of hand!\n";
	}
	resent = false;
	while (!inflightpackets_garuntee.empty())
	{
		if (Timer::getDifference(curr_time, inflightpackets_garuntee.front().timestamp) > TIMEOUT_PERIOD)
		{
			std::cout << "resending because of a timeout\n";
			//resend message
			NetworkDelivery::packet_data_s& packet_data = inflightpackets_garuntee.front().msg_data;

			//printSocketAddr(*reinterpret_cast<sockaddr_in*>(&packet_data.addr));
			//std::cout << packet_data.len << std::endl;
			//BitStreamReader reader(packet_data.data, packet_data.len, false);
			//GAMEPACKETCODE opcode = GAMEPACKETCODE::ENEMY;
			//reader.Read(&opcode);
			//std::cout << (int)opcode << std::endl;

			bitwriter.reset();
			labelpacketGarunteeBegin(bitwriter);
			bitwriter.WriteBytes(packet_data.data, packet_data.len);
			sender_socket->SendTo(bitwriter.getData(), bitwriter.getBytesUsed(), packet_data.addr);
			labelpacketGarunteeEnd(bitwriter.getData(), bitwriter.getBytesUsed(), packet_data.addr);

			inflightpackets_garuntee.pop_front();
			resent = true;
		}
		else
		{
			break;
		}
	}
}

//receiver
NetworkDeliveryReciever::NetworkDeliveryReciever()
{
	nextexpected_packetid = 1;
}

void NetworkDeliveryReciever::unlabelpacket(BitStreamReader& bitreader, uint16_t packet_id, bool& packet_valid)
{
	//drop packets out of order, save in acknowledgements
	uint16_t current_pack_id = packet_id;
	if (current_pack_id == reserved_packid)
	{
		packet_valid = true;
	}
	else if (current_pack_id == nextexpected_packetid)
	{
		if (acknowledgements.empty() || !acknowledgements.back().extendRange(current_pack_id))
		{
			acknowledgements.push_back(current_pack_id);
		}
		nextexpected_packetid = (nextexpected_packetid + 1) % std::numeric_limits<uint16_t>::max();
		packet_valid = true;
	}
	else if (current_pack_id < nextexpected_packetid)
	{
		//this packet lagged behind, drop this packet because it is out of order now
		packet_valid = false;
	}
	else if (current_pack_id > nextexpected_packetid)
	{
		//this packet got ahead of other ones, treat it like the next expected packet
		if (acknowledgements.empty() || !acknowledgements.back().extendRange(current_pack_id))
		{
			acknowledgements.push_back(current_pack_id);
		}
		nextexpected_packetid = (current_pack_id + 1) % std::numeric_limits<uint16_t>::max();
		packet_valid = true;
	}
}

void NetworkDeliveryReciever::writeacknowledgementpacket(BitStreamWriter& bitwriter)
{
	//we have an array of acknowledgement ranges, send these as a packet
	WriteGamePacket_acknowledgement(bitwriter, acknowledgements);

	acknowledgements.clear();
}




/*

before filling and sending packet, let delivery know about it to give it an id
pass in stream so that we can tag id to packet and say if you want it to be garunteed to be sent
then you will fill in the packet and send

then the reciever will call the networkdelivery read, the read will go through all the packets idenfying their packet id numbers
and creating acknowledgements and such, then we can actually parse and read through the packets ourselves


--Sender--
networkdelivery->handleacknowledgementpacket()

networkdelivery->getfailedpacket_opcodes
now just handle each packetcode that got dropped by resending data

networkdelivery->sendinggaruntee(bitstream)
WriteGamePacket(bitstream)
socket->send()

--receiver--

socket->recv()
networkdelivery->unlabelpacket()
bitreader.read(gamepacketcode)
...
if(opcode==acknowledgements)
handleacknowledgementsPacket()
	
networkdelivery->sendacknowledgements()
loop through each packet, for each recv and handle all the packets in the program


--both--
do a recv and loop through every packet handling it, and making sure we skip byte
then potentially need to do another recv but I can probably garuntee there won't be that much information

--questions--
. can I handle every packet incoming at this point? multiple packets in the frame
. what if I disconnect at any point?

*/