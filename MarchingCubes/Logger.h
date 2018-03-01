#pragma once
#include <string>


#define MAX_LOG_MSG 4096


#define LOG(message, ...) { \
	static char fmsg[MAX_LOG_MSG]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogMessage(fmsg); \
}

#ifdef _DEBUG
#define LOG_WARNING(message, ...) { \
	static char fmsg[MAX_LOG_MSG]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogWarning(fmsg, __FILE__, __LINE__); \
}

#define LOG_ERROR(message, ...) { \
	static char fmsg[MAX_LOG_MSG]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogError(fmsg, __FILE__, __LINE__); \
}
#else
#define LOG_WARNING(message, ...) { \
	static char fmsg[MAX_LOG_MSG]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogWarning(fmsg); \
}

#define LOG_ERROR(message, ...) { \
	static char fmsg[MAX_LOG_MSG]; \
	snprintf(fmsg, sizeof(fmsg), message, __VA_ARGS__); \
	Logger::LogError(fmsg); \
}
#endif


/**
* Centralized logging class
* NOTE: All text output should be through this class (preferably by use of LOG(_XYZ) macros)
*/
class Logger
{
public:
	static void LogMessage(std::string msg);

#ifdef _DEBUG
	static void LogWarning(std::string msg, std::string file, int line);
	static void LogError(std::string msg, std::string file, int line);
#else
	static void LogWarning(std::string msg);
	static void LogError(std::string msg);
#endif
};

