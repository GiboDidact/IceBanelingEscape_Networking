#pragma once

//passing in 0 for port will pick available one
//addr_any binds it to any address that you have/own
static sockaddr createSocketAddr_IPv4andPort(const char* ipv4, const char* port, bool addr_any = false)
{
	sockaddr addr;
	memset(reinterpret_cast<sockaddr_in*>(&addr)->sin_zero, 0, sizeof(reinterpret_cast<sockaddr_in*>(&addr)->sin_zero));
	reinterpret_cast<sockaddr_in*>(&addr)->sin_family = AF_INET;
	reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(std::stoul(port));
	if (addr_any) 
	{
		reinterpret_cast<sockaddr_in*>(&addr)->sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	}
	else
	{
		int err = inet_pton(AF_INET, ipv4, &reinterpret_cast<sockaddr_in*>(&addr)->sin_addr);
		if (err != 1) std::cout << err << " string malformed or other error inet_pton\n";
	}

	return addr;
}

static sockaddr createSocketAddr_Simple(uint32_t ipv4, uint16_t port)
{
	sockaddr addr;
	memset(reinterpret_cast<sockaddr_in*>(&addr)->sin_zero, 0, sizeof(reinterpret_cast<sockaddr_in*>(&addr)->sin_zero));
	reinterpret_cast<sockaddr_in*>(&addr)->sin_family = AF_INET;
	reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(port);
	reinterpret_cast<sockaddr_in*>(&addr)->sin_addr.S_un.S_addr = htonl(ipv4);

	return addr;
}

static sockaddr createSocketAddr_Mixed(const char* ipv4, uint16_t port)
{
	sockaddr addr;
	memset(reinterpret_cast<sockaddr_in*>(&addr)->sin_zero, 0, sizeof(reinterpret_cast<sockaddr_in*>(&addr)->sin_zero));
	reinterpret_cast<sockaddr_in*>(&addr)->sin_family = AF_INET;
	reinterpret_cast<sockaddr_in*>(&addr)->sin_port = htons(port);
	if (ipv4)
	{
		int err = inet_pton(AF_INET, ipv4, &reinterpret_cast<sockaddr_in*>(&addr)->sin_addr);
		if (err != 1) std::cout << err << " string malformed or other error inet_pton\n";
	}
	else
	{
		reinterpret_cast<sockaddr_in*>(&addr)->sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	}


	return addr;
}

//host name - sometimes can be domain name like www.example.com or numeric host address string like IPv4 
//service name - can be http or https or just a port number
//can pass in null or host name or service name but not both. If null it will just look for available ones
//socktype - SOCK_STREAM, SOCK_DGRAM
//flags - AI_PASSIVE: use my IP
static sockaddr createSocketAddr_getaddrinfo(const char* hostname, const char* servicename, int socktype, int flags)
{
	sockaddr addr;
	addr.sa_family = AF_INET6;

	ADDRINFOA hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = socktype;
	hints.ai_flags = flags;

	PADDRINFOA infos;
	int err = getaddrinfo(hostname, servicename, nullptr, &infos);
	PADDRINFOA init_infos = infos;
	if (err == SOCKET_ERROR || infos == nullptr)
	{
		std::cout << "creating socket error dont use this address!\n";
		if(infos != nullptr)
			freeaddrinfo(infos);
		return addr;
	}

	while (!infos->ai_addr && infos->ai_next)
	{
		infos = infos->ai_next;
	}
	
	if (!infos->ai_addr) {
		std::cout << "creating socket error dont use this address!\n";
		freeaddrinfo(infos);
		return addr;
	}
	
	memcpy(&addr, &infos->ai_addr, sizeof(sockaddr));

	freeaddrinfo(init_infos); 
}