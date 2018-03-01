#include "Logger.h"
#include <ctime>
#include <sstream>
#include <iostream>


/**
* Get the current system time in format [%H:%M:%S]
*/
static std::string TimeStamp()
{
	time_t timeStamp = time(nullptr);
	tm date;

	gmtime_s(&date, &timeStamp);

	char timec[16];
	//[%d-%m-%Y %H:%M:%S]
	strftime(timec, 80, "[%H:%M:%S]", &date);
	return timec;
}

void Logger::LogMessage(std::string msg)
{
	std::stringstream stream;
	stream << TimeStamp() << ": " << msg << '\n';
	std::cout << stream.str();
}

#ifdef _DEBUG
void Logger::LogWarning(std::string msg, std::string file, int line)
{
	std::stringstream stream;
	stream << "__WARNING__" << TimeStamp() << ": " << msg << '\n' << "\t@ (" << line << ')' << file << '\n';
	std::cout << stream.str();
}

void Logger::LogError(std::string msg, std::string file, int line)
{
	std::stringstream stream;
	stream << "___ERROR___" << TimeStamp() << ": " << msg << '\n' << "\t@ (" << line << ')' << file << '\n';
	std::cerr << stream.str();
}

#else
void Logger::LogWarning(std::string msg)
{
	std::stringstream stream;
	stream << "__WARNING__" << TimeStamp() << ": " << msg << '\n';
	std::cout << stream.str();
}

void Logger::LogError(std::string msg)
{
	std::stringstream stream;
	stream << "___ERROR___" << TimeStamp() << ": " << msg << '\n';
	std::cerr << stream.str();
}
#endif