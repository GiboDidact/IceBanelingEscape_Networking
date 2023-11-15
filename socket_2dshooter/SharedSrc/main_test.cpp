#include "pch.h"
#include <iostream>
#include <string>
#include <array>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>

#include "Sockets/WSA_Helper.h"
#include "Sockets/Socket_Helper.h"
#include "Sockets/SocketAddr_Helper.h"
#include "Sockets/UDP_Socket.h"
#include "Sockets/TCP_Socket.h"

#pragma comment(lib, "ws2_32.lib")

std::string ip_address = "0.0.0.0";
uint16_t port_address = 0;
bool sender = 0;

void StartUDP();
void StartTCP();

//client opens menu, clicks look for games. Shows all lobbies or can create one
//in lobby you can see players names and you can chat and you can ready up
//once everyones ready it launches you in game
//in game you can see the other person and just move around
//you can disconnect, etc

//CHATROOM
//TODO: create text chat system: theres a server and x amount of clients can connect and type and all see it with usernames
//client should be able to hit a connect button, then it connects or stalls, if they stall they can retry, if they connect server 
//should send a welcome message and have thier name. They should see everyone in the chat room who is currently in.
//Then they should be able to type and read the chat of everyone else in the chat
//Then the server accepts and joins people in the chatroom, and recieves their messages and sends that out to everyone else

//CLIENT.EXE and a SERVER.EXE

#include "Sockets/BitStream.h"

int main()
{
	std::cout << "main_test\n";
	BitStreamWriter bitwriter;
	std::vector<char> data = { 'b', 'c' };
	std::vector<int> data2 = { 3, 5, 100, 123123, -3123 };
	bitwriter.WriteBytes(data.data(), data.size());
	bitwriter.WriteBytes(data2.data(), data2.size()*sizeof(int));
	bitwriter.Write(true);

	//bitwriter.Print();

	char* buff = new char[bitwriter.getCapacity()];
	memcpy(buff, bitwriter.getData(), bitwriter.getCapacity());
	BitStreamReader bitreader(buff, bitwriter.getCapacity(), true);

	std::vector<char> indata(2);
	std::vector<int> indata2(5);
	bool val = false;
	bitreader.ReadBytes(indata.data(), indata.size()*sizeof(char));
	bitreader.ReadBytes(indata2.data(), indata2.size() * sizeof(int));
	bitreader.Read(&val);
	std::cout << indata[0] << " " << indata[1] << " " << indata2[0] << " " << indata2[1] <<
		indata2[2] << " " << indata2[3] << " " << indata2[4] <<" "<<val << std::endl;

	return 0;
}

//SHOOTER
int mainz(int argc, char *argv[])
{
	if (argc != 4) {
		std::cout << "needs 3 arguments! (IPv4, port#, sender (0/1))\n";
		return EXIT_FAILURE;
	}

	ip_address = argv[1];
	port_address = std::stoul(std::string(argv[2]));
	sender = std::stoi(std::string(argv[3]));

	std::cout << "ip_address: " << ip_address << " port#: " << port_address << " sender: " << sender << std::endl << std::endl;

	int is_error = StartWSA();
	if (is_error == -1) return EXIT_FAILURE;

	//StartUDP();
	StartTCP();	 


	CloseWSA();

	return EXIT_SUCCESS;
}

void StartTCP()
{
	TCP_Socket socket_listen;

	sockaddr addr_tcp;
	if (sender)
		addr_tcp = createSocketAddr_IPv4andPort(nullptr, "0", true);
	else
		addr_tcp = createSocketAddr_Mixed(ip_address.c_str(), port_address);

	socket_listen.Bind(addr_tcp);
	printSockName(socket_listen.getSocket());

	if (!sender)
	{
		socket_listen.Listen();
		std::cout << "listening...\n\n";
		
		sockaddr_in addr_of_request;
		std::shared_ptr<TCP_Socket> socket_tcp = socket_listen.Accept(addr_of_request);
		std::cout << "accepted "; printSocketAddr(addr_of_request);

		printSockName(socket_tcp->getSocket());

		sockaddr_in addr_check;
		int addr_len = sizeof(sockaddr_in);
		WSA_CHECK(getpeername(socket_tcp->getSocket(), reinterpret_cast<sockaddr*>(&addr_check), &addr_len), "getpeername ");
		std::cout << "new socket peer name "; printSocketAddr(addr_check);

		char name[100];
		gethostname(name, 100);
		std::cout << "hostname: " << name << std::endl;

		std::array<char, 100> buff;
		while (true)
		{
			//set length to 0 and if it returns 0 that means theres data to be read, nonzero length and return 0 FIN packet
			buff.fill(0);
			int bytesReceived =	socket_tcp->Recv(buff.data(), buff.size());
			if (bytesReceived == 0) //FIN PACKET
				break;

			std::string s = "";
			for (int i = 0; i < buff.size(); i++)
			{
				if (buff[i] == '\0')
					break;
				s += buff[i];
			}
			std::cout << " message: " << s << std::endl;
		}
		socket_tcp->ShutDown(SD_RECEIVE);
	}
	else
	{
		sockaddr addr_of_accept = createSocketAddr_Mixed(ip_address.c_str(), port_address);
		socket_listen.Connect(addr_of_accept);
		std::cout << "accepted to "; printSocketAddr(*reinterpret_cast<sockaddr_in*>(&addr_of_accept));

		std::string msg;
		std::getline(std::cin, msg);
		while (msg != "q")
		{
			int bytesSent = socket_listen.Send(msg.c_str(), msg.size());
			std::cout << "bytes sent " << bytesSent << std::endl;

			//set it to no blocking and see if ICMP Port Unreachable
			socket_listen.SetNonBlocking(1);
			char buff[1];
			int bytesReceived = socket_listen.Recv(buff, 1); 
			if (bytesReceived == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)
				std::cout << "ICMP Port Unreachable!\n";
			socket_listen.SetNonBlocking(0);
			
			msg.clear();
			std::getline(std::cin, msg);
		}
		//send FIN packet
		socket_listen.ShutDown(SD_SEND);
	}
}

void StartUDP()
{
	UDP_Socket socket_udp;

	sockaddr addr_udp;
	if (sender)
		addr_udp = createSocketAddr_IPv4andPort(nullptr, "0", true);
	else
		addr_udp = createSocketAddr_Mixed(ip_address.c_str(), port_address);
	
	socket_udp.Bind(addr_udp);
	printSockName(socket_udp.getSocket());


	if (sender) 
	{
		printMaxUDPSize(socket_udp.getSocket());

		sockaddr addr_sendto = createSocketAddr_Mixed(ip_address.c_str(), port_address);

		std::string msg;
		std::getline(std::cin, msg);
		while (strcmp(msg.c_str(), "q"))
		{
			int byteSentCount = socket_udp.SendTo(msg.c_str(), msg.size(), addr_sendto);
			std::cout << "bytes Sent(or queued): " << byteSentCount << std::endl;

			//set it to no blocking and see if ICMP Port Unreachable
			socket_udp.SetNonBlocking(true);

			sockaddr_in addr_sender;
			char buff[1];
			int bytesReceived = socket_udp.RecvFrom(buff, 1, addr_sender);
			if (bytesReceived == SOCKET_ERROR && WSAGetLastError() == WSAECONNRESET)
				std::cout << "ICMP Port Unreachable!\n";

			msg.clear();
			std::getline(std::cin, msg);
		}
	}
	else
	{
		sockaddr_in addr_sender;
		std::array<char, 100> buff;
		while (true)
		{
			buff.fill(0);
			int bytesReceived = socket_udp.RecvFrom(buff.data(), buff.size(), addr_sender);	
			std::cout << "bytes Received " << bytesReceived << " from address "; printSocketAddr(addr_sender);

			std::string s = "";
			for (int i = 0; i < buff.size(); i++)
			{
				if (buff[i] == '\0')
					break;
				s += buff[i];
			}

			std::cout << " message: " << s << std::endl;
		}
	}
}

