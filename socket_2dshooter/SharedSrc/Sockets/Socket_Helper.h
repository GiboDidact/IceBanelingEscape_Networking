#pragma once

static sockaddr_in* GetAsSockAddrIn(sockaddr sockaddr) { return reinterpret_cast<sockaddr_in*>(&sockaddr); }
static sockaddr* GetAsSockAddr(sockaddr_in sockaddrin) { return reinterpret_cast<sockaddr*>(&sockaddrin); }

static void printSocketAddr(const sockaddr_in& inaddr)
{
	std::cout << "port " << ntohs(inaddr.sin_port) << " IPv4 " <<
		(int)inaddr.sin_addr.S_un.S_un_b.s_b1 << "." <<
		(int)inaddr.sin_addr.S_un.S_un_b.s_b2 << "." <<
		(int)inaddr.sin_addr.S_un.S_un_b.s_b3 << "." <<
		(int)inaddr.sin_addr.S_un.S_un_b.s_b4 << std::endl;
}

static void printSockName(SOCKET socket)
{
	sockaddr_in addr_check;
	int addr_len = sizeof(sockaddr_in);
	WSA_CHECK(getsockname(socket, reinterpret_cast<sockaddr*>(&addr_check), &addr_len), "checking socket name ");
	std::cout << "socket name address "; printSocketAddr(addr_check);
}

static void printPeerName(SOCKET socket)
{
	sockaddr_in addr_check;
	int addr_len = sizeof(sockaddr_in);
	WSA_CHECK(getpeername(socket, reinterpret_cast<sockaddr*>(&addr_check), &addr_len), "getpeername ");
	std::cout << "socket peer name: "; printSocketAddr(addr_check);
}

static unsigned int getMaxUDPSize(SOCKET socket)
{
	unsigned int max_msg_size;
	int len = sizeof(unsigned int);
	WSA_CHECK(getsockopt(socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&max_msg_size, &len), "get max msg size ");
	return max_msg_size;
}

static void printMaxUDPSize(SOCKET socket)
{
	unsigned int max_msg_size;
	int len = sizeof(unsigned int);
	WSA_CHECK(getsockopt(socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&max_msg_size, &len), "get max msg size ");
	std::cout << "max UDP msg size: " << max_msg_size << std::endl;
}

static void AddrInfoPrint(const char* host, const char* service)
{
	ADDRINFOA hints;
	ZeroMemory(&hints, sizeof(hints));
	//hints.ai_family = AF_INET;
	//hints.ai_socktype = SOCK_STREAM;
	//hints.ai_flags = AI_PASSIVE; // use my IP

	PADDRINFOA infos;
	int err = getaddrinfo(host, service, &hints, &infos);
	if (err != 0)
	{
		std::cout << "getaddrinfo failed with error " << err << std::endl;
		return;
	}

	//we got linked list, go through and print everything
	int i = 1;
	struct sockaddr_in* sockaddr_ipv4;
	LPSOCKADDR sockaddr_ip;
	WSADATA wsaData;
	int iResult;
	INT iRetval;
	DWORD dwRetval;
	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = 46;
	for (PADDRINFOA ptr = infos; ptr != nullptr; ptr = ptr->ai_next)
	{
		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
			char buf[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, buf, sizeof(buf));
			printf("\tIPv4 address %s port %hu\n", buf, ntohs(sockaddr_ipv4->sin_port));
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");
			// the InetNtop function is available on Windows Vista and later
			// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			// printf("\tIPv6 address %s\n",
			//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

			// We use WSAAddressToString since it is supported on Windows XP and later
			sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
				ipstringbuffer, &ipbufferlength);
			if (iRetval)
				printf("WSAAddressToString failed with %u\n", WSAGetLastError());
			else
				printf("\tIPv6 address %s\n", ipstringbuffer);
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS (NetBIOS)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_family);
			break;
		}
		printf("\tSocket type: ");
		switch (ptr->ai_socktype) {
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_socktype);
			break;
		}
		printf("\tProtocol: ");
		switch (ptr->ai_protocol) {
		case 0:
			printf("Unspecified\n");
			break;
		case IPPROTO_TCP:
			printf("IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			printf("IPPROTO_UDP (UDP) \n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_protocol);
			break;
		}
		printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
		printf("\tCanonical name: %s\n", ptr->ai_canonname);
	}

	freeaddrinfo(infos);
}