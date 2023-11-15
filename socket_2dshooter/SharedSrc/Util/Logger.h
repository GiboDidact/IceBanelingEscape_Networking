#pragma once

template <typename T>
void variadic_print(T v) {
	std::cout << v;
}
template <typename T, typename... Types>
void variadic_print(T var1, Types... var2) {
	std::cout << var1;
	variadic_print(var2...);
}

class Logger
{
public:
	static const uint8_t _INFO = 0;
	static const uint8_t _WARNING = 1;
	static const uint8_t _ERROR = 2;
	//bool de
	template <typename T, typename... Types>
	static void Log(uint8_t type, T var1, Types... var2)
	{
		SetType(type);

		variadic_print(var1, var2...);
		std::cout << std::endl;
		SetType(_INFO, false);
	}

	template <typename T, typename... Types>
	static void LogDebug(uint8_t type, T var1, Types... var2)
	{
#ifdef _DEBUG 
		SetType(type);

		variadic_print(var1, var2...);
		std::cout << std::endl;
		SetType(_INFO, false);
#endif
	}

private:
	static void SetType(uint8_t type, bool print = true)
	{
#ifdef PLATFORM_WINDOWS
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
		switch (type)
		{
		case _INFO:
#ifdef PLATFORM_WINDOWS
			SetConsoleTextAttribute(hConsole, TEXT_WHITE);
#endif
			if(print)  printf("INFO: ");    
			break;
		case _WARNING: 
#ifdef PLATFORM_WINDOWS
			SetConsoleTextAttribute(hConsole, TEXT_YELLOW);
#endif
			if(print)  printf("WARNING: ");
			break;
		case _ERROR:   
#ifdef PLATFORM_WINDOWS
			SetConsoleTextAttribute(hConsole, TEXT_RED);
#endif
			if(print) printf("ERROR: ");
			break;
		default: break;
		};
	}

	static const int TEXT_RED = 12;
	static const int TEXT_YELLOW = 14;
	static const int TEXT_BLUE = 1;
	static const int TEXT_WHITE = 7;
};
