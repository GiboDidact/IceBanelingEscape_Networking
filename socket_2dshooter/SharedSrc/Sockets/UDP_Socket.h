#pragma once

class UDP_Socket
{
public:
	UDP_Socket()
	{
		SOCKET_CHECK(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP), m_socket, "creating UDP_Socket ");
	}
	~UDP_Socket()
	{
		WSA_CHECK(closesocket(m_socket), "~UDP_Socket() ");
	}

	/* Disabling copy and move semantics because XYZ */
	UDP_Socket(const UDP_Socket&) = delete;
	UDP_Socket(UDP_Socket&&) = delete;
	UDP_Socket& operator=(const UDP_Socket&) = delete;
	UDP_Socket& operator=(UDP_Socket&&) = delete;

	int Bind(const sockaddr& inaddr)
	{
		int res;
		WSA_CHECK_S(res, bind(m_socket, &inaddr, sizeof(sockaddr)), "UDP_Socket::Bind() ");
		return res;
	}

	int Bind(sockaddr_in& inaddr)
	{
		int res;
		WSA_CHECK_S(res, bind(m_socket, reinterpret_cast<sockaddr*>(&inaddr), sizeof(sockaddr_in)), "UDP_Socket::Bind() ");
		return res;
	}

	int SendTo(const char* buf, int len, sockaddr& addr)
	{
#ifdef PING_EMULATOR
		//copy data in buf, take length, and take socket
		PingEmulatorUDP::addmsg(m_socket, buf, len, addr);
		return 0;
#endif

		int byteSentCount;
		WSA_CHECK_S(byteSentCount, sendto(m_socket, buf, len, 0, &addr, sizeof(sockaddr)), "UDP_Socket::SendTo() ");
		return byteSentCount;
	}

	int RecvFrom(char* buf, int len, sockaddr_in& addr_in)
	{
		int sender_len = sizeof(addr_in);
		int bytesReceived;
		WSA_CHECK_S(bytesReceived, recvfrom(m_socket, buf, len, 0, reinterpret_cast<sockaddr*>(&addr_in), &sender_len), "UDP_Socket::RecvFrom() ");
		if (bytesReceived >= len)
			std::cout << "received more data than buffer holds, some data probably got lost and truncated\n";
		return bytesReceived;
	}

	int RecvFromNoError(char* buf, int len, sockaddr_in& addr_in)
	{
		int sender_len = sizeof(addr_in);
		int bytesReceived = recvfrom(m_socket, buf, len, 0, reinterpret_cast<sockaddr*>(&addr_in), &sender_len);
		if (bytesReceived >= len)
			std::cout << "received more data than buffer holds, some data probably got lost and truncated\n";
		return bytesReceived;
	}

	void SetNonBlocking(bool nonblocking)
	{
		u_long enable = nonblocking;
		WSA_CHECK(ioctlsocket(m_socket, FIONBIO, &enable), "UDP_Socket::SetNonBlocking() ");
	}

	SOCKET getSocket() { return m_socket; }
private:
	SOCKET m_socket;
};