#pragma once

class TCP_Socket
{
public:
	TCP_Socket()
	{
		SOCKET_CHECK(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP), m_socket, "creating TCP_Socket");
		shutdown_called = false;
	}
	~TCP_Socket()
	{
		WSA_CHECK(closesocket(m_socket), "~TCP_Socket() ");
	}

	/* Disabling copy and move semantics because XYZ */
	TCP_Socket(const TCP_Socket&) = delete;
	TCP_Socket(TCP_Socket&&) = delete;
	TCP_Socket& operator=(const TCP_Socket&) = delete;
	TCP_Socket& operator=(TCP_Socket&&) = delete;

	int Bind(const sockaddr& inaddr)
	{
		int res;
		WSA_CHECK_S(res, bind(m_socket, &inaddr, sizeof(sockaddr)), "TCP_Socket::Bind() ");
		return res;
	}
	int Bind(sockaddr_in& inaddr)
	{
		int res;
		WSA_CHECK_S(res, bind(m_socket, reinterpret_cast<sockaddr*>(&inaddr), sizeof(sockaddr_in)), "TCP_Socket::Bind() ");
		return res;
	}

	void Listen(int backlog = SOMAXCONN)
	{
		WSA_CHECK(listen(m_socket, backlog), "TCP_Socket::Listen() ");
	}

	//stalls until something connected
	std::shared_ptr<TCP_Socket> Accept(sockaddr_in& addr)
	{
		SOCKET socket_tcp;
		int incoming_len = sizeof(sockaddr_in);
		SOCKET_CHECK(accept(m_socket, reinterpret_cast<sockaddr*>(&addr), &incoming_len), socket_tcp, "TCP_Socket::Accept() ");
		
		if (socket_tcp != INVALID_SOCKET)
			return std::shared_ptr<TCP_Socket>(new TCP_Socket(socket_tcp));
		else
			return nullptr;
	}

	//stalls until accepted
	int Connect(sockaddr& addr)
	{
		int result;
		WSA_CHECK_S(result, connect(m_socket, &addr, sizeof(sockaddr)), "TCP_Socket::Connect() ");
		return result;
	}

	int Connect_noerr(sockaddr& addr)
	{
		int result = connect(m_socket, &addr, sizeof(sockaddr));
		return result;
	}

	int Send(const char* buf, int len)
	{
#ifdef PING_EMULATOR
		//copy data in buf, take length, and take socket
		PingEmulator::addmsg(m_socket, buf, len);
		return 0;
#endif
		int byteSentCount;
		WSA_CHECK_S(byteSentCount, send(m_socket, buf, len, 0), "TCP_Socket::SendTo() ");
		return byteSentCount;
	}

	int Recv(char* buf, int len)
	{
		int bytesReceived;
		WSA_CHECK_S(bytesReceived, recv(m_socket, buf, len, 0), "TCP_Socket::Recv() ");
		if (bytesReceived >= len)
			std::cout << "received more data than buffer holds, some data probably got lost and truncated\n";
		return bytesReceived;
	}

	int Recvnoerror(char* buf, int len, int flags = 0)
	{
		int bytesReceived = recv(m_socket, buf, len, flags);
		if (bytesReceived >= len)
			std::cout << "received more data than buffer holds, some data probably got lost and truncated\n";
		return bytesReceived;
	}

	//SD_RECEIVE, SD_SEND, SD_BOTH
	void ShutDown(int how)
	{
		WSA_CHECK(shutdown(m_socket, how), "TCP_Socket::ShutDown() ");

		shutdown_called = true;
	}

	bool isShutDown() const
	{
		return shutdown_called;
	}

	void SetNonBlocking(bool nonblocking)
	{
		u_long enable = nonblocking;
		WSA_CHECK(ioctlsocket(m_socket, FIONBIO, &enable), "UDP_Socket::SetNonBlocking() ");
	}
	
	static int Select(const std::vector<std::shared_ptr<TCP_Socket>>* inReadSet,
		std::vector<std::shared_ptr<TCP_Socket>>* outReadSet,
		const std::vector<std::shared_ptr<TCP_Socket>>* inWriteSet,
		std::vector<std::shared_ptr<TCP_Socket>>* outWriteSet,
		const std::vector<std::shared_ptr<TCP_Socket>>* inExceptSet,
		std::vector<std::shared_ptr<TCP_Socket>>* outExceptSet, const timeval* timeout)
	{

		fd_set read, write, except;

		fd_set* readPtr = FillSetFromVector(read, inReadSet);
		fd_set* writePtr = FillSetFromVector(write, inWriteSet);
		fd_set* exceptPtr = FillSetFromVector(except, inExceptSet);

		int socketsReady = select(0, readPtr, writePtr, exceptPtr, timeout);

		if (socketsReady > 0)
		{
			FillVectorFromSet(outReadSet, inReadSet, read);
			FillVectorFromSet(outWriteSet, inWriteSet, write);
			FillVectorFromSet(outExceptSet, inExceptSet, except);
		}

		return socketsReady;
	}
	

	SOCKET getSocket() { return m_socket; }

private:
	TCP_Socket(SOCKET sock) : m_socket(sock) { shutdown_called = false; }

	SOCKET m_socket;
	bool shutdown_called;

private:
	
	static fd_set* FillSetFromVector(fd_set& outSet, const std::vector<std::shared_ptr<TCP_Socket>>* inSockets)
	{
		if (inSockets)
		{
			FD_ZERO(&outSet);
			for (const auto& socket : *inSockets)
			{
				FD_SET(socket->getSocket(), &outSet);
			}
			return &outSet;
		}
		else
		{
			return nullptr;
		}

	}
	
	static void FillVectorFromSet(std::vector<std::shared_ptr<TCP_Socket>>* outSockets,
		const std::vector<std::shared_ptr<TCP_Socket>>* inSockets,
		const fd_set& inSet)
	{
		if (outSockets && inSockets)
		{
			outSockets->clear();
			for (const auto& socket : *inSockets)
			{
				if (FD_ISSET(socket->getSocket(), &inSet))
				{
					outSockets->push_back(socket);
				}
			}
		}
	}
	
};