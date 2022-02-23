#ifndef SMART_LOG_INTERFACE_HPP
#define SMART_LOG_INTERFACE_HPP

#include "ae.h"

#define USED_STD_OUT	 1

NAMESPACE_BEGIN(smart)

class LogWriter
{
public:
	enum Level {
		Debug = 0,
		Info,
		Warn,
		Error,
		Fatal,
		LevelSize
	};
public:
	static LogWriter* instance();
	static void to_hex(std::string& message, const char* data, unsigned int size);
	static void to_hex(std::string& message, std::string& data);
public:
	void register_interface(WriteLogCallback callback);
	void write(Level level, const std::string& data);
	void write(Level level, const std::string&& data);
public:
	void fatal(const std::string& data);
	void fatal(const std::string&& data);
	void warn(const std::string& data);
	void warn(const std::string&& data);
	void error(const std::string& data);
	void error(const std::string&& data);
	void info(const std::string& data);
	void info(const std::string&& data);
	void debug(const std::string& data);
	void debug(const std::string&& data);
public:
	void set_level(int level);
	int get_level();
	const std::string& get_level_name(int level);
private:
	LogWriter();
	WriteLogCallback callback_;
	int level_;
	std::vector<std::string> levelStr_;
	std::string nullLevel_;
};

NAMESPACE_END(smart)
#endif 

