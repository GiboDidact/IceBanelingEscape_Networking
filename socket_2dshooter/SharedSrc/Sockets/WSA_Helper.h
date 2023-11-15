#pragma once



#ifndef WSA_MACROS
#define WSA_MACROS 
//-------------MACROS
#define TEXT_RED 12
#define TEXT_YELLOW 14
#define TEXT_BLACK 1
#define TEXT_GREEN 10
#define TEXT_ORANGE 5
#define TEXT_WHITE 7


#define SOCKET_CHECK(result, socket, msg) \
socket = result; \
if(socket == INVALID_SOCKET) \
{ \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
	SetConsoleTextAttribute(hConsole, TEXT_RED); \
	std::cout << "SOCKET ERROR " << msg << WSAGetLastError() << std::endl; \
	SetConsoleTextAttribute(hConsole, TEXT_WHITE); \
}
#define WSA_CHECK(result, msg) \
if(result == SOCKET_ERROR) \
{ \
	int err = WSAGetLastError(); \
	if(err != WSAEWOULDBLOCK) { \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
	SetConsoleTextAttribute(hConsole, TEXT_RED); \
	std::cout<<"WSAERROR "<<msg<<WSAGetLastError()<<std::endl; \
	SetConsoleTextAttribute(hConsole, TEXT_WHITE);}\
}
#define WSA_CHECK_S(store, result, msg) \
store = result; \
if(store == SOCKET_ERROR) \
{ \
	int err = WSAGetLastError(); \
	if(err != WSAEWOULDBLOCK) { \
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE); \
	SetConsoleTextAttribute(hConsole, TEXT_RED); \
	std::cout<<"WSAERROR "<<msg<<WSAGetLastError()<<std::endl; \
	SetConsoleTextAttribute(hConsole, TEXT_WHITE);}\
}
#endif
//-------------MACROS


static int StartWSA()
{
	uint8_t WSAmajor_v = 2;
	uint8_t WSAminor_v = 2;

	WSADATA wsaData;
	int err;
	err = WSAStartup(MAKEWORD(WSAmajor_v, WSAminor_v), &wsaData);
	if (err != 0)
	{
		std::cout << "WSAStartup Error (" << err << ") ";
		switch (err)
		{
		case WSASYSNOTREADY: std::cout << "WSASYSNOTREADY- The underlying network subsystem is not ready for network communication"; break;
		case WSAVERNOTSUPPORTED: std::cout << "WSAVERNOTSUPPORTED- The version of Windows Sockets support requested is not provided by this particular Windows Sockets implementation"; break;
		case WSAEINPROGRESS: std::cout << "WSAEINPROGRESS- A blocking Windows Sockets 1.1 operation is in progress"; break;
		case WSAEPROCLIM: std::cout << "WSAEPROCLIM- A limit on the number of tasks supported by the Windows Sockets implementation has been reached"; break;
		case WSAEFAULT: std::cout << "WSAEFAULT- The lpWSAData parameter is not a valid pointer"; break;
		default: std::cout << "unknown error return type";  break;
		}
		return -1;
	}

	std::cout << "requested wsa version: " << (int)WSAmajor_v << " " << (int)WSAminor_v << std::endl;
	std::cout << "--wsaData--\n" <<
		"version: " << (int)LOBYTE(wsaData.wVersion) << " " << (int)HIBYTE(wsaData.wVersion) << "\n" <<
		"highest supported version: " << (int)LOBYTE(wsaData.wHighVersion) << " " << (int)HIBYTE(wsaData.wHighVersion) << "\n" <<
		"system status: " << wsaData.szSystemStatus << "\n" <<
		"description: " << wsaData.szDescription << "\n" <<
		"maxudpdg: " << wsaData.iMaxUdpDg << "\n" <<
		"max sockets: " << wsaData.iMaxSockets <<
		"\n------------------\n";

	if (LOBYTE(wsaData.wVersion) != WSAmajor_v || HIBYTE(wsaData.wVersion) != WSAminor_v) {
		/* Tell the user that we could not find a usable */
		/* WinSock DLL.                                  */
		std::cout << "Could not find requested version " << (int)WSAmajor_v << " " << (int)WSAminor_v << " of Winsock.dll\n";
		WSACleanup();
		return -1;
	}
	else
		std::cout << "The Winsock dll version " << (int)WSAmajor_v << " " << (int)WSAminor_v << " was found okay\n\n";

	return 0;
}

static void CloseWSA()
{
	std::cout << "\n";
	//make sure all sockets are closed and truly unused before shutting down Winsock
	int err = WSACleanup();
	if (err == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		std::cout << "WSACleanup error (" << err << ") ";
		switch (WSAGetLastError())
		{
		case WSANOTINITIALISED: std::cout << "WSANOTINITIALISED- A successful WSAStartup call must occur before using this function";  break;
		case WSAENETDOWN: std::cout << "WSAENETDOWN- The network subsystem has failed";  break;
		case WSAEINPROGRESS: std::cout << "WSAEINPROGRESS- A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function";  break;
		default: std::cout << "unkown error code for this function";
		}
	}
	else
		std::cout << "successfully closed down WSA\n";
}

static std::string WSAERROR_TO_STRING(int result)
{
	std::string a;
	switch (result)
	{
	case WSAEREFUSED: a = "WSAEREFUSED- dwqkjdoqwkdqow"; break;
	default: a = "unknown error code"; break;
	};

	return a;
}